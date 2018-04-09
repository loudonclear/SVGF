#include "BBox.h"
#include <algorithm>
#include "vector3.h"

#include <glm/gtx/component_wise.hpp>
#include <iostream>


void BBox::setMinMax(const glm::vec3& min, const glm::vec3& max)
{
    this->min = min;
    this->max = max;
    extent = max - min;
}

void BBox::setP(const glm::vec3& p) {
    min = p;
    max = p;
    extent = max - min;
}

void BBox::expandToInclude(const glm::vec3& p) {
    min = glm::vec3(glm::min(min.x, p.x), glm::min(min.y, p.y), glm::min(min.z, p.z));
    max = glm::vec3(glm::max(max.x, p.x), glm::max(max.y, p.y), glm::max(max.z, p.z));
    extent = max - min;
}

void BBox::expandToInclude(const BBox& b) {
    min = glm::vec3(glm::min(min.x, b.min.x), glm::min(min.y, b.min.y), glm::min(min.z, b.min.z));
    max = glm::vec3(glm::max(max.x, b.max.x), glm::max(max.y, b.max.y), glm::max(max.z, b.max.z));
    extent = max - min;
}

uint32_t BBox::maxDimension() const {
    uint32_t result = 0;
    if(extent[1] > extent[0]) result = 1;
    if(extent[2] > extent[1]) result = 2;
    return result;
}

float BBox::surfaceArea() const {
    return 2.f*( extent[0]*extent[2] + extent[0]*extent[1] + extent[1]*extent[2] );
}

// http://www.flipcode.com/archives/SSE_RayBox_Intersection_Test.shtml
// turn those verbose intrinsics into something readable.
#define loadps(mem)		_mm_load_ps((const float * const)(mem))
#define storess(ss,mem)		_mm_store_ss((float * const)(mem),(ss))
#define minss			_mm_min_ss
#define maxss			_mm_max_ss
#define minps			_mm_min_ps
#define maxps			_mm_max_ps
#define mulps			_mm_mul_ps
#define subps			_mm_sub_ps
#define rotatelps(ps)		_mm_shuffle_ps((ps),(ps), 0x39)	// a,b,c,d -> b,c,d,a
#define muxhps(low,high)	_mm_movehl_ps((low),(high))	// low{a,b,c,d}|high{e,f,g,h} = {c,d,g,h}
static const float flt_plus_inf = -logf(0);	// let's keep C and C++ compilers happy.
static const float __attribute__((aligned(16)))
ps_cst_plus_inf[4] = {  flt_plus_inf,  flt_plus_inf,  flt_plus_inf,  flt_plus_inf },
ps_cst_minus_inf[4] = { -flt_plus_inf, -flt_plus_inf, -flt_plus_inf, -flt_plus_inf };
bool BBox::intersect(const Ray& ray, float *tnear, float *tfar) const {
    Vector3 _min(min[0], min[1], min[2]);
    Vector3 _max(max[0], max[1], max[2]);
    Vector3 _o(ray.o[0], ray.o[1], ray.o[2]);
    Vector3 _inv_d(ray.inv_d[0], ray.inv_d[1], ray.inv_d[2]);

    // you may already have those values hanging around somewhere
    const __m128
            plus_inf	= loadps(ps_cst_plus_inf),
            minus_inf	= loadps(ps_cst_minus_inf);

    // use whatever's apropriate to load.
    const __m128
            box_min	= loadps(&_min),
            box_max	= loadps(&_max),
            pos	= loadps(&_o),
            inv_dir	= loadps(&_inv_d);

    // use a div if inverted directions aren't available
    const __m128 l1 = mulps(subps(box_min, pos), inv_dir);
    const __m128 l2 = mulps(subps(box_max, pos), inv_dir);

    // the order we use for those min/max is vital to filter out
    // NaNs that happens when an inv_dir is +/- inf and
    // (box_min - pos) is 0. inf * 0 = NaN
    const __m128 filtered_l1a = minps(l1, plus_inf);
    const __m128 filtered_l2a = minps(l2, plus_inf);

    const __m128 filtered_l1b = maxps(l1, minus_inf);
    const __m128 filtered_l2b = maxps(l2, minus_inf);

    // now that we're back on our feet, test those slabs.
    __m128 lmax = maxps(filtered_l1a, filtered_l2a);
    __m128 lmin = minps(filtered_l1b, filtered_l2b);

    // unfold back. try to hide the latency of the shufps & co.
    const __m128 lmax0 = rotatelps(lmax);
    const __m128 lmin0 = rotatelps(lmin);
    lmax = minss(lmax, lmax0);
    lmin = maxss(lmin, lmin0);

    const __m128 lmax1 = muxhps(lmax,lmax);
    const __m128 lmin1 = muxhps(lmin,lmin);
    lmax = minss(lmax, lmax1);
    lmin = maxss(lmin, lmin1);

    const bool ret = _mm_comige_ss(lmax, _mm_setzero_ps()) & _mm_comige_ss(lmax,lmin);

    storess(lmin, tnear);
    storess(lmax, tfar);

    return  ret;
}
