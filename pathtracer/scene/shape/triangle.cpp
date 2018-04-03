#include "triangle.h"

#include "util/CS123Common.h"
#include "pathtracer.h"

using namespace Eigen;

Triangle::Triangle()
{
}

Triangle::Triangle(Vector3f v1, Vector3f v2, Vector3f v3, Vector3f n1, Vector3f n2, Vector3f n3, int index)
    : _v1(v1), _v2(v2), _v3(v3), _n1(n1), _n2(n2), _n3(n3), m_index(index)
{
    _centroid = (_v1 + _v2 + _v3) / 3.f;
    _bbox.setP(_v1);
    _bbox.expandToInclude(_v2);
    _bbox.expandToInclude(_v3);

    m_sa = ((v1 - v3).cross(v2 - v3)).norm();
}

bool Triangle::getIntersection(const Ray &ray, IntersectionInfo *intersection) const
{
    //https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    Vector3f edge1, edge2, h, s, q;
    float a, f, u, v;
    edge1 = _v2 - _v1;
    edge2 = _v3 - _v1;

    h = ray.d.cross(edge2);
    a = edge1.dot(h);

    if(floatEpsEqual(a, 0)) {
        return false;
    }
    f = 1/a;
    s = ray.o - _v1;
    u = f * s.dot(h);
    if(u < 0.f || u > 1.f) {
        return false;
    }
    q = s.cross(edge1);
    v = f * ray.d.dot(q);
    if(v < 0.f || u + v > 1.f) {
        return false;
    }
    float t = f * edge2.dot(q);
    if(t > FLOAT_EPSILON) {
        intersection->t = t;
        intersection->object = this;
        return true;
    } else {
        return false;
    }
}

Vector3f Triangle::sample() const {
    float sr1 = sqrt(PathTracer::random());
    float r2 = PathTracer::random();

    return (1.f - sr1)*_v1 + sr1*(1.f - r2)*_v2 + sr1*r2*_v3;
}

float Triangle::getSurfaceArea() const {
    return m_sa;
}

Vector3f Triangle::getNormal(const IntersectionInfo &I) const
{
    //Calculate Barycentric coordinates to get interpolated normal
    Vector3f p = I.hit;
    Vector3f v0 = _v2 - _v1;
    Vector3f v1 = _v3 - _v1;
    Vector3f v2 = p - _v1;
    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d11 = v1.dot(v1);
    float d20 = v2.dot(v0);
    float d21 = v2.dot(v1);
    float denom = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.f - v - w;

    Vector3f n = v0.cross(v1);
    //If normals weren't loaded from file, calculate them instead (This will be flat shading, not smooth shading)
    Vector3f n1 = floatEpsEqual(_n1.squaredNorm(), 0) ? n : _n1;
    Vector3f n2 = floatEpsEqual(_n2.squaredNorm(), 0) ? n : _n2;
    Vector3f n3 = floatEpsEqual(_n3.squaredNorm(), 0) ? n : _n3;
    return (u * n1 + v * n2 + w * n3);
}

BBox Triangle::getBBox() const
{
    return _bbox;
}

Vector3f Triangle::getCentroid() const
{
    return (_v1 + _v2 + _v3) / 3.f;
}

int Triangle::getIndex() const
{
    return m_index;
}
