#ifndef IntersectionInfo_h_
#define IntersectionInfo_h_

#include <Eigen/Dense>

class Object;

struct IntersectionInfo {
  float t; // Intersection distance along the ray
  const Object* object; // Object that was hit
  Eigen::Vector3f hit; // Location of the intersection
  const void *data = nullptr;
};

#endif
