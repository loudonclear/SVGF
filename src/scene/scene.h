#ifndef SCENE_H
#define SCENE_H

#include "QuaternionCamera.h"

#include "BVH/BVH.h"
#include "gl/util/ColorBuffer.h"
#include "shape/mesh.h"
#include "util/CS123SceneData.h"
#include "util/util.h"

#include <QString>

#include <memory>

class PathTracer;
class SVGFGBuffer;
class ColorVarianceBuffer;
class ResultBuffer;

namespace CS123 { namespace GL {

    class Shader;
}}

class Scene
{
public:
    Scene(int width, int height, unsigned int samples = 1);
    virtual ~Scene();

    static std::unique_ptr<Scene> load(QString filename, int width, int height);

    RenderBuffers trace(bool save = false);
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

    std::shared_ptr<SVGFGBuffer> m_SVGFGBuffer, m_SVGFGBuffer_prev;
    std::shared_ptr<ColorVarianceBuffer> m_colorVarianceBuffer1, m_colorVarianceBuffer2;
    std::unique_ptr<ColorVarianceBuffer> m_colorVarianceHistory;

    std::shared_ptr<CS123::GL::Shader> m_testShader;
    std::shared_ptr<CS123::GL::Shader> m_defaultShader, m_gBufferShader, m_temporalShader, m_waveletHorizontalShader, m_waveletVerticalShader, m_waveletShader, m_initColorLumaShader, m_reconstructionShader;
    bool m_pipeline;

    CS123SceneGlobalData m_globalData;
    std::vector<CS123SceneLightData> m_lights;

    // Does spatial wavelet filtering using several iterations with increasing footprint.
    // rb - the buffer in which to store the result
    // texture - the texture to filter
    // iterations - how many iterations of filters to apply
    // separate - whether to use one horizontal and one vertical filter, or to use one huge 2d filter.
    void waveletPass(ResultBuffer& rb, unsigned int texture, int iterations, bool separate=true);
    void recombineColor(const ColorBuffer& cb, const ResultBuffer& direct, const ResultBuffer& indirect);

    static bool parseTree(CS123SceneNode *root, Scene& scene, const std::string& baseDir);
    static void parseNode(CS123SceneNode *node, const glm::mat4x4 &parentTransform, std::vector<Object *> *objects, const std::string& baseDir, int &id);
    static void addPrimitive(CS123ScenePrimitive *prim, const glm::mat4x4 &transform, std::vector<Object *> *objects, const std::string& baseDir, int &id);
    static std::vector<Mesh*> loadMesh(std::string filePath, const CS123SceneMaterial &material, const glm::mat4x4 &transform, const std::string& baseDir, int &id);
};

#endif // SCENE_H
