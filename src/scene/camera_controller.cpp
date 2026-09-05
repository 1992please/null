#include "scene/camera_controller.h"
#include "components/transform_component.h"
#include "platform/input.h"
#include "core/defines.h"

namespace ne {

CameraController::CameraController(float iMoveSpeed, float iLookSensitivity)
    : mMoveSpeed(iMoveSpeed), mLookSensitivity(iLookSensitivity) {}

void CameraController::update(float iDeltaTime, TransformComponent& ioTransform) {
  // 1. Mouse look toggle and orientation synchronization
  if (Input::isMouseButtonPressed(MouseButton::Right)) {
    mIsLooking = true;
    Input::setCursorMode(CursorMode::Disabled);

    // Synchronize pitch and yaw from current transform orientation once at look start
    Vec3 euler = ioTransform.getEulerAngles();
    mPitch = euler.y;
    mYaw = euler.z;
  } else if (Input::isMouseButtonReleased(MouseButton::Right)) {
    if (mIsLooking) {
      mIsLooking = false;
      Input::setCursorMode(CursorMode::Normal);
    }
  }

  // 2. Mouse look rotation
  if (mIsLooking) {
    Vec2 mouseDelta = Input::getMouseDelta();
    mYaw += mouseDelta.x * mLookSensitivity;
    mPitch = math::clamp(mPitch + mouseDelta.y * mLookSensitivity, -89.0f, 89.0f);
    ioTransform.setEulerAngles(Vec3(0.0f, mPitch, mYaw));
  }

  // 3. Mouse wheel speed adjustment
  Vec2 scroll = Input::getMouseScroll();
  if (scroll.y != 0.0f) {
    mMoveSpeed = math::clamp(mMoveSpeed + scroll.y * 0.5f, 0.2f, 50.0f);
  }

  // 4. 1:1 Keyboard translation movement (W/S/A/D/E/Q + Shift)
  if (iDeltaTime > 0.0f) {
    Vec3 forward = ioTransform.getForward();
    Vec3 right = ioTransform.getRight();
    Vec3 moveDir = Vec3::Zero;

    if (Input::isKeyDown(KeyCode::W)) moveDir += forward;
    if (Input::isKeyDown(KeyCode::S)) moveDir -= forward;
    if (Input::isKeyDown(KeyCode::D)) moveDir += right;
    if (Input::isKeyDown(KeyCode::A)) moveDir -= right;
    if (Input::isKeyDown(KeyCode::E)) moveDir += Vec3::Up;
    if (Input::isKeyDown(KeyCode::Q)) moveDir -= Vec3::Up;

    if (moveDir.lengthSquared() > math::SMALL_NUMBER) {
      moveDir.normalize();
      float speed = mMoveSpeed * (Input::isShiftDown() ? 2.5f : 1.0f);
      ioTransform.translate(moveDir * (speed * iDeltaTime));
    }
  }
}

} // namespace ne
