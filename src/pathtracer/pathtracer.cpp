#include "pathtracer.h"
#include "BVH/Ray.h"
#include "scene/scene.h"
#include "util/CS123Common.h"

#include <glm/gtx/norm.hpp>
#include <glm/glm.hpp>

#include <QRunnable>
#include <QThreadPool>
#include <QCoreApplication>

#include <random>
#include <iostream>

const int seed = 1;

#define CLAMP(x, min, max) { if (x < min) x = min; if (x > max) x = max; }

std::default_random_engine rnd(seed);
std::uniform_real_distribution<float> dis(0, 1);

float PathTracer::random() {
    return dis(rnd);
}

class RenderTask : public QRunnable {
public:
    RenderTask(PathTracer *tracer, const Scene& scene, RenderBuffers *buffs, glm::mat4x4 &invViewMat, int x0, int y0, int x1, int y1) : m_tracer(tracer), m_scene(scene), m_buffs(buffs), m_invViewMat(invViewMat), m_x0(x0), m_y0(y0), m_x1(x1), m_y1(y1) {}
    PathTracer *m_tracer;
    const Scene& m_scene;
    RenderBuffers *m_buffs;
    glm::mat4x4 m_invViewMat;
    int m_x0, m_y0, m_x1, m_y1;
protected:
    virtual void run() { m_tracer->render(m_scene, *m_buffs, m_invViewMat, m_x0, m_y0, m_x1, m_y1); }
};


PathTracer::PathTracer(int width, int height, unsigned int numSamples)
    : m_width(width), m_height(height), m_numSamples(numSamples)
{
}

RenderBuffers PathTracer::traceScene(const Scene& scene) {
    RenderBuffers output_buffs(m_width, m_height);
    glm::mat4x4 invViewMat = glm::inverse(scene.getCamera().getScaleMatrix() * scene.getCamera().getViewMatrix());

    QThreadPool pool;
    const int threads = QThread::idealThreadCount();
    pool.setMaxThreadCount(threads);

    const int yBlockSize = m_height / threads;
    int yOffset = 0;

    for (int block = 0; block < threads - 1; block++) {
        int newYOffset = yOffset + yBlockSize;
        RenderTask *rt = new RenderTask(this, scene, &output_buffs, invViewMat, 0, yOffset, m_width, newYOffset);
        rt->setAutoDelete(true);
        pool.start(rt);
        yOffset = newYOffset;
    }
    RenderTask *rt = new RenderTask(this, scene, &output_buffs, invViewMat, 0, yOffset, m_width, m_height);
    rt->setAutoDelete(true);
    pool.start(rt);

    pool.waitForDone();

    return output_buffs;
}

void PathTracer::render(const Scene& scene, RenderBuffers& buffs, glm::mat4x4 &invViewMat, int x0, int y0, int x1, int y1) {
  std::cout << "SAMPLES: " << m_numSamples << std::endl;
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            int offset = x + ((m_height - y - 1) * m_width);
            auto pix_elem = tracePixel(x, y, scene, invViewMat);
            glm::clamp(pix_elem.m_layers.at(RenderBuffers::INDIRECT), glm::vec3(0.f), glm::vec3(0.85f));
            glm::clamp(pix_elem.m_layers.at(RenderBuffers::DIRECT), glm::vec3(0.f), glm::vec3(0.85f));

            buffs[offset] = pix_elem;
        }
    }
}

RenderBuffers::Element PathTracer::tracePixel(int x, int y, const Scene& scene, const glm::mat4x4 &invViewMatrix) {
    glm::vec3 p(0, 0, 0);
    glm::vec3 d((2.f * (x) / m_width) - 1, 1 - (2.f * (y) / m_height), -1);
    d = glm::normalize(d);
    auto res = RenderBuffers::Element::zero();
    for (unsigned int s = 0; s < m_numSamples; s++) {
        res += traceRay(Ray(Ray(p, d).transform(invViewMatrix)), scene, 0);
    }
    res /= m_numSamples;
    return res;
}


glm::vec3 tangentConvert(const glm::vec3& pointdir, const glm::vec3& normal) {
    glm::vec3 tangent = std::fabs(normal[0]) > std::fabs(normal[1]) ? glm::vec3(normal[2], 0, -normal[0]) : glm::vec3(0, -normal[2], normal[1]);
    tangent = glm::normalize(tangent);
    glm::vec3 bitangent = glm::cross(normal, tangent);

    return glm::normalize(glm::vec3(pointdir[0] * bitangent[0] + pointdir[1] * normal[0] + pointdir[2] * tangent[0],
                    pointdir[0] * bitangent[1] + pointdir[1] * normal[1] + pointdir[2] * tangent[1],
                    pointdir[0] * bitangent[2] + pointdir[1] * normal[2] + pointdir[2] * tangent[2]));
}

void uniformSampleHemisphere(const glm::vec3& normal, glm::vec3 &wi, float &pdf) {
    float r1 = PathTracer::random();
    float r2 = PathTracer::random();

    float phi = 2.f * M_PI * r1;
    float sintheta = sqrt(1.f - r2 * r2);

    wi = tangentConvert(glm::normalize(glm::vec3(sintheta * cos(phi), r2, sintheta * sin(phi))), normal);
    pdf = 1.f / (2.f * M_PI);
}

void cosineSampleHemisphere(const glm::vec3& normal, glm::vec3 &wi, float &pdf) {
    float r1 = PathTracer::random();
    float r2 = PathTracer::random();

    float phi = 2.f * M_PI * r1;
    float sr2 = sqrt(r2);

    wi = tangentConvert(glm::normalize(glm::vec3(sr2 * cos(phi), sqrt(1.f - r2), sr2 * sin(phi))), normal);
    pdf = glm::dot(normal, wi) / M_PI;
}

void cosineExpSampleHemisphere(const glm::vec3& normal, glm::vec3 &wi, float &pdf, int n) {
    float r1 = PathTracer::random();
    float r2 = PathTracer::random();

    float phi = 2.f * M_PI * r1;
    float costheta = pow(r2, 1.f / (n + 1));
    float sintheta = sqrt(std::max(0.f, 1.f - costheta * costheta));

    wi = tangentConvert(glm::normalize(glm::vec3(sintheta*cos(phi), costheta, sintheta*sin(phi))), normal);
    pdf = (n + 1) * pow(costheta, n) / (2.f * M_PI);
}


// TODO: Check for bugs
glm::vec3 directLighting(const glm::vec3& hit, const glm::vec3& normal, const Scene& scene) {
    glm::vec3 intensity(0, 0, 0);

    for (Object* light : *scene.lights) {
        glm::vec3 lightPoint = (light->transform * glm::vec4(light->sample(), 1)).xyz();

        glm::vec3 toLight = (lightPoint - hit);
        float distSquare = glm::length2(toLight);
        toLight = glm::normalize(toLight);

        IntersectionInfo i;
        if (scene.getBVH().getIntersection(Ray(hit + toLight*FLOAT_EPSILON, toLight), &i, false) && i.object == light) {
            const Mesh * m = static_cast<const Mesh *>(i.object);
            const Triangle *t = static_cast<const Triangle *>(i.data);
            const tinyobj::material_t& mat = m->getMaterial(t->getIndex());

            if (glm::dot(toLight, t->getNormal(i)) > 0.f) continue;
            float ndotl = glm::clamp(glm::dot(toLight, normal), 0.f, 1.f);

            intensity += glm::vec3(mat.emission[0], mat.emission[1], mat.emission[2]) * (light->getSurfaceArea() * ndotl * glm::abs(glm::dot(toLight, glm::normalize(t->getNormal(i)))) / distSquare);
        }
    }

    return intensity;
}

const int minDepth = 1;
const int maxDepth = 4;
RenderBuffers::Element PathTracer::traceRay(const Ray& r, const Scene& scene, int depth, bool show_lights) {
    if (depth >= maxDepth){
        return RenderBuffers::Element::zero();
    }
    IntersectionInfo i;
    if(scene.getBVH().getIntersection(r, &i, false)) {
        auto elem = RenderBuffers::Element::zero();

        const Mesh *m = static_cast<const Mesh *>(i.object); // Get the mesh that was intersected
        const Triangle *t = static_cast<const Triangle *>(i.data); // Get the triangle in the mesh that was intersected
        const tinyobj::material_t& mat = m->getMaterial(t->getIndex()); // Get the material of the triangle from the mesh

        const glm::vec3 hit = i.hit;
        const glm::vec3 normal = glm::normalize((m->inverseNormalTransform * glm::vec4(t->getNormal(i), 0)).xyz());
        const bool inward = glm::dot(normal, r.d) < 0;
        const glm::vec3 sn = inward ? normal : -normal;
        elem[RenderBuffers::POSITION] = hit;
        elem[RenderBuffers::NORMALS] = sn;
        elem[RenderBuffers::OBJ_ID] = glm::vec3(m->id());

        if (m->isLight && show_lights) {
            elem[RenderBuffers::ALBEDO] = glm::vec3(mat.emission[0], mat.emission[1], mat.emission[2]);
            elem[RenderBuffers::DIRECT] = glm::vec3(1.0f, 1.0f, 1.0f);
            elem[RenderBuffers::FULL] = elem[RenderBuffers::ALBEDO];
            return elem;
        }
        if (mat.illum == 2) { // Diffuse
            const float pdf_rr = depth < minDepth ? 1.f : std::min(std::max(mat.diffuse[0], std::max(mat.diffuse[1], mat.diffuse[2])), 0.99f);
            elem[RenderBuffers::ALBEDO] = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]) / ((float)M_PI);
            if (random() < pdf_rr) {
                glm::vec3 wi;
                float pdf;
                cosineSampleHemisphere(normal, wi, pdf);

                const float illum_scale = glm::dot(wi, normal) / (pdf * pdf_rr);
                elem[RenderBuffers::DIRECT] = directLighting(hit, normal, scene) * illum_scale;
                elem[RenderBuffers::INDIRECT] = traceRay(Ray(hit + FLOAT_EPSILON * wi, wi),
                                                         scene, depth + 1, false)[RenderBuffers::FULL] * illum_scale;
                elem[RenderBuffers::FULL] = elem[RenderBuffers::ALBEDO] * (elem[RenderBuffers::DIRECT] + elem[RenderBuffers::INDIRECT]);
            }
            return elem;
        } else if (mat.illum == 5) { // Perfect specular
            elem[RenderBuffers::ALBEDO] = glm::vec3(1.0f, 1.0f, 1.0f);
            const float pdf_rr = depth < minDepth ? 1.f : std::min(std::max(mat.specular[0], std::max(mat.specular[1], mat.specular[2])), 0.99f);
            if (random() < pdf_rr) {
              glm::vec3 refl =
                  glm::normalize(r.d - 2.f * normal * glm::dot(normal, r.d));
              if (glm::dot(normal, r.d) >= 0.f) refl = r.d;
              elem[RenderBuffers::INDIRECT] = traceRay(Ray(hit + FLOAT_EPSILON * refl, refl), scene, depth + 1, true)[RenderBuffers::FULL] / pdf_rr;
              elem[RenderBuffers::FULL] = elem[RenderBuffers::ALBEDO] * elem[RenderBuffers::INDIRECT];
            }
            return elem;

        } else if (mat.illum == 7) { // Refraction
            elem[RenderBuffers::ALBEDO] = glm::vec3(1.0f, 1.0f, 1.0f);
            const float pdf_rr = depth < minDepth ? 1.f : 0.95f;
            if (random() < pdf_rr) {
                const glm::vec3 refl = glm::normalize(r.d - 2.f * normal * glm::dot(normal, r.d));
                const float ni = 1.f;
                const float nt = mat.ior;
                const float ratio = inward ? ni / nt : nt / ni;

                const float costheta = glm::dot(r.d, sn);
                const float radicand = 1.f - ratio * ratio * (1.f - costheta*costheta);

                // TODO m_full
                if (radicand < FLOAT_EPSILON) {
                  elem[RenderBuffers::INDIRECT] = traceRay(Ray(hit + FLOAT_EPSILON * refl, refl), scene, depth + 1, true)[RenderBuffers::FULL] / pdf_rr;
                } else {
                    glm::vec3 refr;
                    if (inward) {
                        refr = r.d * ratio - normal * (costheta * ratio + glm::sqrt(radicand));
                    } else {
                        refr = r.d * ratio + normal * (costheta * ratio + glm::sqrt(radicand));
                    }
                    const float R0 = (nt - ni) * (nt - ni) / ((nt + ni) * (nt + ni));
                    const float Rtheta = R0 + (1.f - R0) * std::pow(1.f - (inward ? -costheta : glm::dot(refr, normal)), 5);
                    if (random() < Rtheta) {
                      elem[RenderBuffers::INDIRECT] = traceRay(Ray(hit + FLOAT_EPSILON * refl, refl), scene, depth + 1, true)[RenderBuffers::FULL] / pdf_rr;
                    } else {
                      elem[RenderBuffers::INDIRECT] = traceRay(Ray(hit + FLOAT_EPSILON * refr, refr), scene, depth + 1, true)[RenderBuffers::FULL] / pdf_rr;
                    }
                }
                elem[RenderBuffers::FULL] = elem[RenderBuffers::ALBEDO] * elem[RenderBuffers::INDIRECT];
            }
            return elem;
        }
    }
    auto elem = RenderBuffers::Element::zero();
    elem[RenderBuffers::OBJ_ID] = glm::vec3(-1.0);
    return elem;
}
