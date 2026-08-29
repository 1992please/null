#pragma once

#include "core/math/math.h"

namespace ne {

struct TransformComponent;
class Window;

class CameraController {
public:
  CameraController(float iMoveSpeed = 2.0f, float iLookSensitivity = 0.1f);

  void update(Window* iWindow, float iDeltaTime, TransformComponent& ioTransform);

private:
  float mMoveSpeed;
  float mLookSensitivity;

  double mLastMouseX{0.0};
  double mLastMouseY{0.0};
  bool mWasLooking{false};
};

} // namespace ne
