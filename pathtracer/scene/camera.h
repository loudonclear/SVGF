#ifndef CAMERA_H
#define CAMERA_H

#include <Eigen/Dense>

class Camera
{
public:
    Camera();

    virtual Eigen::Matrix4f getViewMatrix() const = 0;
    virtual Eigen::Matrix4f getScaleMatrix() const = 0;

    virtual Eigen::Vector3f getLook() const = 0;
    virtual Eigen::Vector3f getUp() const = 0;
    virtual Eigen::Vector3f getPosition() const = 0;
};

#endif // CAMERA_H
