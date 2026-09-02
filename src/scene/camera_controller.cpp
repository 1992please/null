#include "scene/camera_controller.h"
#include "components/transform_component.h"
#include "platform/window.h"
#include "core/defines.h"

namespace ne {

CameraController::CameraController(float iMoveSpeed, float iLookSensitivity)
    : mMoveSpeed(iMoveSpeed), mLookSensitivity(iLookSensitivity) {}

bool CameraController::onKey(KeyCode iKey, InputAction iAction, KeyMods iMods) {
  bool isPressed = (iAction == InputAction::Press || iAction == InputAction::Repeat);
  bool isRelease = (iAction == InputAction::Release);

  if (!isPressed && !isRelease) {
    return false;
  }

  // Update speed boost dynamically via KeyMods or Shift key
  mSpeedBoost = (iMods & KeyMods::Shift);

  switch (iKey) {
    case KeyCode::W: mMoveForward = isPressed; return true;
    case KeyCode::S: mMoveBackward = isPressed; return true;
    case KeyCode::D: mMoveRight = isPressed; return true;
    case KeyCode::A: mMoveLeft = isPressed; return true;
    case KeyCode::E:
    case KeyCode::Space: mMoveUp = isPressed; return true;
    case KeyCode::Q:
    case KeyCode::C: mMoveDown = isPressed; return true;
    case KeyCode::LeftShift:
    case KeyCode::RightShift: mSpeedBoost = isPressed; return true;
    default: return false;
  }
}

bool CameraController::onMouseButton(Window* iWindow, MouseButton iButton, InputAction iAction, KeyMods iMods) {
  NE_UNUSED(iMods);
  if (iButton == MouseButton::Right) {
    if (iAction == InputAction::Press) {
      mIsLooking = true;
      mMouseDeltaX = 0.0;
      mMouseDeltaY = 0.0;
      if (iWindow) {
        iWindow->getCursorPos(&mLastMouseX, &mLastMouseY);
        iWindow->setCursorMode(CursorMode::Disabled);
      }
      return true;
    } else if (iAction == InputAction::Release) {
      mIsLooking = false;
      mMouseDeltaX = 0.0;
      mMouseDeltaY = 0.0;
      if (iWindow) {
        iWindow->setCursorMode(CursorMode::Normal);
      }
      return true;
    }
  }
  return false;
}

void CameraController::onCursorPos(double iXpos, double iYpos) {
  if (mIsLooking) {
    mMouseDeltaX += (iXpos - mLastMouseX);
    mMouseDeltaY += (iYpos - mLastMouseY);
  }

  mLastMouseX = iXpos;
  mLastMouseY = iYpos;
}

void CameraController::reset() {
  mMoveForward = false;
  mMoveBackward = false;
  mMoveRight = false;
  mMoveLeft = false;
  mMoveUp = false;
  mMoveDown = false;
  mSpeedBoost = false;
  mIsLooking = false;
  mMouseDeltaX = 0.0;
  mMouseDeltaY = 0.0;
}

void CameraController::update(float iDeltaTime, TransformComponent& ioTransform) {
  // 1. Apply accumulated mouse look rotation
  if (mIsLooking) {
    Vec3 euler = ioTransform.getEulerAngles();
    euler.y = math::clamp(euler.y + static_cast<float>(mMouseDeltaY) * mLookSensitivity, -89.0f, 89.0f);
    euler.z += static_cast<float>(mMouseDeltaX) * mLookSensitivity;
    euler.x = 0.0f;

    ioTransform.setEulerAngles(euler);

    mMouseDeltaX = 0.0;
    mMouseDeltaY = 0.0;
  }

  // 2. Apply keyboard translation movement
  if (iDeltaTime > 0.0f) {
    Vec3 forward = ioTransform.getForward();
    Vec3 right = ioTransform.getRight();
    Vec3 moveDir = Vec3::Zero;

    if (mMoveForward) moveDir += forward;
    if (mMoveBackward) moveDir -= forward;
    if (mMoveRight) moveDir += right;
    if (mMoveLeft) moveDir -= right;
    if (mMoveUp) moveDir += Vec3::Up;
    if (mMoveDown) moveDir -= Vec3::Up;

    if (moveDir.lengthSquared() > math::SMALL_NUMBER) {
      moveDir.normalize();
      float speed = mMoveSpeed * (mSpeedBoost ? 2.5f : 1.0f);
      ioTransform.translate(moveDir * (speed * iDeltaTime));
    }
  }
}

} // namespace ne
