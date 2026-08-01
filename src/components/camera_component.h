#pragma once

#include "math/math.h"

namespace ne {

/**
 * @struct CameraComponent
 * @brief Manages camera lens, frustum parameters, and projection matrix generation.
 *
 * Designed for left-handed Unreal Engine coordinate conventions (+X Forward, +Y Right, +Z Up).
 * Supports Standard-Z, Reverse-Z, and Infinite Far Clip perspective and orthographic projections.
 */
struct CameraComponent {
  enum class ProjectionType { Perspective, Orthographic };

  ProjectionType mProjectionType{ProjectionType::Perspective};

  // Lens Parameters
  float mFovDeg{45.0f};             // Vertical Field of View in degrees
  float mAspectRatio{16.0f / 9.0f};  // Viewport Width / Height
  float mNearClip{0.1f};
  float mFarClip{1000.0f};
  float mOrthoSize{5.0f};           // Vertical size for orthographic camera

  // Depth & Active Camera Flags
  bool mUseReverseZ{true};          // Reverse-Z float depth (1.0 near, 0.0 far)
  bool mInfiniteFarClip{false};     // Infinite far plane projection
  bool mIsPrimary{true};            // Main rendering camera

  // Cached Projection Matrices
  Mat4 mProjectionMatrix{1.0f};
  Mat4 mInverseProjectionMatrix{1.0f};

  // Mutators & Recalculation
  void updateProjection();
  void setPerspective(float iFovDeg, float iAspect, float iNear, float iFar = 0.0f);
  void setOrthographic(float iSize, float iAspect, float iNear, float iFar);
};

} // namespace ne
