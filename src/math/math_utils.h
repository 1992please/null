#pragma once

/**
 * @file math_utils.h
 * @brief Engine Scalar Math Utilities & Constants
 */

#include "math/simd.h"
#include <cmath>

namespace ne::math {

constexpr float PI = 3.1415926535897932f;
constexpr float INV_PI = 0.31830988618f;
constexpr float HALF_PI = 1.57079632679f;

constexpr float SMALL_NUMBER = 1.e-8f;
constexpr float KINDA_SMALL_NUMBER = 1.e-4f;
constexpr float BIG_NUMBER = 3.4e+38f;
constexpr float UE_GOLDEN_RATIO = 1.6180339887498948482045868343656381f;

constexpr float DEG_TO_RAD = PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / PI;

template <typename T>
constexpr T clamp(T iVal, T iMinVal, T iMaxVal) {
  return (iVal < iMinVal) ? iMinVal : ((iVal > iMaxVal) ? iMaxVal : iVal);
}

template <typename T, typename U = float>
constexpr T lerp(T iA, T iB, U iT) {
  return static_cast<T>(iA + (iB - iA) * iT);
}

template <typename T>
constexpr T min(T iA, T iB) {
  return (iA < iB) ? iA : iB;
}

template <typename T>
constexpr T max(T iA, T iB) {
  return (iA > iB) ? iA : iB;
}

template <typename T>
constexpr T abs(T iVal) {
  return (iVal < T(0)) ? -iVal : iVal;
}

template <typename T>
constexpr T radians(T iDeg) {
  return iDeg * static_cast<T>(DEG_TO_RAD);
}

template <typename T>
constexpr T degrees(T iRad) {
  return iRad * static_cast<T>(RAD_TO_DEG);
}

constexpr bool equals(float iA, float iB, float iTolerance = KINDA_SMALL_NUMBER) {
  return abs(iA - iB) <= iTolerance;
}

constexpr bool equals(double iA, double iB, double iTolerance = static_cast<double>(KINDA_SMALL_NUMBER)) {
  return abs(iA - iB) <= iTolerance;
}

inline float sqrt(float iVal) {
  return std::sqrt(iVal);
}

inline float sin(float iRad) {
  return std::sin(iRad);
}

inline float cos(float iRad) {
  return std::cos(iRad);
}

inline float tan(float iRad) {
  return std::tan(iRad);
}

inline float asin(float iVal) {
  return std::asin(iVal);
}

inline float acos(float iVal) {
  return std::acos(iVal);
}

inline float atan2(float iY, float iX) {
  return std::atan2(iY, iX);
}

} // namespace ne::math

