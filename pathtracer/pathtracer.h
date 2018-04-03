#ifndef PATHTRACER_H
#define PATHTRACER_H

#include <QImage>

#include "scene/scene.h"


class PathTracer
{
public:
    PathTracer(int width, int height, int numSamples);

    void traceScene(QRgb *imageData, const Scene &scene);
    void render(const Scene& scene, Eigen::Vector3f *intensityValues, Eigen::Matrix4f &invViewMat, int x0, int y0, int x1, int y1);
    static float random();

private:
    int m_width, m_height, m_numSamples;

    void toneMap(QRgb *imageData, Eigen::Vector3f *intensityValues);

    Eigen::Vector3f tracePixel(int x, int y, const Scene &scene, const Eigen::Matrix4f &invViewMatrix);
    Eigen::Vector3f traceRay(const Ray& r, const Scene &scene, int depth);
};

#endif // PATHTRACER_H
