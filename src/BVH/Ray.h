#ifndef Ray_h
#define Ray_h

#include <glm.hpp>
#include "util/CS123Common.h"

struct Ray {

    Ray(const glm::vec3 o, const glm::vec3 d)
        :o(o), d(glm::normalize(d)), inv_d(glm::normalize(glm::vec3(1.f / d.x, 1.f / d.y, 1.f / d.z))) {
    }

    Ray transform(glm::mat4x4 mat) const {
        glm::vec4 oo = mat * glm::vec4(o, 1);
        glm::vec4 od = mat * glm::vec4(d, 0);
        return Ray(oo.xyz(), od.xyz());
    }


  glm::vec3 o; // Ray Origin
  glm::vec3 d; // Ray Direction
  glm::vec3 inv_d; // Inverse of each Ray Direction component
};

#endif
