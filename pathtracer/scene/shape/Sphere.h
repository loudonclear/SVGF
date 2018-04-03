#ifndef Sphere_h_
#define Sphere_h_

#include <cmath>
#include <BVH/Object.h>

#include <Eigen/Dense>

//! For the purposes of demonstrating the BVH, a simple sphere
struct Sphere : public Object {
  Eigen::Vector3f center; // Center of the sphere
  float r, r2; // Radius, Radius^2

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  void setCenter(const Eigen::Vector3f& center) {
      this->center = center;
  }
  void setRadius(float radius) {
      r = radius;
      r2 = radius * radius;
  }

  bool getIntersection(const Ray& ray, IntersectionInfo* I) const {
    Eigen::Vector3f s = center - ray.o;
    float sd = s.dot(ray.d);
    float ss = s.dot(s);

    // Compute discriminant
    float disc = sd*sd - ss + r2;

    // Complex values: No intersection
    if( disc < 0.f ) return false;

    // Assume we are not in a sphere... The first hit is the lesser valued
    I->object = this;
    I->t = sd - sqrt(disc);
    return true;
  }

  Eigen::Vector3f getNormal(const IntersectionInfo& I) const {
    return (I.hit - center).normalized();
  }

  BBox getBBox() const {
      BBox box;
      box.setMinMax(center-Eigen::Vector3f(r,r,r), center+Eigen::Vector3f(r,r,r));
      return box;
  }

  Eigen::Vector3f getCentroid() const {
    return center;
  }

};

#endif
