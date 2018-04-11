#include "pathtracer.h"
#include "util/CS123Common.h"

#include <glm/gtx/norm.hpp>
#include <glm/glm.hpp>

#include <QRunnable>
#include <QThreadPool>

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
    const Scene m_scene;
    RenderBuffers *m_buffs;
    glm::mat4x4 m_invViewMat;
    int m_x0, m_y0, m_x1, m_y1;
protected:
    virtual void run() { m_tracer->render(m_scene, *m_buffs, m_invViewMat, m_x0, m_y0, m_x1, m_y1); }
};


PathTracer::PathTracer(int width, int height, int numSamples)
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
        rt->setAutoDelete(false);
        pool.start(rt);
        yOffset = newYOffset;
    }
    RenderTask *rt = new RenderTask(this, scene, &output_buffs, invViewMat, 0, yOffset, m_width, m_height);
    rt->setAutoDelete(false);
    pool.start(rt);

    pool.waitForDone();

    return output_buffs;
}

void PathTracer::render(const Scene& scene, RenderBuffers& buffs, glm::mat4x4 &invViewMat, int x0, int y0, int x1, int y1) {
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            int offset = x + (y * m_width);
            buffs[offset] = tracePixel(x, y, scene, invViewMat);
        }
    }
}

RenderBuffers::Element PathTracer::tracePixel(int x, int y, const Scene& scene, const glm::mat4x4 &invViewMatrix) {
    glm::vec3 p(0, 0, 0);
    auto res = RenderBuffers::Element::zero();
    for (int s = 0; s < m_numSamples; s++) {
        glm::vec3 d((2.f * (x + random()) / m_width) - 1, 1 - (2.f * (y + random()) / m_height), -1);
        d = glm::normalize(d);

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

            float ndotl = glm::dot(toLight, normal);
            if (ndotl <= 0.f) continue;

            intensity += glm::vec3(mat.emission[0], mat.emission[1], mat.emission[2]) * (light->getSurfaceArea() * ndotl * glm::abs(glm::dot(toLight, glm::normalize(t->getNormal(i)))) / distSquare);
        }
    }

    return intensity;
}

RenderBuffers::Element PathTracer::traceRay(const Ray& r, const Scene& scene, int depth) {
    glm::vec3 l(0, 0, 0);

    IntersectionInfo i;
    if(scene.getBVH().getIntersection(r, &i, false)) {
        const Mesh *m = static_cast<const Mesh *>(i.object); // Get the mesh that was intersected
        const Triangle *t = static_cast<const Triangle *>(i.data); // Get the triangle in the mesh that was intersected
        const tinyobj::material_t& mat = m->getMaterial(t->getIndex()); // Get the material of the triangle from the mesh

        if (m->isLight && depth == 0) {
            auto elem = RenderBuffers::Element::zero();
            elem.m_albedo = glm::vec3(mat.emission[0], mat.emission[1], mat.emission[2]);
            elem.m_direct = glm::vec3(1.0f, 1.0f, 1.0f);
            elem.m_full = elem.m_albedo;
            return elem;
        }
        const glm::vec3 hit = i.hit;
        const glm::vec3 normal = glm::normalize((m->inverseNormalTransform * glm::vec4(t->getNormal(i), 0)).xyz());
        const bool inward = glm::dot(normal, r.d) < 0;
        const glm::vec3 sn = inward ? normal : -normal;

        if (mat.illum == 2) { // Diffuse
            const float pdf_rr = std::min(std::max(mat.diffuse[0], std::max(mat.diffuse[1], mat.diffuse[2])), 0.99f);

            if (random() < pdf_rr) {
                glm::vec3 wi;
                float pdf;
                cosineSampleHemisphere(normal, wi, pdf);

                const glm::vec3 directIllumination = directLighting(hit, normal, scene);
                const glm::vec3 indirectIllumination = traceRay(Ray(hit + FLOAT_EPSILON * wi, wi), scene, depth + 1).m_full;

                const glm::vec3 brdf = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]) / ((float)M_PI);

                const float illum_scale = glm::dot(wi, normal) / (pdf * pdf_rr);
                const glm::vec3 illum = (directIllumination + indirectIllumination) * glm::dot(wi, normal) / (pdf * pdf_rr);

                // TODO still using old code for full, for debugarino
                auto elem = RenderBuffers::Element::zero();
                elem.m_albedo = brdf;
                elem.m_direct = directIllumination * illum_scale;
                elem.m_indirect = indirectIllumination * illum_scale;
                elem.m_full =  glm::vec3(brdf[0]*illum[0], brdf[1]*illum[1], brdf[2]*illum[2]);
                return elem;
            }

        } else if (mat.illum == 3) { // Glossy specular
            const float pdf_rr = 0.95f;
            //std::min(std::max(mat.specular[0], std::max(mat.specular[1], mat.specular[2])), 0.99f);

            if (random() < pdf_rr) {
                glm::vec3 wi;
                float pdf;
                uniformSampleHemisphere(normal, wi, pdf);

                // Phong
//                const glm::vec3 refl = glm::normalize(-wi + 2.f*normal*glm::dot(normal, wi));

//                const glm::vec3 directIllumination = directLighting(hit, normal, scene);

//                const glm::vec3 indirectIllumination = traceRay(Ray(hit + FLOAT_EPSILON * wi, wi), scene, depth + 1);

//                glm::vec3 brdf = glm::vec3(mat.specular) * (mat.shininess + 2.f) * pow(glm::dot(refl, -r.d), mat.shininess) / (2.f * M_PI);
//                CLAMP(brdf[0], 0, 1);
//                CLAMP(brdf[1], 0, 1);
//                CLAMP(brdf[2], 0, 1);

//                const glm::vec3 illum = (directIllumination + indirectIllumination) * glm::dot(wi, normal) / (pdf * pdf_rr);

//                l += glm::vec3(brdf[0]*illum[0], brdf[1]*illum[1], brdf[2]*illum[2]);


                // Cook Torrance
                glm::vec3 wo = -r.d;
                float m = 0.22f;
                glm::vec3 h = glm::normalize(wo + wi);
                float widoth = glm::dot(wi, h);
                float wodoth = glm::dot(wo, h);
                float ndotwi = glm::dot(wi, normal);
                float ndotwo = glm::dot(wo, normal);
                float ndoth = glm::dot(h, normal);
                float d = 1.f / (M_PI*m*m*pow(ndoth, 4)) * std::exp((ndoth*ndoth - 1)/(m*m*ndoth*ndoth));

                float fterm = std::pow(1 - widoth, 5);
                glm::vec3 f = glm::vec3(mat.specular[0] + (1 - mat.specular[0])*fterm, mat.specular[1] + (1 - mat.specular[1])*fterm, mat.specular[2] + (1 - mat.specular[2])*fterm);
                float g = std::min(1.f, std::min(2*ndoth*ndotwo / wodoth, 2*ndoth*ndotwi / wodoth));

                const glm::vec3 directIllumination = directLighting(hit, normal, scene);
                const glm::vec3 indirectIllumination = traceRay(Ray(hit + FLOAT_EPSILON * wi, wi), scene, depth + 1).m_full;

                //pdf included
                glm::vec3 brdf = ((float) M_PI) * d*g*f / (2.f*ndotwo);

                const glm::vec3 illum = (directIllumination + indirectIllumination) / (pdf_rr);
                // TODO m_full
                auto elem = RenderBuffers::Element::zero();
                elem.m_albedo = brdf;
                elem.m_direct = directIllumination / pdf_rr;
                elem.m_indirect = indirectIllumination / pdf_rr;
                elem.m_full = glm::vec3(brdf[0]*illum[0], brdf[1]*illum[1], brdf[2]*illum[2]);
                return elem;
            }

        } else if (mat.illum == 5) { // Perfect specular
            const float pdf_rr = std::min(std::max(mat.specular[0], std::max(mat.specular[1], mat.specular[2])), 0.99f);

            if (random() < pdf_rr) {
                const glm::vec3 refl = glm::normalize(r.d - 2.f * normal * glm::dot(normal, r.d));
                // TODO m_full
                auto elem = RenderBuffers::Element::zero();
                // TODO colored mirrors or nah?
                // elem.m_albedo = glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]);
                elem.m_albedo = glm::vec3(1.0f, 1.0f, 1.0f);
                elem.m_indirect = traceRay(Ray(hit + FLOAT_EPSILON * refl, refl), scene, 0).m_full / pdf_rr;
                elem.m_full = elem.m_albedo * elem.m_indirect;
                return elem;
            }

        } else if (mat.illum == 7) { // Refraction
            const float pdf_rr = 0.99f;

            if (random() < pdf_rr) {
                const glm::vec3 refl = glm::normalize(r.d - 2.f * normal * glm::dot(normal, r.d));

                const float ni = 1.f;
                const float nt = mat.ior;
                const float ratio = inward ? ni / nt : nt / ni;

                const float costheta = glm::dot(r.d, sn);
                const float radicand = 1.f - ratio * ratio * (1.f - costheta*costheta);

                // TODO m_full
                auto elem = RenderBuffers::Element::zero();
                // TODO colored mirrors or nah?
                // elem.m_albedo = glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]);
                elem.m_albedo = glm::vec3(1.0f, 1.0f, 1.0f);
                if (radicand < 0) {
                  elem.m_indirect = traceRay(Ray(hit + FLOAT_EPSILON * refl, refl), scene, 0).m_full / pdf_rr;
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
                        elem.m_indirect = traceRay(Ray(hit + FLOAT_EPSILON * refl, refl), scene, 0).m_full / pdf_rr;
                    } else {
                        elem.m_indirect = traceRay(Ray(hit + FLOAT_EPSILON * refr, refr), scene, 0).m_full / pdf_rr;
                    }
                }
                elem.m_full = elem.m_albedo * elem.m_indirect;
                return elem;
            }
        }
    }
    return RenderBuffers::Element::zero();
}
