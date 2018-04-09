#ifndef IntersectionInfo_h_
#define IntersectionInfo_h_

#include <glm.hpp>

class Object;

struct IntersectionInfo {
  float t; // Intersection distance along the ray
  const Object* object; // Object that was hit
  glm::vec3 hit; // Location of the intersection
  const void *data = nullptr;
};

#endif
