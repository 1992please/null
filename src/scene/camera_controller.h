#pragma once

#include "core/math/math.h"

namespace ne {

struct TransformComponent;
class Window;

class CameraController {
public:
  CameraController(float iMoveSpeed = 4.0f, float iLookSensitivity = 0.15f);

  void update(const Window* iWindow, float iDeltaTime, TransformComponent& ioTransform);

private:
  float mMoveSpeed{4.0f};
  float mLookSensitivity{0.15f};

  double mLastMouseX{0.0};
  double mLastMouseY{0.0};
  bool mFirstMouse{true};
};

} // namespace ne
