#ifndef Object_h_
#define Object_h_

#include "IntersectionInfo.h"
#include "Ray.h"
#include "BBox.h"
#include <memory>

#include "gl/shaders/Shader.h"
#include "util/CS123SceneData.h"

class Object {
public:
  Object() {
    transform = inverseTransform = normalTransform = inverseNormalTransform =
        glm::mat4x4(1.f);
  }
  virtual ~Object() {}
  //! All "Objects" must be able to test for intersections with rays.
  virtual bool getIntersection(const Ray &ray,
                               IntersectionInfo *intersection) const = 0;

  virtual void render(std::shared_ptr<CS123::GL::Shader> &shader, const bool pipeline) const = 0;

  virtual glm::vec3 sample() const = 0;

  virtual float getSurfaceArea() const = 0;

  //! Return an object normal based on an intersection
  virtual glm::vec3 getNormal(const IntersectionInfo &I) const = 0;

  //! Return a bounding box for this object
  virtual BBox getBBox() const = 0;

  //! Return the centroid for this object. (Used in BVH Sorting)
  virtual glm::vec3 getCentroid() const = 0;

  bool isLight;
  CS123SceneMaterial material;

  virtual void setTransform(const glm::mat4x4 transform) {
    this->transform = transform;
    this->inverseTransform = glm::inverse(transform);
    this->normalTransform = glm::mat4x4(glm::mat3x3(transform));
    this->inverseNormalTransform =
        glm::mat4x4(glm::transpose(glm::inverse(glm::mat3x3(transform))));
  }

  glm::mat4x4 transform;
  glm::mat4x4 normalTransform;
  glm::mat4x4 inverseTransform;
  glm::mat4x4 inverseNormalTransform;
};

#endif
