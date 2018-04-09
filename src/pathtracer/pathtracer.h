#ifndef PATHTRACER_H
#define PATHTRACER_H

#include <QImage>

#include "scene/scene.h"


class PathTracer
{
public:
    PathTracer(int width, int height, int numSamples);

    void traceScene(QRgb *imageData, const Scene &scene);
    void render(const Scene& scene, glm::vec3 *intensityValues, glm::mat4x4 &invViewMat, int x0, int y0, int x1, int y1);
    static float random();

private:
    int m_width, m_height, m_numSamples;

    void toneMap(QRgb *imageData, glm::vec3 *intensityValues);

    glm::vec3 tracePixel(int x, int y, const Scene &scene, const glm::mat4x4 &invViewMatrix);
    glm::vec3 traceRay(const Ray& r, const Scene &scene, int depth);
};

#endif // PATHTRACER_H
