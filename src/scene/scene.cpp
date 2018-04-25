#include "scene.h"

#include "gl/shaders/Shader.h"
#include "gl/textures/Texture2D.h"
#include "gl/util/ColorBuffer.h"
#include "gl/util/ColorVarianceBuffer.h"
#include "gl/util/FullScreenQuad.h"
#include "gl/util/ResourceLoader.h"
#include "gl/util/SVGFGBuffer.h"
#include "gl/util/ResultBuffer.h"
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

Scene::Scene(int width, int height, unsigned int samples) : width(width), height(height), m_pipeline(false)
{
    init_shaders();
    m_pathTracer = std::make_shared<PathTracer>(width, height, samples);
    m_SVGFGBuffer = std::make_shared<SVGFGBuffer>(width, height);
    m_SVGFGBuffer_prev = std::make_shared<SVGFGBuffer>(width, height);
    m_colorVarianceBuffer1 = std::make_shared<ColorVarianceBuffer>(width, height);
    m_colorVarianceBuffer2 = std::make_shared<ColorVarianceBuffer>(width, height);
    m_directHistory = std::make_unique<ColorHistoryBuffer>(width, height);
    m_indirectHistory = std::make_unique<ColorHistoryBuffer>(width, height);
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
  m_gBufferShader = std::make_unique<Shader>(
      Shader::from_files("gbuffer.vert", "gbuffer.frag"));
  m_temporalAccumulationShader = std::make_unique<Shader>(
      Shader::from_files("quad.vert", "temporal_accumulation.frag"));
  m_waveletHorizontalShader = std::make_unique<Shader>(
      Shader::from_files("quad.vert", "hwavelet.frag"));
  m_waveletVerticalShader = std::make_unique<Shader>(
      Shader::from_files("quad.vert", "vwavelet.frag"));
  m_waveletShader =
      std::make_unique<Shader>(Shader::from_files("quad.vert", "wavelet.frag"));
  m_initColorLumaShader = std::make_unique<Shader>(
      Shader::from_files("quad.vert", "colorluma.frag"));
  m_reconstructionShader = std::make_unique<Shader>(
      Shader::from_files("quad.vert", "reconstruction.frag"));
  m_updateHistoryShader = std::make_unique<Shader>(Shader::from_files("quad.vert", "update_history.frag"));
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

RenderBuffers Scene::trace(bool save) {
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    auto buffers = m_pathTracer->traceScene(*this);
    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    float duration = duration_cast<milliseconds>( t2 - t1 ).count() / 1000.0;
    std::cout << "Scene took " << duration << " seconds to render." << std::endl;
    if (save) {
      save_render_buffers(buffers);
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


void Scene::render() {
    Buffer::unbind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Pipeline:
    if (m_pipeline) {
      high_resolution_clock::time_point t1 = high_resolution_clock::now();
        // G-buffer
        // INPUT: scene
        // OUTPUT: depth, normals, mesh/mat ids, motion vectors

        m_SVGFGBuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_gBufferShader->bind();
        m_gBufferShader->setUniform("p", m_camera.getProjectionMatrix());
        m_gBufferShader->setUniform("v", m_camera.getViewMatrix());

        for (Object *obj : *_objects) {
            m_gBufferShader->setUniform("m", obj->transform);
            obj->render(m_gBufferShader, m_pipeline);
        }

        m_gBufferShader->unbind();
        m_SVGFGBuffer->unbind();


        // Pathtracing
        // INPUT: scene
        // OUTPUT: direct/indirect lighting color

        auto buffers = this->trace();
        ColorBuffer cb = ColorBuffer(width, height, buffers);

        // TODO: Temporal accumulation shader
        // INPUT: color, motion vectors, normals, depth, mesh/mat ids, color history, moments history, prev normals, prev depth, prev mesh/mat ids
        // OUTPUT: integrated color, integrated moments
        float integration_alpha = 0.2;
        ColorVarianceBuffer direct_accumulated(width, height);
        ColorVarianceBuffer indirect_accumulated(width, height);
        accumulate(*m_directHistory, cb.getDirectTexture(), direct_accumulated, integration_alpha);
        accumulate(*m_indirectHistory, cb.getIndirectTexture(), indirect_accumulated, integration_alpha);


        // TODO: Variance estimation
        // INPUT: integrated moments
        // OUTPUT: variance


        // TODO: Wavelet filter
        // INPUT: integrated color, variance, luminance
        // OUTPUT: 1st level filtered color, 5th level filtered color
        bool separate = true;
        ResultBuffer direct(width, height);
        ResultBuffer indirect(width, height);
        waveletPass(direct, direct_accumulated.color_variance_texture(), *m_directHistory, 5, separate);
        waveletPass(indirect, indirect_accumulated.color_variance_texture(), *m_indirectHistory, 5, separate);

        // TODO: Update color and moments history
        // INPUT: 1st level filtered color, integrated moments
        // OUTPUT: color history, moments history


        // TODO: Reconstruction
        // INPUT: direct/indirect lighting, 5th level filtered color
        // OUTPUT: combined light and primary albedo
        this->recombineColor(cb, direct, indirect);


        // TODO: Post-processing (tone mapping, temporal antialiasing)
        // INPUT: combined light and primary albedo
        // OUTPUT: rendered image

        std::cout << "End frame" << std::endl;
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        float duration = duration_cast<milliseconds>( t2 - t1 ).count() / 1000.0;
        std::cout << "Scene took " << duration << " seconds to filter." << std::endl;

        // Swap current and previous G buffers
        m_SVGFGBuffer.swap(m_SVGFGBuffer_prev);
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

void Scene::flip_rgba_texture(const CS123::GL::Texture2D &tex, Buffer &output_buff) {
  output_buff.bind();
  m_initColorLumaShader->bind();
  m_initColorLumaShader->setTexture("color", tex);
  renderQuad();
  m_initColorLumaShader->unbind();
  output_buff.unbind();
}

void Scene::accumulate(ColorHistoryBuffer &history,
                       const Texture2D &new_color_tex,
                       ColorVarianceBuffer &accumulator, float alpha) {
  accumulator.bind();
  m_temporalAccumulationShader->bind();
  m_temporalAccumulationShader->setUniform("alpha", alpha);
  m_temporalAccumulationShader->setTexture("col_history",
                                           history.color_history());
  m_temporalAccumulationShader->setTexture("current_color", new_color_tex);
  m_temporalAccumulationShader->setTexture("moments", history.moments());
  renderQuad();
  m_temporalAccumulationShader->unbind();
  accumulator.unbind();
}

void Scene::waveletPass(ResultBuffer& rb, const Texture2D& texture, ColorHistoryBuffer& history, int iterations, bool separate) {
    // Initial color and luma
    flip_rgba_texture(texture, *m_colorVarianceBuffer1);

    if (separate) {
      // Blit between two FBOs for colorVariance
        m_colorVarianceBuffer2->bind();
        m_waveletHorizontalShader->bind();
        m_waveletHorizontalShader->setTexture(
            "colorVariance", m_colorVarianceBuffer1->color_variance_texture());
        m_waveletHorizontalShader->setTexture("gDepthIDs", m_SVGFGBuffer->depth_ids_texture());
        m_waveletHorizontalShader->setTexture("gNormal", m_SVGFGBuffer->normal_texture());
        m_waveletHorizontalShader->setUniform("level", 0);
        renderQuad();

        m_colorVarianceBuffer1->bind();
        m_waveletVerticalShader->bind();
        m_waveletVerticalShader->setTexture(
            "colorVariance", m_colorVarianceBuffer2->color_variance_texture());
        m_waveletVerticalShader->setTexture("gDepthIDs", m_SVGFGBuffer->depth_ids_texture());
        m_waveletVerticalShader->setTexture("gNormal", m_SVGFGBuffer->normal_texture());
        m_waveletVerticalShader->setUniform("level", 0);
        renderQuad();
    }
    else {
        m_colorVarianceBuffer2->bind();
        m_waveletShader->bind();
        m_waveletShader->setTexture("colorVariance", m_colorVarianceBuffer1->color_variance_texture());
        m_waveletShader->setTexture("gDepthIDs",
                                    m_SVGFGBuffer->depth_ids_texture());
        m_waveletShader->setTexture("gNormal", m_SVGFGBuffer->normal_texture());
        m_waveletShader->setUniform("level", 0);
        renderQuad();
    }

    // TODO XXX non-separated filter uses colorvariancebufer2 instead of 1
    // TODO: Store 1st level filtered color
    this->flip_rgba_texture(m_colorVarianceBuffer1->color_variance_texture(), history);

    if (separate) {
      for (int i = 1; i < iterations; i++) {
        m_colorVarianceBuffer2->bind();
        m_waveletHorizontalShader->bind();
        m_waveletHorizontalShader->setUniform("level", i);
        m_waveletHorizontalShader->setTexture("colorVariance", m_colorVarianceBuffer1->color_variance_texture());
        renderQuad();

        m_colorVarianceBuffer1->bind();
        m_waveletVerticalShader->bind();
        m_waveletVerticalShader->setUniform("level", i);
        m_waveletVerticalShader->setTexture("colorVariance", m_colorVarianceBuffer2->color_variance_texture());
        renderQuad();
      }
    } else {
      for (int i = 1; i < iterations; ++i) {
        // non-owning references used to alternate between the two buffers
        ColorVarianceBuffer *cvb_input, *cvb_output;
        if (i % 2 == 0) {
          cvb_input = m_colorVarianceBuffer2.get();
          cvb_output = m_colorVarianceBuffer1.get();
        } else {
          cvb_input = m_colorVarianceBuffer1.get();
          cvb_output = m_colorVarianceBuffer2.get();
        }

        cvb_input->bind();
        m_waveletShader->bind();
        m_waveletShader->setUniform("level", i);
        m_waveletShader->setTexture("colorVariance", cvb_output->color_variance_texture());
        renderQuad();
      }
    }
    this->flip_rgba_texture(m_colorVarianceBuffer1->color_variance_texture(), rb);
}

void Scene::recombineColor(const ColorBuffer &cb, const ResultBuffer &direct,
                           const ResultBuffer &indirect) {
     // output to screen
     Buffer::unbind();
     m_reconstructionShader->bind();
     m_reconstructionShader->setTexture("direct", direct.color_texture());
     m_reconstructionShader->setTexture("indirect", indirect.color_texture());
     m_reconstructionShader->setTexture("albedo", cb.getAlbedoTexture());
     renderQuad();
     m_reconstructionShader->unbind();
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

//const std::vector<CS123SceneLightData> &Scene::getLights()
//{
//    return m_lights;
//}
