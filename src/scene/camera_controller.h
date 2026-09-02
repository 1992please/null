#pragma once

#include "core/math/math.h"
#include "platform/input_types.h"

namespace ne {

struct TransformComponent;
class Window;

class CameraController {
public:
  CameraController(float iMoveSpeed = 2.0f, float iLookSensitivity = 0.1f);

  bool onKey(KeyCode iKey, InputAction iAction, KeyMods iMods);
  bool onMouseButton(Window* iWindow, MouseButton iButton, InputAction iAction, KeyMods iMods);
  void onCursorPos(double iXpos, double iYpos);
  void reset();

  void update(float iDeltaTime, TransformComponent& ioTransform);

  bool isLooking() const { return mIsLooking; }

  float getMoveSpeed() const { return mMoveSpeed; }
  void setMoveSpeed(float iSpeed) { mMoveSpeed = iSpeed; }

  float getLookSensitivity() const { return mLookSensitivity; }
  void setLookSensitivity(float iSensitivity) { mLookSensitivity = iSensitivity; }

private:
  float mMoveSpeed;
  float mLookSensitivity;

  double mLastMouseX{0.0};
  double mLastMouseY{0.0};
  double mMouseDeltaX{0.0};
  double mMouseDeltaY{0.0};
  bool mIsLooking{false};

  // Active movement states
  bool mMoveForward{false};
  bool mMoveBackward{false};
  bool mMoveRight{false};
  bool mMoveLeft{false};
  bool mMoveUp{false};
  bool mMoveDown{false};
  bool mSpeedBoost{false};
};

} // namespace ne
