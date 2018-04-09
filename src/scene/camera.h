#ifndef CAMERA_H
#define CAMERA_H

#include <glm.hpp>

class Camera
{
public:
    Camera() {}

    virtual glm::mat4x4 getViewMatrix() const = 0;
    virtual glm::mat4x4 getScaleMatrix() const = 0;

    virtual glm::vec3 getLook() const = 0;
    virtual glm::vec3 getUp() const = 0;
    virtual glm::vec3 getPosition() const = 0;
};

#endif // CAMERA_H
