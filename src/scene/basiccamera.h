#ifndef BASICCAMERA_H
#define BASICCAMERA_H

#include "camera.h"

class BasicCamera : public Camera
{
public:
    BasicCamera(){}
    BasicCamera(glm::vec3 position, glm::vec3 direction, glm::vec3 up, float heightAngle, float aspect);

    virtual glm::mat4x4 getViewMatrix() const;
    virtual glm::mat4x4 getScaleMatrix() const;

    virtual glm::vec3 getLook() const;
    virtual glm::vec3 getUp() const;
    virtual glm::vec3 getPosition() const;
private:
    glm::vec3 m_position;
    glm::vec3 m_direction;
    glm::vec3 m_up;

   float m_heightAngle;
   float m_aspectRatio;
};

#endif // BASICCAMERA_H
