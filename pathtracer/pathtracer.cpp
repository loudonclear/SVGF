#include "pathtracer.h"

#include <iostream>
#include <Eigen/Dense>
#include <util/CS123Common.h>
#include <QRunnable>
#include <QThreadPool>


using namespace Eigen;

const bool dof = false;
const int seed = 1;

const float blur = 0.0015f;
const float focus = 0.117f;
const float fov = 2.2f;

#define clamp(x, min, max) { if (x < min) x = min; if (x > max) x = max; }

std::default_random_engine rnd(seed);
std::uniform_real_distribution<float> dis(0, 1);

float PathTracer::random() {
    return dis(rnd);
}

inline unsigned char REAL2byte(REAL f) {
    int i = static_cast<int>((f * 255.0 + 0.5));
    return (i < 0) ? 0 : (i > 255) ? 255 : i;
}

class RenderTask : public QRunnable {
public:
    RenderTask(PathTracer *tracer, const Scene& scene, Vector3f *intensityValues, Matrix4f &invViewMat, int x0, int y0, int x1, int y1) : m_tracer(tracer), m_scene(scene), m_intensityValues(intensityValues), m_invViewMat(invViewMat), m_x0(x0), m_y0(y0), m_x1(x1), m_y1(y1) {}
    PathTracer *m_tracer;
    const Scene m_scene;
    Vector3f *m_intensityValues;
    Matrix4f m_invViewMat;
    int m_x0, m_y0, m_x1, m_y1;
protected:
    virtual void run() { m_tracer->render(m_scene, m_intensityValues, m_invViewMat, m_x0, m_y0, m_x1, m_y1); }
};


PathTracer::PathTracer(int width, int height, int numSamples)
    : m_width(width), m_height(height), m_numSamples(numSamples)
{
}

void PathTracer::traceScene(QRgb *imageData, const Scene& scene) {
    Vector3f *intensityValues = new Vector3f[m_width * m_height];
    Matrix4f invViewMat = (scene.getCamera().getScaleMatrix() * scene.getCamera().getViewMatrix()).inverse();

    QThreadPool pool;
    const int threads = QThread::idealThreadCount();
    pool.setMaxThreadCount(threads);

    const int yBlockSize = m_height / threads;
    int yOffset = 0;

    for (int block = 0; block < threads - 1; block++) {
        int newYOffset = yOffset + yBlockSize;
        RenderTask *rt = new RenderTask(this, scene, intensityValues, invViewMat, 0, yOffset, m_width, newYOffset);
        rt->setAutoDelete(false);
        pool.start(rt);
        yOffset = newYOffset;
    }
    RenderTask *rt = new RenderTask(this, scene, intensityValues, invViewMat, 0, yOffset, m_width, m_height);
    rt->setAutoDelete(false);
    pool.start(rt);

    pool.waitForDone();

    toneMap(imageData, intensityValues);
    delete intensityValues;
}

void PathTracer::render(const Scene& scene, Vector3f *intensityValues, Matrix4f &invViewMat, int x0, int y0, int x1, int y1) {
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            int offset = x + (y * m_width);
            intensityValues[offset] = tracePixel(x, y, scene, invViewMat);
        }
    }
}

Vector3f PathTracer::tracePixel(int x, int y, const Scene& scene, const Matrix4f &invViewMatrix) {
    Vector3f p(0, 0, 0);

    BasicCamera cam = scene.getCamera();

    Vector3f res(0, 0, 0);
    for (int s = 0; s < m_numSamples; s++) {
        Vector3f d((2.f * (x + random()) / m_width) - 1, 1 - (2.f * (y + random()) / m_height), -1);
        d.normalize();

        if (dof) { // Depth of field
            Vector3f lensray = Vector3f(d[0], d[1], fov).normalized();
            Vector3f look = cam.getLook();
            Vector3f up = cam.getUp();
            Vector3f right = look.cross(up);
            Vector3f newdir = lensray[0]*right + lensray[1]*up + lensray[2]*look;

            Vector3f lenspos = blur * Vector3f(-1.f + 2.f * random(), -1.f + 2.f * random(), 0);
            Vector3f lensvec = (lensray * focus - lenspos).normalized();

            Vector3f pos = cam.getPosition() + lenspos[0]*right + lenspos[1]*up;
            Vector3f ra = (newdir + lensvec[0]*right + lensvec[1]*up).normalized();

            Ray r = Ray(pos, ra);
            res += traceRay(r, scene, 0);
        } else {
            res += traceRay(Ray(Ray(p, d).transform(invViewMatrix)), scene, 0);
        }

    }
    res /= m_numSamples;

    return res;
}


Vector3f tangentConvert(const Vector3f& pointdir, const Vector3f& normal) {
    Vector3f tangent = std::fabs(normal[0]) > std::fabs(normal[1]) ? Vector3f(normal[2], 0, -normal[0]) : Vector3f(0, -normal[2], normal[1]);
    tangent.normalize();
    Vector3f bitangent = normal.cross(tangent);

    return Vector3f(pointdir[0] * bitangent[0] + pointdir[1] * normal[0] + pointdir[2] * tangent[0],
                    pointdir[0] * bitangent[1] + pointdir[1] * normal[1] + pointdir[2] * tangent[1],
                    pointdir[0] * bitangent[2] + pointdir[1] * normal[2] + pointdir[2] * tangent[2]).normalized();
}

void uniformSampleHemisphere(const Vector3f& normal, Vector3f &wi, float &pdf) {
    float r1 = PathTracer::random();
    float r2 = PathTracer::random();

    float phi = 2.f * M_PI * r1;
    float sintheta = sqrt(1.f - r2 * r2);

    wi = tangentConvert(Vector3f(sintheta * cos(phi), r2, sintheta * sin(phi)).normalized(), normal);
    pdf = 1.f / (2.f * M_PI);
}

void cosineSampleHemisphere(const Vector3f& normal, Vector3f &wi, float &pdf) {
    float r1 = PathTracer::random();
    float r2 = PathTracer::random();

    float phi = 2.f * M_PI * r1;
    float sr2 = sqrt(r2);

    wi = tangentConvert(Vector3f(sr2 * cos(phi), sqrt(1.f - r2), sr2 * sin(phi)).normalized(), normal);
    pdf = normal.dot(wi) / M_PI;
}

void cosineExpSampleHemisphere(const Vector3f& normal, Vector3f &wi, float &pdf, int n) {
    float r1 = PathTracer::random();
    float r2 = PathTracer::random();

    float phi = 2.f * M_PI * r1;
    float costheta = pow(r2, 1.f / (n + 1));
    float sintheta = sqrt(std::max(0.f, 1.f - costheta * costheta));

    wi = tangentConvert(Vector3f(sintheta*cos(phi), costheta, sintheta*sin(phi)).normalized(), normal);
    pdf = (n + 1) * pow(costheta, n) / (2.f * M_PI);
}



Vector3f directLighting(const Vector3f& hit, const Vector3f& normal, const Scene& scene) {
    Vector3f intensity(0, 0, 0);

    for (Object* light : *scene.lights) {
        Vector3f lightPoint = light->transform * light->sample();

        Vector3f toLight = (lightPoint - hit);
        float distSquare = toLight.squaredNorm();
        toLight.normalize();

        IntersectionInfo i;
        if (scene.getBVH().getIntersection(Ray(hit + toLight*FLOAT_EPSILON, toLight), &i, false) && i.object == light) {
            const Mesh * m = static_cast<const Mesh *>(i.object);
            const Triangle *t = static_cast<const Triangle *>(i.data);
            const tinyobj::material_t& mat = m->getMaterial(t->getIndex());

            float ndotl = toLight.dot(normal);
            if (ndotl <= 0.f) continue;

            intensity += Vector3f(mat.emission) * (light->getSurfaceArea() * ndotl * fabs(toLight.dot(t->getNormal(i).normalized())) / distSquare);
        }
    }

    return intensity;
}

Vector3f PathTracer::traceRay(const Ray& r, const Scene& scene, int depth) {
    Vector3f l(0, 0, 0);

    IntersectionInfo i;
    if(scene.getBVH().getIntersection(r, &i, false)) {
        const Mesh *m = static_cast<const Mesh *>(i.object); // Get the mesh that was intersected
        const Triangle *t = static_cast<const Triangle *>(i.data); // Get the triangle in the mesh that was intersected
        const tinyobj::material_t& mat = m->getMaterial(t->getIndex()); // Get the material of the triangle from the mesh

        if (m->isLight && depth == 0) {
            return Vector3f(mat.emission);
        }



        const Vector3f hit = i.hit;
        const Vector3f normal = (m->inverseNormalTransform * t->getNormal(i)).normalized();
        const bool inward = normal.dot(r.d) < 0;
        const Vector3f sn = inward ? normal : -normal;

        if (mat.illum == 2) { // Diffuse
            const float pdf_rr = std::min(std::max(mat.diffuse[0], std::max(mat.diffuse[1], mat.diffuse[2])), 0.99f);

            if (random() < pdf_rr) {
                Vector3f wi;
                float pdf;
                cosineSampleHemisphere(normal, wi, pdf);

                const Vector3f directIllumination = directLighting(hit, normal, scene);
                const Vector3f indirectIllumination = traceRay(Ray(hit + FLOAT_EPSILON * wi, wi), scene, depth + 1);

                const Vector3f brdf = Vector3f(mat.diffuse) / M_PI;

                const Vector3f illum = (directIllumination + indirectIllumination) * wi.dot(normal) / (pdf * pdf_rr);

                l += Vector3f(brdf[0]*illum[0], brdf[1]*illum[1], brdf[2]*illum[2]);
            }

        } else if (mat.illum == 3) { // Glossy specular
            const float pdf_rr = 0.95f;//std::min(std::max(mat.specular[0], std::max(mat.specular[1], mat.specular[2])), 0.99f);

            if (random() < pdf_rr) {
                Vector3f wi;
                float pdf;
                uniformSampleHemisphere(normal, wi, pdf);

                // Phong
//                const Vector3f refl = (-wi + 2.f*normal*normal.dot(wi)).normalized();

//                const Vector3f directIllumination = directLighting(hit, normal, scene);

//                const Vector3f indirectIllumination = traceRay(Ray(hit + FLOAT_EPSILON * wi, wi), scene, depth + 1);

//                Vector3f brdf = Vector3f(mat.specular) * (mat.shininess + 2.f) * pow(refl.dot(-r.d), mat.shininess) / (2.f * M_PI);
//                clamp(brdf[0], 0, 1);
//                clamp(brdf[1], 0, 1);
//                clamp(brdf[2], 0, 1);

//                const Vector3f illum = (directIllumination + indirectIllumination) * wi.dot(normal) / (pdf * pdf_rr);

//                l += Vector3f(brdf[0]*illum[0], brdf[1]*illum[1], brdf[2]*illum[2]);


                // Cook Torrance
                Vector3f wo = -r.d;
                float m = 0.22f;
                Vector3f h = (wo + wi).normalized();
                float widoth = wi.dot(h);
                float wodoth = wo.dot(h);
                float ndotwi = wi.dot(normal);
                float ndotwo = wo.dot(normal);
                float ndoth = h.dot(normal);


                float d = 1.f / (M_PI*m*m*pow(ndoth, 4)) * std::exp((ndoth*ndoth - 1)/(m*m*ndoth*ndoth));

                float fterm = std::pow(1 - widoth, 5);
                Vector3f f = Vector3f(mat.specular[0] + (1 - mat.specular[0])*fterm, mat.specular[1] + (1 - mat.specular[1])*fterm, mat.specular[2] + (1 - mat.specular[2])*fterm);
                float g = std::min(1.f, std::min(2*ndoth*ndotwo / wodoth, 2*ndoth*ndotwi / wodoth));

                const Vector3f directIllumination = directLighting(hit, normal, scene);
                const Vector3f indirectIllumination = traceRay(Ray(hit + FLOAT_EPSILON * wi, wi), scene, depth + 1);

                //pdf included
                Vector3f brdf = M_PI * d*f*g / (2*ndotwo);

                const Vector3f illum = (directIllumination + indirectIllumination) / (pdf_rr);

                l += Vector3f(brdf[0]*illum[0], brdf[1]*illum[1], brdf[2]*illum[2]);
            }

        } else if (mat.illum == 5) { // Perfect specular
            const float pdf_rr = std::min(std::max(mat.specular[0], std::max(mat.specular[1], mat.specular[2])), 0.99f);

            if (random() < pdf_rr) {
                const Vector3f refl = (r.d - 2.f * normal * normal.dot(r.d)).normalized();
                l += traceRay(Ray(hit + FLOAT_EPSILON * refl, refl), scene, 0) / pdf_rr;
            }

        } else if (mat.illum == 7) { // Refraction
            const float pdf_rr = 0.99f;

            if (random() < pdf_rr) {
                const Vector3f refl = (r.d - 2.f * normal * normal.dot(r.d)).normalized();

                const float ni = 1.f;
                const float nt = mat.ior;
                const float ratio = inward ? ni / nt : nt / ni;

                const float costheta = r.d.dot(sn);
                const float radicand = 1.f - ratio * ratio * (1.f - costheta*costheta);

                if (radicand < 0) {
                    l += traceRay(Ray(hit + FLOAT_EPSILON * refl, refl), scene, 0) / pdf_rr;
                } else {
                    Vector3f refr;
                    if (inward) {
                        refr = r.d * ratio - normal * (costheta * ratio + sqrt(radicand));
                    } else {
                        refr = r.d * ratio + normal * (costheta * ratio + sqrt(radicand));
                    }

                    const float R0 = (nt - ni) * (nt - ni) / ((nt + ni) * (nt + ni));
                    const float Rtheta = R0 + (1.f - R0) * std::pow(1.f - (inward ? -costheta : refr.dot(normal)), 5);

                    if (random() < Rtheta) {
                        l += traceRay(Ray(hit + FLOAT_EPSILON * refl, refl), scene, 0) / pdf_rr;
                    } else {
                        l += traceRay(Ray(hit + FLOAT_EPSILON * refr, refr), scene, 0) / pdf_rr;
                    }

                }
            }

        }

    }

    return l;
}

void PathTracer::toneMap(QRgb *imageData, Vector3f *intensityValues) {
    for(int y = 0; y < m_height; ++y) {
        for(int x = 0; x < m_width; ++x) {
            int offset = x + (y * m_width);
            Vector3f intensityValue = intensityValues[offset];
            imageData[offset] = qRgb(REAL2byte(intensityValue(0) / (intensityValue(0) + 1.f)),
                                     REAL2byte(intensityValue(1) / (intensityValue(1) + 1.f)),
                                     REAL2byte(intensityValue(2) / (intensityValue(2) + 1.f)));
        }
    }
}
