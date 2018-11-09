#ifndef PATHTRACER_H
#define PATHTRACER_H

#include "BVH/Ray.h"
#include "util/util.h"

#include <QImage>

#include <memory>

class Scene;

class PathTracer
{
public:
    PathTracer(int width, int height, unsigned int numSamples);
    RenderBuffers traceScene(const Scene &scene);
    void render(const Scene& scene, RenderBuffers& buffs, glm::mat4x4 &invViewMat, int x0, int y0, int x1, int y1);

    unsigned int numSamples() const { return m_numSamples; }
    void numSamples(unsigned int s) { m_numSamples = s; }

    unsigned int maxDepth() const { return m_maxDepth; }
    void maxDepth(unsigned int d) { m_maxDepth = d; }

    static float random();

private:
    int m_width, m_height;
    unsigned int m_numSamples, m_maxDepth;


  RenderBuffers::Element tracePixel(int x, int y, const Scene &scene, const glm::mat4x4 &invViewMatrix);
  RenderBuffers::Element traceRay(const Ray& r, const Scene &scene, int depth, bool show_lights = true);
};

#endif // PATHTRACER_H
