#include "QuaternionCamera.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"


void QuaternionCamera::rebuildViewMatrix() {
    m_viewMatrix = glm::transpose(glm::toMat4(m_rot)) * glm::translate(-m_eye.xyz());
}

void QuaternionCamera::rebuildProjectionMatrix() {
    m_projectionMatrix = glm::perspective(glm::radians(m_heightAngle), m_aspectRatio, m_near, m_far) / m_far;
}


glm::mat4x4 QuaternionCamera::getProjectionMatrix() const {
    return m_projectionMatrix;
}

glm::mat4x4 QuaternionCamera::getScaleMatrix() const {

    float far_recip = 1.f / m_far;
    float far_tan_fov_recip = far_recip / tan(glm::radians(m_heightAngle * 0.5f));

    return glm::mat4x4(far_tan_fov_recip / m_aspectRatio, 0, 0, 0,
                       0, far_tan_fov_recip, 0, 0,
                       0, 0, far_recip, 0,
                       0, 0, 0, 1);
}

glm::mat4x4 QuaternionCamera::getViewMatrix() const {
    return m_viewMatrix;
}

glm::vec4 QuaternionCamera::getPosition() const {
    return m_eye;
}

glm::vec4 QuaternionCamera::getLook() const {
    return glm::rotate(m_rot, glm::vec4(0, 0, -1, 0));
}

glm::vec4 QuaternionCamera::getUp() const {
    return glm::rotate(m_rot, glm::vec4(0, 1, 0, 0));
}

glm::vec4 QuaternionCamera::getRight() const {
    return glm::rotate(m_rot, glm::vec4(1, 0, 0, 0));
}

float QuaternionCamera::getAspectRatio() const {
    return m_aspectRatio;
}

float QuaternionCamera::getHeightAngle() const {
    return m_heightAngle;
}


void QuaternionCamera::orientLook(const glm::vec4 &eye, const glm::vec4 &look, const glm::vec4 &up) {
    m_eye = eye;
    m_rot = glm::toQuat(glm::transpose(glm::lookAt(glm::vec3(0), look.xyz(), up.xyz())));

    rebuildViewMatrix();
    rebuildProjectionMatrix();
}


void QuaternionCamera::setHeightAngle(float h) {
    m_heightAngle = h;
    rebuildProjectionMatrix();
}

void QuaternionCamera::setAspectRatio(float a) {
    m_aspectRatio = a;
    rebuildProjectionMatrix();
}

void QuaternionCamera::translate(const glm::vec4 &v) {
    m_eye += v;
    rebuildViewMatrix();
}


void QuaternionCamera::rotate(float degrees, const glm::vec3 &v) {
    m_rot = glm::rotate(m_rot, glm::radians(degrees), v);
    rebuildViewMatrix();
}

void QuaternionCamera::rotateU(float degrees) {
    rotate(degrees, glm::vec3(1, 0, 0));
}

void QuaternionCamera::rotateV(float degrees) {
    rotate(degrees, glm::vec3(0, 1, 0));
}

void QuaternionCamera::rotateW(float degrees) {
    rotate(degrees, glm::vec3(0, 0, 1));
}

void QuaternionCamera::setClip(float nearPlane, float farPlane) {
    m_near = nearPlane;
    m_far = farPlane;
    rebuildProjectionMatrix();
}
