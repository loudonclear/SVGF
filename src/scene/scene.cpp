#include "scene.h"

#include "gl/shaders/Shader.h"
#include <util/CS123XmlSceneParser.h>
#include <util/CS123Common.h>
#include <QImage>
#include <iostream>
#include <chrono>

#define TINYOBJLOADER_IMPLEMENTATION
#include "util/tiny_obj_loader.h"

#include "glm/gtx/transform.hpp"
#include "glm/gtx/string_cast.hpp"

#include "pathtracer/pathtracer.h"
#include "gl/util/ResourceLoader.h"
#include "gl/util/SVGFGBuffer.h"

using namespace std;
using namespace std::chrono;

Scene::Scene(int width, int height) : width(width), height(height)
{
    m_defaultShader = std::make_unique<Shader>(ResourceLoader::loadResourceFileToString(":/shaders/shader.vert"), ResourceLoader::loadResourceFileToString(":/shaders/shader.frag"));
    m_defaultShader->setUniform("useLighting", false);


    m_pathTracer = std::make_shared<PathTracer>(width, height, 1);

    m_SVGFGBuffer = std::make_shared<SVGFGBuffer>(width, height);

    // Currently just horizontal
    m_waveletShader = std::make_unique<Shader>(ResourceLoader::loadResourceFileToString(":/shaders/wavelet.vert"), ResourceLoader::loadResourceFileToString(":/shaders/wavelet.frag"));
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

std::unique_ptr<Scene> Scene::load(QString filename, int width, int height) {
  CS123XmlSceneParser parser(filename.toStdString());
  if (!parser.parse()) {
    return nullptr;
  }
  CS123SceneCameraData cameraData;
  parser.getCameraData(cameraData);
  BasicCamera camera(cameraData.pos.xyz(), cameraData.look.xyz(),
                     cameraData.up.xyz(), cameraData.heightAngle,
                     (float)width / (float)height);
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

void Scene::trace() const {
    QImage image(width, height, QImage::Format_RGB32);
    QRgb *data = reinterpret_cast<QRgb *>(image.bits());

    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    m_pathTracer->traceScene(data, *this);
    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    float duration = duration_cast<milliseconds>( t2 - t1 ).count() / 1000.0;
    cout << "Scene took " << duration << " seconds to render." << endl;

    bool success = image.save(QFileInfo("../res/results/output.png").absoluteFilePath());
    if(!success) {
        success = image.save("../res/results/output.png", "PNG");
    }
    if(success) {
        cout << "Wrote rendered image " << endl;
    } else {
        cerr << "Error: failed to write image " << endl;
    }
}

void Scene::render() const {
    m_defaultShader->bind();
    m_defaultShader->setUniform("p", m_camera.getScaleMatrix());
    m_defaultShader->setUniform("v", m_camera.getViewMatrix());

    //std::cout << glm::to_string(m_camera.getLook()) << std::endl;

    // TEST
    for (Object *obj : *_objects) {
        m_defaultShader->setUniform("m", obj->transform);
        m_defaultShader->setUniform("ambient_color", glm::vec3(0.2f, 0.2f, 0.2f));
        m_defaultShader->setUniform("diffuse_color", glm::vec3(0.2f, 0.2f, 0.2f));
        obj->render();
    }
    std::cout<<"Tracing..." << std::endl;
    trace();
    std::cout<<"Done!" << std::endl;


    // Pipeline:

    // TODO: Render G-buffer and 1spp direct/indirect light

    // TODO: Temporal shader

    // TODO: Wavelet filter

    // TODO: Post-processing

    m_defaultShader->unbind();
}

void Scene::setBVH(const BVH &bvh)
{
    m_bvh = new BVH(bvh);
}

bool Scene::parseTree(CS123SceneNode *root, Scene& scene, const std::string &baseDir)
{
    std::vector<Object *> *objects = new std::vector<Object *>;
    parseNode(root, glm::mat4x4(1.f), objects, baseDir);
    if(objects->size() == 0) {
        return false;
    }
    std::cout << "Parsed tree, creating BVH" << std::endl;
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

void Scene::parseNode(CS123SceneNode *node, const glm::mat4x4 &parentTransform, std::vector<Object *> *objects, const std::string &baseDir)
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
        addPrimitive(prim, transform, objects, baseDir);
    }
    for(CS123SceneNode *child : node->children) {
        parseNode(child, transform, objects, baseDir);
    }
}

void Scene::addPrimitive(CS123ScenePrimitive *prim, const glm::mat4x4 &transform, std::vector<Object *> *objects, const std::string &baseDir)
{
    std::vector<Mesh*> objs;
    switch(prim->type) {
    case PrimitiveType::PRIMITIVE_MESH:
        std::cout << "Loading mesh " << prim->meshfile << std::endl;
        objs = loadMesh(prim->meshfile, prim->material, transform, baseDir);
        objects->insert(objects->end(), std::begin(objs), std::end(objs));
        std::cout << "Done loading mesh" << std::endl;
        break;
    default:
        std::cerr << "We don't handle any other formats yet" << std::endl;
        break;
    }
}

std::vector<Mesh*> Scene::loadMesh(std::string filePath, const CS123SceneMaterial & material, const glm::mat4x4 &transform, const std::string &baseDir)
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

    //TODO populate vectors and use tranform
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
        m->init(vertices,
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

const BasicCamera &Scene::getCamera() const
{
    return m_camera;
}

void Scene::setCamera(const BasicCamera &camera)
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
