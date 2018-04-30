#ifndef SCENE_H
#define SCENE_H

#include "QuaternionCamera.h"

#include "BVH/BVH.h"
#include "gl/util/ColorBuffer.h"
#include "gl/util/ColorHistoryBuffer.h"
#include "gl/util/ColorVarianceBuffer.h"
#include "gl/util/ResultBuffer.h"
#include "gl/util/SVGFGBuffer.h"
#include "shape/mesh.h"
#include "util/CS123SceneData.h"
#include "util/util.h"

#include <QString>

#include <memory>

class PathTracer;

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
    void resize(int w, int h);
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

    float& integration_alpha() {return m_integration_alpha; }
    const float& integration_alpha() const {return m_integration_alpha; }

    //const std::vector<CS123SceneLightData>& getLights();
    std::vector<Object *> *lights;

private:
    int width, height;

    BVH *m_bvh;
    std::vector<Object *> *_objects;

    QuaternionCamera m_camera, m_camera_prev;
    std::shared_ptr<PathTracer> m_pathTracer;

    std::shared_ptr<SVGFGBuffer> m_SVGFGBuffer, m_SVGFGBuffer_prev;
    std::shared_ptr<ColorVarianceBuffer> m_colorVarianceBuffer1, m_colorVarianceBuffer2;
    std::unique_ptr<ColorHistoryBuffer> m_directHistory, m_indirectHistory;

    std::shared_ptr<CS123::GL::Shader> m_testShader;
    std::shared_ptr<CS123::GL::Shader> m_calcVarianceShader, m_colorCopyShader,
        m_copyMomentsShader, m_defaultShader, m_drawAlphaShader, m_fxaaShader,
        m_gBufferShader, m_motionVectorsShader, m_reconstructionShader,
        m_temporalAccumulationShader, m_updateHistoryShader, m_waveletShader;
    bool m_pipeline;

    // for debug
    std::vector<std::unique_ptr<glm::vec3[]>> luma_history;

    CS123SceneGlobalData m_globalData;
    std::vector<CS123SceneLightData> m_lights;

    // filtering params
    unsigned int samples;
    float m_integration_alpha = 0.2;

    // convenience function for loading all the shaders.
    void init_shaders();

    /* Code for Running Shaders  */
    // void copy_texture_color(const CS123::GL::Texture2D& tex, Buffer& output_buff);
    void draw_alpha(const CS123::GL::Texture2D& tex);
    void flip_rgba_texture(const CS123::GL::Texture2D& tex, Buffer& output_buff);
    // Calculate motion vectors from the previous frame.
    void calc_motion_vectors(ResultBuffer& out) const;
    // Combine history and new frame to create integrated color & moments
    void accumulate(ColorHistoryBuffer &history,
                    const ResultBuffer &motion_vectors,
                    const CS123::GL::Texture2D &new_color_tex,
                    ColorHistoryBuffer &accumulator, float alpha);
    void calc_variance(const ColorHistoryBuffer& accumulated, ColorVarianceBuffer& out);

    // Does spatial wavelet filtering using several iterations with increasing footprint.
    // rb - the buffer in which to store the result
    // intput_buff - the buffer containing the texture to filter. Currently uses GL_COLOR_ATTACHMENT0
    // iterations - how many iterations of filters to apply
    // separate - whether to use one horizontal and one vertical filter, or to use one huge 2d filter.
    void waveletPass(ResultBuffer& rb, const Buffer& input_buff, ColorHistoryBuffer& history, int iterations);
    void recombineColor(const ColorBuffer& cb, const ResultBuffer& direct, const ResultBuffer& indirect);

  /* Scene BVH code  */

    static bool parseTree(CS123SceneNode *root, Scene& scene, const std::string& baseDir);
    static void parseNode(CS123SceneNode *node, const glm::mat4x4 &parentTransform, std::vector<Object *> *objects, const std::string& baseDir, int &id);
    static void addPrimitive(CS123ScenePrimitive *prim, const glm::mat4x4 &transform, std::vector<Object *> *objects, const std::string& baseDir, int &id);
    static std::vector<Mesh*> loadMesh(std::string filePath, const CS123SceneMaterial &material, const glm::mat4x4 &transform, const std::string& baseDir, int &id);
};

#endif // SCENE_H
