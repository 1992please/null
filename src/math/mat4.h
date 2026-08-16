#pragma once

/**
 * @file mat4.h
 * @brief 4x4 Transformation Matrix struct inheriting from glm::mat4.
 */

#include "math/vec3.h"
#include "math/vec4.h"
#include "math/quat.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

namespace ne {

/**
 * @struct Mat4
 * @brief 4x4 float matrix inheriting from glm::mat4 for 100% binary & math compatibility,
 * providing default identity initialization, static transformation factories, and instance helpers.
 */
struct Mat4 : public glm::mat4 {
  using glm::mat4::mat;

  constexpr Mat4() : glm::mat4(1.0f) {}
  constexpr Mat4(float iScalar) : glm::mat4(iScalar) {}
  constexpr Mat4(const glm::mat4& iM) : glm::mat4(iM) {}

  static const Mat4 Identity;

  // --- Matrix & Vector Multiplication Operators ---

  inline Mat4 operator*(const Mat4& iM) const {
    return glm::operator*(static_cast<const glm::mat4&>(*this), static_cast<const glm::mat4&>(iM));
  }

  inline Vec4 operator*(const Vec4& iV) const {
    glm::vec4 res = static_cast<const glm::mat4&>(*this) * glm::vec4(iV.x, iV.y, iV.z, iV.w);
    return Vec4(res.x, res.y, res.z, res.w);
  }

  // --- Static Transformation Factories ---

  static inline Mat4 translate(const Vec3& iV) {
    return glm::translate(glm::mat4(1.0f), glm::vec3(iV.x, iV.y, iV.z));
  }

  static inline Mat4 rotate(float iAngleRad, const Vec3& iAxis) {
    return glm::rotate(glm::mat4(1.0f), iAngleRad, glm::vec3(iAxis.x, iAxis.y, iAxis.z));
  }

  static inline Mat4 scale(const Vec3& iV) {
    return glm::scale(glm::mat4(1.0f), glm::vec3(iV.x, iV.y, iV.z));
  }

  static inline Mat4 fromQuat(const Quat& iQ) {
    return glm::mat4_cast(static_cast<const glm::quat&>(iQ));
  }

  static inline Mat4 inverse(const Mat4& iM) {
    return glm::inverse(static_cast<const glm::mat4&>(iM));
  }

  // --- Instance Transformation Methods ---

  inline Mat4 translated(const Vec3& iV) const {
    return glm::translate(static_cast<const glm::mat4&>(*this), glm::vec3(iV.x, iV.y, iV.z));
  }

  inline Mat4 rotated(float iAngleRad, const Vec3& iAxis) const {
    return glm::rotate(static_cast<const glm::mat4&>(*this), iAngleRad, glm::vec3(iAxis.x, iAxis.y, iAxis.z));
  }

  inline Mat4 scaled(const Vec3& iV) const {
    return glm::scale(static_cast<const glm::mat4&>(*this), glm::vec3(iV.x, iV.y, iV.z));
  }

  inline Mat4 inversed() const {
    return glm::inverse(static_cast<const glm::mat4&>(*this));
  }

  inline bool equals(const Mat4& iOther, float iTolerance = math::KINDA_SMALL_NUMBER) const {
    for (int c = 0; c < 4; ++c) {
      for (int r = 0; r < 4; ++r) {
        if (math::abs((*this)[c][r] - iOther[c][r]) > iTolerance) {
          return false;
        }
      }
    }
    return true;
  }

  inline std::string toString() const {
    char buf[256];
    const auto& m = *this;
    std::snprintf(buf, sizeof(buf),
      "Mat4(\n  [%.3f, %.3f, %.3f, %.3f]\n  [%.3f, %.3f, %.3f, %.3f]\n  [%.3f, %.3f, %.3f, %.3f]\n  [%.3f, %.3f, %.3f, %.3f]\n)",
      m[0][0], m[1][0], m[2][0], m[3][0],
      m[0][1], m[1][1], m[2][1], m[3][1],
      m[0][2], m[1][2], m[2][2], m[3][2],
      m[0][3], m[1][3], m[2][3], m[3][3]);
    return std::string(buf);
  }
};

inline const Mat4 Mat4::Identity{1.0f};

} // namespace ne
