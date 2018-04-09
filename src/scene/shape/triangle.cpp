#include "triangle.h"

#include "util/CS123Common.h"
#include "pathtracer/pathtracer.h"
#include <glm/gtx/norm.hpp>

Triangle::Triangle()
{
}

Triangle::Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 n1, glm::vec3 n2, glm::vec3 n3, int index)
    : _v1(v1), _v2(v2), _v3(v3), _n1(n1), _n2(n2), _n3(n3), m_index(index)
{
    _centroid = (_v1 + _v2 + _v3) / 3.f;
    _bbox.setP(_v1);
    _bbox.expandToInclude(_v2);
    _bbox.expandToInclude(_v3);

    m_sa = glm::length(glm::cross(v1 - v3, v2 - v3));
}

bool Triangle::getIntersection(const Ray &ray, IntersectionInfo *intersection) const
{
    //https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    glm::vec3 edge1, edge2, h, s, q;
    float a, f, u, v;
    edge1 = _v2 - _v1;
    edge2 = _v3 - _v1;

    h = glm::cross(ray.d, edge2);
    a = glm::dot(edge1, h);

    if(floatEpsEqual(a, 0)) {
        return false;
    }
    f = 1/a;
    s = ray.o - _v1;
    u = f * glm::dot(s, h);
    if(u < 0.f || u > 1.f) {
        return false;
    }
    q = glm::cross(s, edge1);
    v = f * glm::dot(ray.d, q);
    if(v < 0.f || u + v > 1.f) {
        return false;
    }
    float t = f * glm::dot(edge2, q);
    if(t > FLOAT_EPSILON) {
        intersection->t = t;
        intersection->object = this;
        return true;
    } else {
        return false;
    }
}

glm::vec3 Triangle::sample() const {
    float sr1 = sqrt(PathTracer::random());
    float r2 = PathTracer::random();

    return (1.f - sr1)*_v1 + sr1*(1.f - r2)*_v2 + sr1*r2*_v3;
}

float Triangle::getSurfaceArea() const {
    return m_sa;
}

glm::vec3 Triangle::getNormal(const IntersectionInfo &I) const
{
    //Calculate Barycentric coordinates to get interpolated normal
    glm::vec3 p = I.hit;
    glm::vec3 v0 = _v2 - _v1;
    glm::vec3 v1 = _v3 - _v1;
    glm::vec3 v2 = p - _v1;
    float d00 = glm::dot(v0, v0);
    float d01 = glm::dot(v0, v1);
    float d11 = glm::dot(v1, v1);
    float d20 = glm::dot(v2, v0);
    float d21 = glm::dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.f - v - w;

    glm::vec3 n = glm::cross(v0, v1);
    //If normals weren't loaded from file, calculate them instead (This will be flat shading, not smooth shading)
    glm::vec3 n1 = floatEpsEqual(glm::length2(_n1), 0) ? n : _n1;
    glm::vec3 n2 = floatEpsEqual(glm::length2(_n2), 0) ? n : _n2;
    glm::vec3 n3 = floatEpsEqual(glm::length2(_n3), 0) ? n : _n3;
    return (u * n1 + v * n2 + w * n3);
}

BBox Triangle::getBBox() const
{
    return _bbox;
}

glm::vec3 Triangle::getCentroid() const
{
    return (_v1 + _v2 + _v3) / 3.f;
}

int Triangle::getIndex() const
{
    return m_index;
}
