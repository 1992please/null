#pragma once

/**
 * @file simd.h
 * @brief Platform-isolated SIMD hardware intrinsics and inverse square root utility.
 */

#include <cmath>

#if defined(__SSE__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
#include <immintrin.h>
#define NE_MATH_USE_SSE
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#define NE_MATH_USE_NEON
#endif

namespace ne::math {

/**
 * @brief High-precision, hardware-accelerated inverse square root (1.0f / sqrt(x)).
 * Uses hardware reciprocal square root with one Newton-Raphson refinement step:
 * y_1 = y_0 * (1.5 - 0.5 * x * y_0^2), yielding ~23 bits of single-precision IEEE float accuracy
 * at ~3-4x the performance of divss/sqrtss.
 */
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
