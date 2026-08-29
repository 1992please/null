#include "scene/camera_controller.h"
#include "components/transform_component.h"
#include "platform/window.h"

namespace ne {

CameraController::CameraController(float iMoveSpeed, float iLookSensitivity)
    : mMoveSpeed(iMoveSpeed), mLookSensitivity(iLookSensitivity) {}

void CameraController::update(const Window* iWindow, float iDeltaTime, TransformComponent& ioTransform) {
  if (!iWindow)
    return;

  // 1. Mouse Look (Pitch & Yaw directly into TransformComponent Euler angles)
  bool isLooking = iWindow->isMouseButtonPressed(MouseButton::Right) || iWindow->isMouseButtonPressed(MouseButton::Left);
  double currentX, currentY;
  iWindow->getCursorPos(&currentX, &currentY);

  if (isLooking) {
    if (!mFirstMouse) {
      double deltaX = currentX - mLastMouseX;
      double deltaY = currentY - mLastMouseY;

      Vec3 euler = ioTransform.getEulerAngles();
      euler.y = math::clamp(euler.y + static_cast<float>(deltaY) * mLookSensitivity, -89.0f, 89.0f); // Pitch
      euler.z += static_cast<float>(deltaX) * mLookSensitivity;                                      // Yaw
      euler.x = 0.0f;                                                                                // Keep roll zero

      ioTransform.setEulerAngles(euler);
    }
    mFirstMouse = false;
  } else {
    mFirstMouse = true;
  }

  mLastMouseX = currentX;
  mLastMouseY = currentY;

  // 2. Keyboard Fly Movement (WASD + QE + Shift)
  if (iDeltaTime > 0.0f) {
    Vec3 forward = ioTransform.getForward();
    Vec3 right = ioTransform.getRight();
    Vec3 moveDir = Vec3::Zero;

    if (iWindow->isKeyPressed(KeyCode::W))
      moveDir += forward;
    if (iWindow->isKeyPressed(KeyCode::S))
      moveDir -= forward;
    if (iWindow->isKeyPressed(KeyCode::D))
      moveDir += right;
    if (iWindow->isKeyPressed(KeyCode::A))
      moveDir -= right;
    if (iWindow->isKeyPressed(KeyCode::E) || iWindow->isKeyPressed(KeyCode::Space))
      moveDir += Vec3::Up;
    if (iWindow->isKeyPressed(KeyCode::Q) || iWindow->isKeyPressed(KeyCode::C))
      moveDir -= Vec3::Up;

    if (moveDir.lengthSquared() > math::SMALL_NUMBER) {
      moveDir.normalize();
      float speed = mMoveSpeed;
      if (iWindow->isKeyPressed(KeyCode::LeftShift) || iWindow->isKeyPressed(KeyCode::RightShift)) {
        speed *= 2.5f;
      }
      ioTransform.translate(moveDir * (speed * iDeltaTime));
    }
  }
}

} // namespace ne
