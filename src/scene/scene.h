#ifndef SCENE_H
#define SCENE_H

#include <QString>
#include <memory>

#include "BVH/BVH.h"
#include "basiccamera.h"
#include "util/CS123SceneData.h"
#include "shape/mesh.h"

class PathTracer;
class SVGFGBuffer;

namespace CS123 { namespace GL {

    class Shader;
}}

class Scene
{
public:
    Scene(int width, int height);
    virtual ~Scene();

    static std::unique_ptr<Scene> load(QString filename, int width, int height);

    void trace() const;
    void render() const;

    void setBVH(const BVH &bvh);
    const BVH& getBVH() const;

    const BasicCamera& getCamera() const;

    void setCamera(const BasicCamera& camera);
    void setGlobalData(const CS123SceneGlobalData& data);
    void addLight(const CS123SceneLightData& data);

    //const std::vector<CS123SceneLightData>& getLights();
    std::vector<Object *> *lights;

private:
    int width, height;

    BVH *m_bvh;
    std::vector<Object *> *_objects;

    BasicCamera m_camera;
    std::shared_ptr<PathTracer> m_pathTracer;

    std::shared_ptr<SVGFGBuffer> m_SVGFGBuffer;

    std::shared_ptr<CS123::GL::Shader> m_defaultShader, m_gBufferShader, m_waveletShader;


    CS123SceneGlobalData m_globalData;
    std::vector<CS123SceneLightData> m_lights;

    static bool parseTree(CS123SceneNode *root, Scene& scene, const std::string& baseDir);
    static void parseNode(CS123SceneNode *node, const glm::mat4x4 &parentTransform, std::vector<Object *> *objects, const std::string& baseDir);
    static void addPrimitive(CS123ScenePrimitive *prim, const glm::mat4x4 &transform, std::vector<Object *> *objects, const std::string& baseDir);
    static std::vector<Mesh*> loadMesh(std::string filePath, const CS123SceneMaterial &material, const glm::mat4x4 &transform, const std::string& baseDir);
};

#endif // SCENE_H
