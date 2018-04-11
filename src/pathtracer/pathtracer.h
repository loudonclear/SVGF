#ifndef PATHTRACER_H
#define PATHTRACER_H

#include "scene/scene.h"
#include "util/util.h"

#include <QImage>

#include <memory>

class PathTracer
{
public:
    PathTracer(int width, int height, int numSamples);
    RenderBuffers traceScene(const Scene &scene);
    void render(const Scene& scene, RenderBuffers& buffs, glm::mat4x4 &invViewMat, int x0, int y0, int x1, int y1);
    static float random();

private:
  int m_width, m_height, m_numSamples;


  RenderBuffers::Element tracePixel(int x, int y, const Scene &scene, const glm::mat4x4 &invViewMatrix);
  RenderBuffers::Element traceRay(const Ray& r, const Scene &scene, int depth);
};

#endif // PATHTRACER_H
