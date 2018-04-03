#include "basiccamera.h"

#include <iostream>

using namespace Eigen;

BasicCamera::BasicCamera(Vector3f position, Vector3f direction, Vector3f up, float heightAngle, float aspect)
    : m_position(position), m_direction(direction), m_up(up),
      m_heightAngle(heightAngle), m_aspectRatio(aspect)
{
}

Matrix4f BasicCamera::getViewMatrix() const
{
    Vector3f f = m_direction.normalized();
    Vector3f u = m_up.normalized();
    Vector3f s = f.cross(u);
    u = s.cross(f);

    Matrix4f Result;
    Result << s.x(),s.y(),s.z(), -s.dot(m_position),
            u.x(),u.y(),u.z(), -u.dot(m_position),
            -f.x(),-f.y(),-f.z(), f.dot(m_position),
            0, 0, 0, 1;
    return Result;
}

Matrix4f BasicCamera::getScaleMatrix() const
{
    float heightAngleRads = M_PI * m_heightAngle / 360.f;//We need half the angle
    float tanThetaH = tan(heightAngleRads);
    float tanThetaW = m_aspectRatio * tanThetaH;

    Matrix4f scale = Matrix4f::Identity();
    scale(0, 0) = 1 / tanThetaW;
    scale(1, 1) = 1 / tanThetaH;
    return scale;
}

Vector3f BasicCamera::getLook() const {
    return m_direction.normalized();
}

Vector3f BasicCamera::getUp() const {
    return m_up.normalized();
}

Vector3f BasicCamera::getPosition() const {
    return m_position;
}
