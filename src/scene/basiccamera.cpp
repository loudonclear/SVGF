#include "basiccamera.h"

#include <iostream>

BasicCamera::BasicCamera(glm::vec3 position, glm::vec3 direction, glm::vec3 up, float heightAngle, float aspect)
    : m_position(position), m_direction(direction), m_up(up),
      m_heightAngle(heightAngle), m_aspectRatio(aspect)
{
}

glm::mat4x4 BasicCamera::getViewMatrix() const
{
    glm::vec3 f = glm::normalize(m_direction);
    glm::vec3 u = glm::normalize(m_up);
    glm::vec3 s = glm::cross(f, u);
    u = glm::cross(s, f);

    glm::mat4x4 Result = glm::transpose(glm::mat4x4(s.x, s.y, s.z, -glm::dot(s, m_position),
                                                    u.x, u.y, u.z, -glm::dot(u, m_position),
                                                    -f.x, -f.y, -f.z, glm::dot(f, m_position),
                                                    0, 0, 0, 1));
    return Result;
}

glm::mat4x4 BasicCamera::getScaleMatrix() const
{
    float heightAngleRads = M_PI * m_heightAngle / 360.f;//We need half the angle
    float tanThetaH = tan(heightAngleRads);
    float tanThetaW = m_aspectRatio * tanThetaH;

    glm::mat4x4 scale = glm::mat4x4(1.f);
    scale[0][0] = 1 / tanThetaW;
    scale[1][1] = 1 / tanThetaH;
    return scale;
}

glm::vec3 BasicCamera::getLook() const {
    return glm::normalize(m_direction);
}

glm::vec3 BasicCamera::getUp() const {
    return glm::normalize(m_up);
}

glm::vec3 BasicCamera::getPosition() const {
    return m_position;
}
