#include "scene.h"

#include "gl/shaders/Shader.h"
#include "gl/textures/Texture2D.h"
#include "gl/util/FullScreenQuad.h"
#include "gl/util/ResourceLoader.h"
#include "pathtracer/pathtracer.h"
#include "util/CS123Common.h"
#include "util/CS123XmlSceneParser.h"
#include "util/util.h"

#include "glm/gtx/transform.hpp"
#include "glm/gtx/string_cast.hpp"

#include <GL/glew.h>

#include <QImage>

#include <iostream>
#include <chrono>

#define TINYOBJLOADER_IMPLEMENTATION
#include "util/tiny_obj_loader.h"

using namespace std;
using namespace std::chrono;
using namespace CS123::GL;

Scene::Scene(int width, int height, unsigned int samples) : width(width), height(height), samples(samples), wavelet_iterations(5), m_pipeline(false)
{
    init_shaders();
    m_pathTracer = std::make_shared<PathTracer>(width, height, samples);
    m_SVGFGBuffer = std::make_shared<SVGFGBuffer>(width, height);
    m_SVGFGBuffer_prev = std::make_shared<SVGFGBuffer>(width, height);
    m_colorVarianceBuffer1 = std::make_shared<ColorVarianceBuffer>(width, height);
    m_colorVarianceBuffer2 = std::make_shared<ColorVarianceBuffer>(width, height);
    m_directHistory = std::make_unique<ColorHistoryBuffer>(width, height);
    m_indirectHistory = std::make_unique<ColorHistoryBuffer>(width, height);
    m_displayBuffer = std::make_unique<DisplayBuffer>(width, height);
}


Scene::~Scene()
{
    for(unsigned int i = 0; i < _objects->size(); ++i) {
        Object * o = (*_objects)[i];
        delete o;
    }
    delete _objects;
    delete m_bvh;
    delete lights;
}

bool& Scene::pipeline() {
    return m_pipeline;
}

const bool& Scene::pipeline() const {
    return m_pipeline;
}

void Scene::init_shaders() {
  m_testShader =
      std::make_unique<Shader>(Shader::from_files("test.vert", "test.frag"));
  m_defaultShader = std::make_unique<Shader>(
      Shader::from_files("shader.vert", "shader.frag"));
  m_drawAlphaShader = std::make_unique<Shader>(
      Shader::from_files("quad.vert", "draw_alpha.frag"));
  m_gBufferShader = std::make_unique<Shader>(
      Shader::from_files("gbuffer.vert", "gbuffer.frag"));
  m_motionVectorsShader = std::make_unique<Shader>(
      Shader::from_files("quad.vert", "motion_vectors.frag"));
  m_temporalAccumulationShader = std::make_unique<Shader>(
      Shader::from_files("quad.vert", "temporal_accumulation.frag"));
  m_calcVarianceShader = std::make_unique<Shader>(Shader::from_files("quad.vert", "calc_variance.frag"));
  m_copyMomentsShader = std::make_unique<Shader>(Shader::from_files("quad.vert", "copy_moments.frag"));
  m_waveletShader =
      std::make_unique<Shader>(Shader::from_files("quad.vert", "wavelet.frag"));
  m_updateHistoryShader = std::make_unique<Shader>(Shader::from_files("quad.vert", "update_history.frag"));
  m_colorCopyShader = std::make_unique<Shader>(
      Shader::from_files("quad.vert", "colorcopy.frag"));
  m_reconstructionShader = std::make_unique<Shader>(
      Shader::from_files("quad.vert", "reconstruction.frag"));
  m_fxaaShader = std::make_unique<Shader>(
      Shader::from_files("quad.vert", "fxaa.frag"));
}

std::unique_ptr<Scene> Scene::load(QString filename, int width, int height) {
  CS123XmlSceneParser parser(filename.toStdString());
  if (!parser.parse()) {
    return nullptr;
  }
  CS123SceneCameraData cameraData;
  parser.getCameraData(cameraData);
  QuaternionCamera camera(cameraData.pos.xyz(), cameraData.look.xyz(),
                     cameraData.up.xyz(), cameraData.heightAngle,
                     (float)width / (float)height, 0.1f, 100.f);
  std::unique_ptr <Scene> scene = std::make_unique<Scene>(width, height);
  scene->setCamera(camera);

  CS123SceneGlobalData globalData;
  parser.getGlobalData(globalData);
  scene->setGlobalData(globalData);

  CS123SceneLightData lightData;
  for (int i = 0, size = parser.getNumLights(); i < size; ++i) {
    parser.getLightData(i, lightData);
    scene->addLight(lightData);
  }

  QFileInfo info(filename);
  QString dir = info.path();
  CS123SceneNode *root = parser.getRootNode();
  if (!parseTree(root, *scene, dir.toStdString() + "/")) {
    return nullptr;
  }
  return scene;
}

void Scene::change_settings(int renderMode, int numSamples, int waveletIterations, float alpha, float sigmaP, float sigmaN, float sigmaL) {
    m_reconstructionShader->bind();
    m_reconstructionShader->setUniform("mode", renderMode);
    m_reconstructionShader->unbind();

    m_pathTracer->numSamples(numSamples);

    wavelet_iterations = waveletIterations;

    m_integration_alpha = alpha;


    m_waveletShader->bind();
    m_waveletShader->setUniform("sigmaP", sigmaP);
    m_waveletShader->setUniform("sigmaN", sigmaN);
    m_waveletShader->setUniform("sigmaL", sigmaL);
    m_waveletShader->unbind();
}

RenderBuffers Scene::trace(bool save) {
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    auto buffers = m_pathTracer->traceScene(*this);
    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    float duration = duration_cast<milliseconds>( t2 - t1 ).count() / 1000.0;
    std::cout << "Scene took " << duration << " seconds to trace." << std::endl;
    if (save) {
      buffers.save();
    }
    return buffers;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Scene::resize(int w, int h) {
    width = w;
    height = h;
    QuaternionCamera cam = getCamera();
    cam.setAspectRatio(static_cast<float>(w) / static_cast<float>(h));
    this->setCamera(cam);

    m_pathTracer = std::make_shared<PathTracer>(width, height, m_pathTracer->numSamples());
    m_SVGFGBuffer = std::make_shared<SVGFGBuffer>(width, height);
    m_SVGFGBuffer_prev = std::make_shared<SVGFGBuffer>(width, height);
    m_colorVarianceBuffer1 = std::make_shared<ColorVarianceBuffer>(width, height);
    m_colorVarianceBuffer2 = std::make_shared<ColorVarianceBuffer>(width, height);
    m_directHistory = std::make_unique<ColorHistoryBuffer>(width, height);
    m_indirectHistory = std::make_unique<ColorHistoryBuffer>(width, height);
    m_displayBuffer = std::make_unique<DisplayBuffer>(width, height);
}

void Scene::render() {
    Buffer::unbind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Pipeline:
    if (m_pipeline) {
        // G-buffer
        // INPUT: scene
        // OUTPUT: depth, normals, mesh/mat ids, motion vectors

        // TODO OpenGL's G-Buffer did not match up with pathtracing data
        // m_SVGFGBuffer->bind();
        // glClearColor(0.0, 0.0, 0.0, -1);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glClearColor(0.0, 0.0, 0.0, 0);

        // m_gBufferShader->bind();
        // m_gBufferShader->setUniform("p", m_camera.getProjectionMatrix());
        // m_gBufferShader->setUniform("v", m_camera.getViewMatrix());

        // for (Object *obj : *_objects) {
        //     m_gBufferShader->setUniform("m", obj->transform);
        //     obj->render(m_gBufferShader, m_pipeline);
        // }

        // m_gBufferShader->unbind();
        // m_SVGFGBuffer->unbind();


        // Pathtracing
        // INPUT: scene
        // OUTPUT: direct/indirect lighting color
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        auto buffers = this->trace();
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        ColorBuffer cb = ColorBuffer(width, height, buffers);
        m_SVGFGBuffer->set_textures(buffers);

        // Calc motion vectors
        // This stores texture coordinates into the prevous screen as (u_prev, v_prev, valid) (RGB)
        ResultBuffer motion_vectors(width, height);
        calc_motion_vectors(motion_vectors);

        // Temporal accumulation shader
        // INPUT: color, motion vectors, normals, depth, mesh/mat ids, color history, moments history, prev normals, prev depth, prev mesh/mat ids
        // OUTPUT: integrated color, integrated moments
        ColorHistoryBuffer direct_accumulated(width, height);
        accumulate(*m_directHistory, motion_vectors, cb.getDirectTexture(), direct_accumulated, m_integration_alpha);
        ColorHistoryBuffer indirect_accumulated(width, height);
        accumulate(*m_indirectHistory, motion_vectors, cb.getIndirectTexture(), indirect_accumulated, m_integration_alpha);

        // TODO: Variance estimation
        // INPUT: integrated moments
        // OUTPUT: variance


        // Wavelet filter
        // INPUT: integrated color, variance, luminance
        // OUTPUT: 1st level filtered color, 5th level filtered color
        // Update color and moments history
        // INPUT: 1st level filtered color, integrated moments
        // OUTPUT: color history, moments history

        ResultBuffer direct(width, height);
        ResultBuffer indirect(width, height);

        ColorVarianceBuffer cv_temp(width, height);
        calc_variance(direct_accumulated, cv_temp);
        waveletPass(direct, cv_temp, *m_directHistory, wavelet_iterations);
        calc_variance(indirect_accumulated, cv_temp);
        waveletPass(indirect, cv_temp, *m_indirectHistory, wavelet_iterations);


        // Reconstruction (w/tone mapping)
        // INPUT: direct/indirect lighting, 5th level filtered color
        // OUTPUT: combined light and primary albedo
        this->recombineColor(cb, direct, indirect);

        // Post-processing (fxaa)
        // INPUT: combined light and primary albedo
        // OUTPUT: rendered image

        m_fxaaShader->bind();
        m_fxaaShader->setTexture("color", m_displayBuffer->color_texture());
        renderQuad();
        m_fxaaShader->unbind();

        // motion_vectors.display();

        high_resolution_clock::time_point t3 = high_resolution_clock::now();
        float duration = duration_cast<milliseconds>( t3 - t2 ).count() / 1000.0;
        std::cout << "Scene took " << duration << " seconds to filter." << std::endl;

        // Swap current and previous G buffers, update camera_prev
        // TODO maybe don't do this in render code?
        m_SVGFGBuffer.swap(m_SVGFGBuffer_prev);
        m_camera_prev = m_camera;
    } else {
        // Visualization
        m_defaultShader->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_defaultShader->setUniform("p", m_camera.getProjectionMatrix());
        m_defaultShader->setUniform("v", m_camera.getViewMatrix());

        m_defaultShader->setUniform("light_dir", glm::vec3(0, 0, -1.f));
        m_defaultShader->setUniform("light_color", glm::vec3(0.2f, 0.2f, 0.2f));

        int i = 0;
        for (Object *obj : *_objects) {
            i++;
            m_defaultShader->setUniform("m", obj->transform);
            obj->render(m_defaultShader, m_pipeline);
        }
        m_defaultShader->unbind();
    }
}

void Scene::draw_alpha(const CS123::GL::Texture2D& tex){
  Buffer::unbind();
  m_drawAlphaShader->bind();
  m_drawAlphaShader->setTexture("color", tex);
  renderQuad();
  m_drawAlphaShader->unbind();
  Buffer::unbind();
}

void Scene::calc_motion_vectors(ResultBuffer& out) const {
  if ((m_camera.getViewMatrix() != m_camera_prev.getViewMatrix()) ||
      (m_camera.getProjectionMatrix() != m_camera_prev.getProjectionMatrix())) {
    std::cout << "CHANGE" << std::endl;
  }
  out.bind();
  m_motionVectorsShader->bind();
  m_motionVectorsShader->setUniform("v_prev", m_camera_prev.getViewMatrix());
  m_motionVectorsShader->setUniform("p_prev", m_camera_prev.getProjectionMatrix());
  m_motionVectorsShader->setTexture("pos_id_prev", m_SVGFGBuffer_prev->position_mesh_id_texture());
  m_motionVectorsShader->setTexture("normals_prev", m_SVGFGBuffer_prev->normal_texture());
  m_motionVectorsShader->setTexture("pos_id", m_SVGFGBuffer->position_mesh_id_texture());
  m_motionVectorsShader->setTexture("normals", m_SVGFGBuffer->normal_texture());
  renderQuad();
  m_motionVectorsShader->unbind();
  out.unbind();
}

void Scene::accumulate(ColorHistoryBuffer &history, const ResultBuffer& motion_vectors,
                       const Texture2D &new_color_tex,
                       ColorHistoryBuffer &accumulator, float alpha) {
  accumulator.bind();
  m_temporalAccumulationShader->bind();
  m_temporalAccumulationShader->setUniform("alpha", alpha);
  m_temporalAccumulationShader->setTexture("col_history",
                                           history.color_history());
  m_temporalAccumulationShader->setTexture("motion_vectors",
                                           motion_vectors.color_texture());
  m_temporalAccumulationShader->setTexture("current_color", new_color_tex);
  m_temporalAccumulationShader->setTexture("moments", history.moments());
  renderQuad();
  m_temporalAccumulationShader->unbind();
  accumulator.unbind();

  // TODO update history moments
  accumulator.blit_to(GL_COLOR_ATTACHMENT0, history, GL_COLOR_ATTACHMENT0);
  accumulator.blit_to(GL_COLOR_ATTACHMENT1, history, GL_COLOR_ATTACHMENT1);
}

void Scene::calc_variance(const ColorHistoryBuffer& accumulated, ColorVarianceBuffer& out){
  out.bind();
  m_calcVarianceShader->bind();
  m_calcVarianceShader->setTexture("col_history", accumulated.color_history());
  m_calcVarianceShader->setTexture("moments_history", accumulated.moments());
  renderQuad();
  m_calcVarianceShader->unbind();
  out.unbind();
}

void Scene::waveletPass(ResultBuffer& rb, const Buffer& input_buff, ColorHistoryBuffer& history, int iterations) {
    ColorVarianceBuffer *cvb_input = m_colorVarianceBuffer1.get();
    ColorVarianceBuffer *cvb_output = m_colorVarianceBuffer2.get();
    // Initial color
    input_buff.blit_to(*cvb_input);

    // Blit between two FBOs for colorVariance
    cvb_output->bind();
    m_waveletShader->bind();
    m_waveletShader->setTexture("colorVariance", cvb_input->color_variance_texture());
    m_waveletShader->setTexture("gPositionMeshID",
                                m_SVGFGBuffer->position_mesh_id_texture());
    m_waveletShader->setTexture("gNormal", m_SVGFGBuffer->normal_texture());
    m_waveletShader->setUniform("level", 0);
    renderQuad();

    // update color history with 1st level filtered color
    glColorMaski(history.id(), true, true, true, false);
    cvb_output->blit_to(history);
    glColorMaski(history.id(), true, true, true, true);
    std::swap(cvb_input, cvb_output);

    for (int i = 1; i < iterations; ++i) {
        // non-owning references used to alternate between the two buffers
        cvb_output->bind();
        m_waveletShader->bind();
        m_waveletShader->setUniform("level", i);
        m_waveletShader->setTexture("colorVariance", cvb_input->color_variance_texture());
        renderQuad();
        std::swap(cvb_input, cvb_output);
    }

    // Store 5th level filtered color
    cvb_input->blit_to(rb);
}

void Scene::recombineColor(const ColorBuffer &cb, const ResultBuffer &direct,
                           const ResultBuffer &indirect) {
     m_displayBuffer->bind();
     m_reconstructionShader->bind();
     m_reconstructionShader->setTexture("direct", direct.color_texture());
     m_reconstructionShader->setTexture("indirect", indirect.color_texture());
     m_reconstructionShader->setTexture("albedo", cb.getAlbedoTexture());
     renderQuad();
     m_reconstructionShader->unbind();
     m_displayBuffer->unbind();
}

void Scene::setBVH(const BVH &bvh)
{
    m_bvh = new BVH(bvh);
}

bool Scene::parseTree(CS123SceneNode *root, Scene& scene, const std::string &baseDir)
{
    std::vector<Object *> *objects = new std::vector<Object *>;
    int id = 0;
    parseNode(root, glm::mat4x4(1.f), objects, baseDir, id);

    if(objects->size() == 0) {
        return false;
    }
    std::cout << "Parsed tree. " << objects->size() << " objects. Creating BVH..." << std::endl;
    BVH *bvh = new BVH(objects);

    std::vector<Object *> *lights = new std::vector<Object *>;
    for (Object* obj : *objects) {
        if (obj->isLight) {
            lights->push_back(obj);
        }
    }

    scene.lights = lights;
    scene._objects = objects;
    scene.setBVH(*bvh);
    return true;
}

void Scene::parseNode(CS123SceneNode *node, const glm::mat4x4 &parentTransform, std::vector<Object *> *objects, const std::string &baseDir, int &id)
{
    glm::mat4x4 transform = parentTransform;
    for(CS123SceneTransformation *trans : node->transformations) {
        switch(trans->type) {
        case TRANSFORMATION_TRANSLATE:
            transform = transform * glm::translate(trans->translate);
            break;
        case TRANSFORMATION_SCALE:
            transform = transform * glm::scale(trans->scale);
            break;
        case TRANSFORMATION_ROTATE:
            transform = transform * glm::rotate(trans->angle, trans->rotate);
            break;
        case TRANSFORMATION_MATRIX:
            transform = transform * trans->matrix;
            break;
        }
    }
    for(CS123ScenePrimitive *prim : node->primitives) {
        addPrimitive(prim, transform, objects, baseDir, id);
    }
    for(CS123SceneNode *child : node->children) {
        parseNode(child, transform, objects, baseDir, id);
    }
}

void Scene::addPrimitive(CS123ScenePrimitive *prim, const glm::mat4x4 &transform, std::vector<Object *> *objects, const std::string &baseDir, int &id)
{
    std::vector<Mesh*> objs;
    switch(prim->type) {
    case PrimitiveType::PRIMITIVE_MESH:
        std::cout << "Loading mesh " << prim->meshfile << std::endl;
        objs = loadMesh(prim->meshfile, prim->material, transform, baseDir, id);
        objects->insert(objects->end(), std::begin(objs), std::end(objs));
        std::cout << "Done loading mesh" << std::endl;
        break;
    default:
        std::cerr << "We don't handle any other formats yet" << std::endl;
        break;
    }
}

std::vector<Mesh*> Scene::loadMesh(std::string filePath, const CS123SceneMaterial & material, const glm::mat4x4 &transform, const std::string &baseDir, int &id)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    QFileInfo info(QString((baseDir + filePath).c_str()));
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
                                info.absoluteFilePath().toStdString().c_str(), (info.absolutePath().toStdString() + "/").c_str(), true);
    if(!err.empty()) {
        std::cerr << err << std::endl;
    }

    std::vector<Mesh*> res;
    if(!ret) {
        std::cerr << "Failed to load/parse .obj file" << std::endl;
        return res;
    }

    for(size_t s = 0; s < shapes.size(); s++) {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> colors;
        std::vector<glm::vec2> uvs;
        std::vector<int> materialIds;
        std::vector<glm::ivec3> faces;


        size_t index_offset = 0;
        for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            unsigned int fv = shapes[s].mesh.num_face_vertices[f];

            glm::ivec3 face;
            for(size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3*idx.vertex_index+0];
                tinyobj::real_t vy = attrib.vertices[3*idx.vertex_index+1];
                tinyobj::real_t vz = attrib.vertices[3*idx.vertex_index+2];
                tinyobj::real_t nx;
                tinyobj::real_t ny;
                tinyobj::real_t nz;
                tinyobj::real_t tx;
                tinyobj::real_t ty;

                if(idx.normal_index != -1) {
                    nx = attrib.normals[3*idx.normal_index+0];
                    ny = attrib.normals[3*idx.normal_index+1];
                    nz = attrib.normals[3*idx.normal_index+2];
                } else {
                    nx = 0;
                    ny = 0;
                    nz = 0;
                }
                if(idx.texcoord_index != -1) {
                    tx = attrib.texcoords[2*idx.texcoord_index+0];
                    ty = attrib.texcoords[2*idx.texcoord_index+1];
                } else {
                    tx = 0;
                    ty = 0;
                }

                tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
                tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
                tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];

                face[v] = vertices.size();
//                vertices.push_back(transform * glm::vec3(vx, vy, vz));
//                normals.push_back((transform.linear().inverse().transpose() * glm::vec3(nx, ny, nz)).normalized());
                vertices.push_back(glm::vec3(vx, vy, vz));
                normals.push_back(glm::normalize(glm::vec3(nx, ny, nz)));
                uvs.push_back(glm::vec2(tx, ty));
                colors.push_back(glm::vec3(red, green, blue));
            }
            faces.push_back(face);
            materialIds.push_back(shapes[s].mesh.material_ids[f]);

            index_offset += fv;
        }

        Mesh *m = new Mesh;
        m->init(id++,
                vertices,
                normals,
                uvs,
                colors,
                faces,
                materialIds,
                materials,
                material);
        m->setTransform(transform);

        m->isLight = false;
        for(int matid : materialIds) {
            tinyobj::material_t mat = materials[matid];
            if (mat.emission[0] + mat.emission[1] + mat.emission[2] > 0.f) {
                m->isLight = true;
                break;
            }
        }

        res.push_back(m);
        std::cout << "Loaded " << faces.size() << " faces" << std::endl;
    }

    return res;
}

const BVH &Scene::getBVH() const
{
    return *m_bvh;
}

const QuaternionCamera &Scene::getCamera() const
{
    return m_camera;
}

void Scene::setCamera(const QuaternionCamera &camera)
{
  // m_camera_prev = camera;
    m_camera = camera;
}

void Scene::setGlobalData(const CS123SceneGlobalData& data)
{
    m_globalData = data;
}

void Scene::addLight(const CS123SceneLightData &data)
{
    m_lights.push_back(data);
}
