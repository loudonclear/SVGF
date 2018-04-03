#ifndef BBox_h
#define BBox_h

#include "Ray.h"
#include "Eigen/Dense"
#include <stdint.h>

struct BBox {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  Eigen::Vector3f min;
  Eigen::Vector3f max;
  Eigen::Vector3f extent;

  void setMinMax(const Eigen::Vector3f& min, const Eigen::Vector3f& max);
  void setP(const Eigen::Vector3f& min);

  bool intersect(const Ray& ray, float *tnear, float *tfar) const;
  void expandToInclude(const Eigen::Vector3f& p);
  void expandToInclude(const BBox& b);
  uint32_t maxDimension() const;
  float surfaceArea() const;
};

#endif
