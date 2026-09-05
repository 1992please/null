#pragma once

#include "core/math/math.h"
#include "platform/input_types.h"

namespace ne {

struct TransformComponent;

class CameraController {
public:
  CameraController(float iMoveSpeed = 2.0f, float iLookSensitivity = 0.1f);

  void update(float iDeltaTime, TransformComponent& ioTransform);

  bool isLooking() const { return mIsLooking; }

  float getMoveSpeed() const { return mMoveSpeed; }
  void setMoveSpeed(float iSpeed) { mMoveSpeed = iSpeed; }

  float getLookSensitivity() const { return mLookSensitivity; }
  void setLookSensitivity(float iSensitivity) { mLookSensitivity = iSensitivity; }

private:
  float mMoveSpeed;
  float mLookSensitivity;

  float mPitch{0.0f};
  float mYaw{0.0f};
  bool mIsLooking{false};
};

} // namespace ne
