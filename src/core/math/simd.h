#pragma once

#include <cmath>

#if defined(__SSE__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
#include <immintrin.h>
#define NE_MATH_USE_SSE
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#define NE_MATH_USE_NEON
#endif

namespace ne::math {

// 1 / sqrt(x): hardware reciprocal square root plus one Newton-Raphson step (~23 bits of precision)
inline float invSqrt(float iVal) {
#if defined(NE_MATH_USE_SSE)
  __m128 val = _mm_set_ss(iVal);
  __m128 r0 = _mm_rsqrt_ss(val);
  __m128 half = _mm_set_ss(0.5f);
  __m128 threeHalfs = _mm_set_ss(1.5f);
  __m128 r0Squared = _mm_mul_ss(r0, r0);
  __m128 halfValR0Sq = _mm_mul_ss(_mm_mul_ss(half, val), r0Squared);
  __m128 nr = _mm_sub_ss(threeHalfs, halfValR0Sq);
  __m128 res = _mm_mul_ss(r0, nr);
  return _mm_cvtss_f32(res);
#elif defined(NE_MATH_USE_NEON)
  float32x4_t val = vdupq_n_f32(iVal);
  float32x4_t r0 = vrsqrteq_f32(val);
  float32x4_t step = vrsqrtsq_f32(vmulq_f32(val, r0), r0);
  float32x4_t res = vmulq_f32(r0, step);
  return vgetq_lane_f32(res, 0);
#else
  return 1.0f / std::sqrt(iVal);
#endif
}

} // namespace ne::math
