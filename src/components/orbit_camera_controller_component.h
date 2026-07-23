#pragma once

#include "core/math.h"

namespace ne {

struct OrbitCameraControllerComponent {
  Vec3 mTarget{0.0f};
  float mDistance{4.0f};
  float mMinDistance{0.1f};
  float mMaxDistance{100.0f};
  float mYaw{0.0f};   // Radians
  float mPitch{0.0f}; // Radians
  float mMinPitch{-glm::radians(89.0f)};
  float mMaxPitch{glm::radians(89.0f)};
  float mRotateSensitivity{0.005f};
  float mPanSensitivity{0.002f};
  float mZoomSensitivity{0.5f};
};

} // namespace ne
