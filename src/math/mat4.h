#pragma once

/**
 * @file mat4.h
 * @brief 4x4 Transformation Matrix struct.
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
 * @brief 4x4 float matrix providing 100% binary & math compatibility with Vulkan & GLM,
 * providing default identity initialization, static transformation factories, and instance helpers.
 */
struct Mat4 {
  Vec4 cols[4]{
    Vec4(1.0f, 0.0f, 0.0f, 0.0f),
    Vec4(0.0f, 1.0f, 0.0f, 0.0f),
    Vec4(0.0f, 0.0f, 1.0f, 0.0f),
    Vec4(0.0f, 0.0f, 0.0f, 1.0f)
  };

  constexpr Mat4() = default;
  constexpr explicit Mat4(float iScalar)
    : cols{
        Vec4(iScalar, 0.0f, 0.0f, 0.0f),
        Vec4(0.0f, iScalar, 0.0f, 0.0f),
        Vec4(0.0f, 0.0f, iScalar, 0.0f),
        Vec4(0.0f, 0.0f, 0.0f, iScalar)
      } {}
  constexpr Mat4(const Vec4& iC0, const Vec4& iC1, const Vec4& iC2, const Vec4& iC3)
    : cols{iC0, iC1, iC2, iC3} {}

  constexpr Vec4& operator[](size_t iIndex) { return cols[iIndex]; }
  constexpr const Vec4& operator[](size_t iIndex) const { return cols[iIndex]; }

  constexpr const float* data() const { return cols[0].data(); }
  constexpr float* data() { return cols[0].data(); }

  static const Mat4 Identity;

  // --- Matrix & Vector Multiplication Operators ---

  inline Mat4 operator*(const Mat4& iM) const {
    const auto& selfGlm = *reinterpret_cast<const glm::mat4*>(this);
    const auto& otherGlm = *reinterpret_cast<const glm::mat4*>(&iM);
    glm::mat4 res = selfGlm * otherGlm;
    return *reinterpret_cast<const Mat4*>(&res);
  }

  inline Vec4 operator*(const Vec4& iV) const {
    const auto& selfGlm = *reinterpret_cast<const glm::mat4*>(this);
    glm::vec4 res = selfGlm * glm::vec4(iV.x, iV.y, iV.z, iV.w);
    return Vec4(res.x, res.y, res.z, res.w);
  }

  // --- Static Transformation Factories ---

  static inline Mat4 translate(const Vec3& iV) {
    glm::mat4 res = glm::translate(glm::mat4(1.0f), glm::vec3(iV.x, iV.y, iV.z));
    return *reinterpret_cast<const Mat4*>(&res);
  }

  static inline Mat4 rotate(float iAngleRad, const Vec3& iAxis) {
    glm::mat4 res = glm::rotate(glm::mat4(1.0f), iAngleRad, glm::vec3(iAxis.x, iAxis.y, iAxis.z));
    return *reinterpret_cast<const Mat4*>(&res);
  }

  static inline Mat4 scale(const Vec3& iV) {
    glm::mat4 res = glm::scale(glm::mat4(1.0f), glm::vec3(iV.x, iV.y, iV.z));
    return *reinterpret_cast<const Mat4*>(&res);
  }

  static inline Mat4 fromQuat(const Quat& iQ) {
    const auto& qGlm = *reinterpret_cast<const glm::quat*>(&iQ);
    glm::mat4 res = glm::mat4_cast(qGlm);
    return *reinterpret_cast<const Mat4*>(&res);
  }

  static inline Mat4 inverse(const Mat4& iM) {
    const auto& mGlm = *reinterpret_cast<const glm::mat4*>(&iM);
    glm::mat4 res = glm::inverse(mGlm);
    return *reinterpret_cast<const Mat4*>(&res);
  }

  // --- Instance Transformation Methods ---

  inline Mat4 translated(const Vec3& iV) const {
    const auto& selfGlm = *reinterpret_cast<const glm::mat4*>(this);
    glm::mat4 res = glm::translate(selfGlm, glm::vec3(iV.x, iV.y, iV.z));
    return *reinterpret_cast<const Mat4*>(&res);
  }

  inline Mat4 rotated(float iAngleRad, const Vec3& iAxis) const {
    const auto& selfGlm = *reinterpret_cast<const glm::mat4*>(this);
    glm::mat4 res = glm::rotate(selfGlm, iAngleRad, glm::vec3(iAxis.x, iAxis.y, iAxis.z));
    return *reinterpret_cast<const Mat4*>(&res);
  }

  inline Mat4 scaled(const Vec3& iV) const {
    const auto& selfGlm = *reinterpret_cast<const glm::mat4*>(this);
    glm::mat4 res = glm::scale(selfGlm, glm::vec3(iV.x, iV.y, iV.z));
    return *reinterpret_cast<const Mat4*>(&res);
  }

  inline Mat4 inversed() const {
    return inverse(*this);
  }

  inline bool equals(const Mat4& iOther, float iTolerance = math::KINDA_SMALL_NUMBER) const {
    for (int c = 0; c < 4; ++c) {
      for (int r = 0; r < 4; ++r) {
        if (math::abs(cols[c][r] - iOther.cols[c][r]) > iTolerance) {
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
