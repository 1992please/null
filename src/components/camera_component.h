#pragma once

#include "core/math/math.h"

namespace ne {

struct CameraComponent {
  enum class ProjectionType { Perspective, Orthographic };

  ProjectionType mProjectionType{ProjectionType::Perspective};
  float mFovDeg{45.0f};             // Perspective: vertical field of view in degrees
  float mOrthoSize{5.0f};           // Orthographic: vertical view size
  float mAspectRatio{16.0f / 9.0f}; // Viewport width / height
  float mNearClip{0.1f};
  float mFarClip{1000.0f};          // Perspective: <= 0 selects an infinite far plane
  bool mIsPrimary{true};

  Mat4 getProjectionMatrix() const;

  // View space is the ROS camera optical frame (+X right, +Y down, +Z forward), which matches Vulkan NDC,
  // so the view matrix is a pure rotation + translation. World scale is ignored.
  Mat4 getViewMatrix(const Mat4& iWorldMatrix) const;

  Mat4 getViewProjectionMatrix(const Mat4& iWorldMatrix) const { return getProjectionMatrix() * getViewMatrix(iWorldMatrix); }
};

} // namespace ne
