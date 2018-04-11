#ifndef QUATERNIONCAMERA_H
#define QUATERNIONCAMERA_H


#include <glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

/**
 * @class QuaternionCamera
 *
 * You are NOT responsible for understanding the inner workings of this class. This class is provided
 * for (optional) use as a black box for the Sceneview camera. This camera behaves just as your Camtrans camera
 * should once completed (Be aware Camtrans does not allow certain functions such as glm::translate, glm::rotate
 * and glm::perspective. Refer to the Camtrans handout for more details).
 *
 * If you are interested in learning more about Quaternions and/or modern camera implementations you may want
 * to read Quaternion Calculus and Fast Animation by Ken Shoemake (http://www.cs.ucr.edu/~vbz/resources/quatut.pdf).
 */
class QuaternionCamera {
private:
    void rebuildViewMatrix();
    void rebuildProjectionMatrix();

    glm::mat4x4 m_viewMatrix;
    glm::mat4x4 m_projectionMatrix;

    glm::vec4 m_eye;
    glm::quat m_rot;

    float m_heightAngle;
    float m_aspectRatio;
    float m_near;
    float m_far;

public:
    QuaternionCamera(glm::vec3 position, glm::vec3 direction, glm::vec3 up, float heightAngle, float aspect, float near, float far) : m_heightAngle(heightAngle), m_aspectRatio(aspect), m_near(near), m_far(far) {
        orientLook(glm::vec4(position, 1.f),
                   glm::vec4(direction, 0.f),
                   glm::vec4(up, 0.f));
    }
    QuaternionCamera() : m_heightAngle(60), m_aspectRatio(1), m_near(1), m_far(30) {
        orientLook(glm::vec4(0.f, 0.f, 2.f, 0.f),
                   glm::vec4(0.f, 0.f, -1.f, 0.f),
                   glm::vec4(0.f, 1.f, 0.f, 0.f));
    }
    ~QuaternionCamera() {}

    glm::mat4x4 getViewMatrix() const;
    glm::mat4x4 getProjectionMatrix() const;
    glm::mat4x4 getScaleMatrix() const;

    glm::vec4 getPosition() const;
    glm::vec4 getLook() const;
    glm::vec4 getUp() const;
    glm::vec4 getRight() const;
    float getAspectRatio() const;
    float getHeightAngle() const;

    void orientLook(const glm::vec4 &eye, const glm::vec4 &look, const glm::vec4 &up);

    void setHeightAngle(float h);
    void setAspectRatio(float a);
    void translate(const glm::vec4 &v);
    void rotate(float degrees, const glm::vec3 &v);
    void rotateU(float degrees);
    void rotateV(float degrees);
    void rotateW(float degrees);
    void setClip(float nearPlane, float farPlane);
};

#endif // QUATERNIONCAMERA_H
