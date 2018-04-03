/**
 * @file CS123Common.h
 *
 * Contains data structures and macros commonly used in CS123.
 */
#pragma once
#ifndef __CS123COMMON_H__
#define __CS123COMMON_H__

#include <math.h>


//// glu.h in different location on macs
//#ifdef __APPLE__
//#include <glu.h>
//#else
//#include <GL/glu.h>
//#endif

// from http://en.wikipedia.org/wiki/Assertion_(computing)
#define COMPILE_TIME_ASSERT(pred) switch(0){case 0:case pred:;}

typedef float REAL;

#define IMAGE_WIDTH 500
#define IMAGE_HEIGHT 500

#define MIN(a, b) (a) < (b) ? (a) : (b)
#define MAX(a, b) (a) > (b) ? (a) : (b)

// ---------------------
// Common math utilities
// ---------------------

const float FLOAT_EPSILON = 1e-4f;
const double DOUBLE_EPSILON = 1e-8;

inline bool floatEpsEqual(float a, float b) {
    // If the difference between a and b is less than epsilon, they are equal
    return fabs(a - b) < FLOAT_EPSILON;
}

inline bool doubleEpsEqual(double a, double b)
{
    // If the difference between a and b is less than epsilon, they are equal
    return fabs(a - b) < DOUBLE_EPSILON;
}

inline Eigen::Vector4f vec3Tovec4(const Eigen::Vector3f &v, float w) {
    return Eigen::Vector4f(v.x(), v.y(), v.z(), w);
}

#endif
