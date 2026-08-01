#pragma once

/**
 * @file quat.h
 * @brief Quaternion rotation struct inheriting from glm::quat.
 */

#include "math/vec3.h"
#include <glm/gtc/quaternion.hpp>

namespace ne {

/**
 * @struct Quat
 * @brief Quaternion rotation inheriting from glm::quat for 100% binary & math compatibility,
 * providing default identity initialization (w=1, x=0, y=0, z=0), static factories, and instance helpers.
 */
struct Quat : public glm::quat {
  using glm::quat::quat;

  constexpr Quat() : glm::quat(1.0f, 0.0f, 0.0f, 0.0f) {}
  constexpr Quat(float iW, float iX, float iY, float iZ) : glm::quat(iW, iX, iY, iZ) {}
  constexpr Quat(const glm::quat& iQ) : glm::quat(iQ) {}

  static const Quat Identity;

  // --- Vector Transformation Operator ---

  inline Vec3 operator*(const Vec3& iV) const {
    glm::vec3 rotatedGlm = static_cast<const glm::quat&>(*this) * glm::vec3(iV.x, iV.y, iV.z);
    return Vec3(rotatedGlm.x, rotatedGlm.y, rotatedGlm.z);
  }

  // --- Static Factories ---

  static inline Quat angleAxis(float iAngleRad, const Vec3& iAxis) {
    return glm::angleAxis(iAngleRad, glm::vec3(iAxis.x, iAxis.y, iAxis.z));
  }

  static inline Quat fromEuler(const Vec3& iEulerDegrees) {
    return glm::quat(glm::vec3(math::radians(iEulerDegrees.x), math::radians(iEulerDegrees.y), math::radians(iEulerDegrees.z)));
  }

  static inline Quat slerp(const Quat& iA, const Quat& iB, float iT) {
    return glm::slerp(static_cast<const glm::quat&>(iA), static_cast<const glm::quat&>(iB), iT);
  }

  // --- Instance Methods ---

  inline Quat conjugate() const {
    return glm::conjugate(static_cast<const glm::quat&>(*this));
  }

  inline Vec3 toEuler() const {
    glm::vec3 rad = glm::eulerAngles(static_cast<const glm::quat&>(*this));
    return Vec3(math::degrees(rad.x), math::degrees(rad.y), math::degrees(rad.z));
  }

  inline bool normalize(float iTolerance = math::SMALL_NUMBER) {
    float lenSq = w * w + x * x + y * y + z * z;
    if (lenSq > iTolerance) {
      float invLen = 1.0f / math::sqrt(lenSq);
      w *= invLen;
      x *= invLen;
      y *= invLen;
      z *= invLen;
      return true;
    }
    w = 1.0f;
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    return false;
  }

  inline std::string toString() const {
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Quat(w=%.3f, x=%.3f, y=%.3f, z=%.3f)", w, x, y, z);
    return std::string(buf);
  }
};

inline const Quat Quat::Identity{1.0f, 0.0f, 0.0f, 0.0f};

} // namespace ne
