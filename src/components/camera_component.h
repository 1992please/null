#pragma once

#include "core/math.h"

namespace ne {

enum class ProjectionType {
  Perspective,
  Orthographic
};

struct CameraComponent {
  ProjectionType mProjectionType{ProjectionType::Perspective};

  // Perspective settings
  float mFovDeg{45.0f};

  // Orthographic settings
  float mOrthoSize{5.0f};

  // Common settings
  float mNearPlane{0.1f};
  float mFarPlane{100.0f};
  bool mIsPrimary{true};

  Mat4 mProjectionMatrix{1.0f};
  Mat4 mViewMatrix{1.0f};

  Mat4 getViewProjectionMatrix() const {
    return mProjectionMatrix * mViewMatrix;
  }
};

} // namespace ne
