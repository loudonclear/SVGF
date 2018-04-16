#ifndef SCENE_H
#define SCENE_H

#include <QString>
#include <memory>

#include "BVH/BVH.h"
#include "QuaternionCamera.h"
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
    Scene(int width, int height, unsigned int samples = 1);
    virtual ~Scene();

    static std::unique_ptr<Scene> load(QString filename, int width, int height);

    void trace();
    void render();
    bool& pipeline();
    const bool& pipeline() const;

    void setBVH(const BVH &bvh);
    const BVH& getBVH() const;

    const QuaternionCamera& getCamera() const;

    void setCamera(const QuaternionCamera &camera);
    void setGlobalData(const CS123SceneGlobalData& data);
    void addLight(const CS123SceneLightData& data);

    PathTracer &pathTracer() { return *m_pathTracer; }
    const PathTracer &pathTracer() const { return *m_pathTracer; }

    //const std::vector<CS123SceneLightData>& getLights();
    std::vector<Object *> *lights;

private:
    int width, height;

    BVH *m_bvh;
    std::vector<Object *> *_objects;

    QuaternionCamera m_camera;
    std::shared_ptr<PathTracer> m_pathTracer;

    std::shared_ptr<SVGFGBuffer> m_SVGFGBuffer;

    std::shared_ptr<CS123::GL::Shader> m_testShader;
    std::shared_ptr<CS123::GL::Shader> m_defaultShader, m_gBufferShader, m_temporalShader, m_waveletShader;
    bool m_pipeline;

    CS123SceneGlobalData m_globalData;
    std::vector<CS123SceneLightData> m_lights;

    static bool parseTree(CS123SceneNode *root, Scene& scene, const std::string& baseDir);
    static void parseNode(CS123SceneNode *node, const glm::mat4x4 &parentTransform, std::vector<Object *> *objects, const std::string& baseDir, int &id);
    static void addPrimitive(CS123ScenePrimitive *prim, const glm::mat4x4 &transform, std::vector<Object *> *objects, const std::string& baseDir, int &id);
    static std::vector<Mesh*> loadMesh(std::string filePath, const CS123SceneMaterial &material, const glm::mat4x4 &transform, const std::string& baseDir, int &id);
};

#endif // SCENE_H
