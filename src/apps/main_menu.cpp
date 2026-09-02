#include "apps/main_menu.h"
#include "apps/basic_app.h"
#include "components/camera_component.h"
#include "components/transform_component.h"
#include "core/ecs.h"
#include "core/time.h"
#include "platform/window.h"

#include <imgui.h>

namespace ne {

void MainMenu::draw(BasicApp& iApp) {
  drawMainMenuBar(iApp);

  if (mShowDiagnostics) {
    drawDiagnostics(iApp);
  }

  if (mShowCameraSettings) {
    drawCameraSettings(iApp);
  }

  if (mShowAboutModal) {
    drawAboutModal();
  }

  if (mShowDemoWindow) {
    ImGui::ShowDemoWindow(&mShowDemoWindow);
  }
}

void MainMenu::drawMainMenuBar(BasicApp& iApp) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Exit", "Alt+F4")) {
        if (auto* win = iApp.getWindow()) {
          win->setShouldClose(true);
        }
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
      ImGui::MenuItem("Diagnostics / Stats", nullptr, &mShowDiagnostics);
      ImGui::MenuItem("Camera Controls", nullptr, &mShowCameraSettings);
      ImGui::Separator();
      ImGui::MenuItem("Toggle UI Overlay", "F1", nullptr, false);
      ImGui::MenuItem("ImGui Demo Window", nullptr, &mShowDemoWindow);
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Camera")) {
      if (ImGui::MenuItem("Reset Position & Orientation")) {
        resetCamera(iApp);
      }
      ImGui::Separator();
      float currentSpeed = iApp.getCameraController().getMoveSpeed();
      if (ImGui::MenuItem("Speed: Slow (1.0 m/s)", nullptr, currentSpeed == 1.0f)) {
        iApp.getCameraController().setMoveSpeed(1.0f);
      }
      if (ImGui::MenuItem("Speed: Normal (2.0 m/s)", nullptr, currentSpeed == 2.0f)) {
        iApp.getCameraController().setMoveSpeed(2.0f);
      }
      if (ImGui::MenuItem("Speed: Fast (5.0 m/s)", nullptr, currentSpeed == 5.0f)) {
        iApp.getCameraController().setMoveSpeed(5.0f);
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
      if (ImGui::MenuItem("About Null Engine...")) {
        mShowAboutModal = true;
      }
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}

void MainMenu::drawDiagnostics(BasicApp& iApp) {
  ImGui::SetNextWindowSize(ImVec2(340, 240), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Diagnostics", &mShowDiagnostics)) {
    float currentFrameTime = Time::getUnscaledDeltaTime();
    float currentFPS = currentFrameTime > 0.0f ? 1.0f / currentFrameTime : 0.0f;
    ImGui::Text("Performance");
    ImGui::Separator();
    ImGui::Text("Frame Rate: %.1f FPS", currentFPS);
    ImGui::Text("Frame Time: %.2f ms", currentFrameTime * 1000);
    ImGui::Spacing();
    ImGui::Text("Scene Statistics");
    ImGui::Separator();
    if (auto* reg = iApp.getRegistry()) {
      ImGui::Text("Active Entities: %zu", reg->size());
    }
    ImGui::Spacing();
    ImGui::Text("Simulation Time Controls");
    ImGui::Separator();
    float timeScale = Time::getTimeScale();
    if (ImGui::SliderFloat("Time Scale", &timeScale, 0.0f, 3.0f, "%.2fx")) {
      Time::setTimeScale(timeScale);
    }
    if (ImGui::Button("Pause (0x)")) {
      Time::setTimeScale(0.0f);
    }
    ImGui::SameLine();
    if (ImGui::Button("0.5x")) {
      Time::setTimeScale(0.5f);
    }
    ImGui::SameLine();
    if (ImGui::Button("1.0x")) {
      Time::setTimeScale(1.0f);
    }
    ImGui::SameLine();
    if (ImGui::Button("2.0x")) {
      Time::setTimeScale(2.0f);
    }
  }
  ImGui::End();
}

void MainMenu::drawCameraSettings(BasicApp& iApp) {
  ImGui::SetNextWindowSize(ImVec2(360, 340), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Camera Settings", &mShowCameraSettings)) {
    auto* reg = iApp.getRegistry();
    Entity camEntity = iApp.getCameraEntity();

    if (reg && reg->isValid(camEntity) && reg->hasComponent<TransformComponent>(camEntity) &&
        reg->hasComponent<CameraComponent>(camEntity)) {
      auto& transform = reg->getComponent<TransformComponent>(camEntity);
      auto& camera = reg->getComponent<CameraComponent>(camEntity);

      ImGui::Text("Transform (+X Fwd, +Y Right, +Z Up)");
      ImGui::Separator();
      Vec3 pos = transform.getPosition();
      if (ImGui::DragFloat3("Position (m)", &pos.x, 0.05f)) {
        transform.setPosition(pos);
      }

      Vec3 euler = transform.getEulerAngles();
      if (ImGui::DragFloat3("Rotation (deg)", &euler.x, 0.5f)) {
        transform.setEulerAngles(euler);
      }

      ImGui::Spacing();
      ImGui::Text("Projection (Reverse-Z Depth)");
      ImGui::Separator();
      float fov = camera.mFovDeg;
      if (ImGui::SliderFloat("FOV (deg)", &fov, 10.0f, 120.0f, "%.1f")) {
        camera.setPerspective(fov, camera.mAspectRatio, camera.mNearClip, camera.mFarClip);
      }

      float nearClip = camera.mNearClip;
      if (ImGui::DragFloat("Near Clip (m)", &nearClip, 0.01f, 0.01f, 10.0f, "%.2f")) {
        camera.setPerspective(camera.mFovDeg, camera.mAspectRatio, nearClip, camera.mFarClip);
      }

      float farClip = camera.mFarClip;
      if (ImGui::DragFloat("Far Clip (m)", &farClip, 1.0f, 1.0f, 1000.0f, "%.1f")) {
        camera.setPerspective(camera.mFovDeg, camera.mAspectRatio, camera.mNearClip, farClip);
      }

      ImGui::Spacing();
      ImGui::Text("Controller Parameters");
      ImGui::Separator();
      float speed = iApp.getCameraController().getMoveSpeed();
      if (ImGui::SliderFloat("Fly Speed (m/s)", &speed, 0.1f, 20.0f, "%.1f")) {
        iApp.getCameraController().setMoveSpeed(speed);
      }

      float sensitivity = iApp.getCameraController().getLookSensitivity();
      if (ImGui::SliderFloat("Look Sensitivity", &sensitivity, 0.01f, 1.0f, "%.2f")) {
        iApp.getCameraController().setLookSensitivity(sensitivity);
      }

      ImGui::Spacing();
      if (ImGui::Button("Reset Camera Transform")) {
        resetCamera(iApp);
      }
    }
  }
  ImGui::End();
}

void MainMenu::resetCamera(BasicApp& iApp) {
  if (auto* reg = iApp.getRegistry()) {
    Entity camEntity = iApp.getCameraEntity();
    if (reg->isValid(camEntity) && reg->hasComponent<TransformComponent>(camEntity)) {
      auto& transform = reg->getComponent<TransformComponent>(camEntity);
      transform.setPosition(Vec3(-4.0f, 0.0f, 0.0f));
      transform.setRotation(Quat::Identity);
    }
  }
}

void MainMenu::drawAboutModal() {
  if (mShowAboutModal) {
    ImGui::OpenPopup("About Null Engine");
  }

  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(460, 330), ImGuiCond_Appearing);

  if (ImGui::BeginPopupModal("About Null Engine", &mShowAboutModal, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Null Engine - 3D Model Viewer");
    ImGui::TextDisabled("Version 1.0.0 (C++20 & Vulkan 1.4)");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Core Technical Architecture:");
    ImGui::BulletText("Vulkan 1.4 Dynamic Rendering & Synchronization2");
    ImGui::BulletText("Programmable Vertex Pulling via BDA & MDI");
    ImGui::BulletText("Slang Shading Language -> SPIR-V 1.4");
    ImGui::BulletText("Unreal Coordinate System (+X Fwd, +Y Right, +Z Up)");
    ImGui::BulletText("Floating-Point Reverse-Z Depth (0.0 Far Clear)");
    ImGui::Spacing();

    ImGui::Text("Navigation & Controls:");
    ImGui::BulletText("Fly Camera: WASD + QE (Up/Down)");
    ImGui::BulletText("Mouse Look: Hold Right-Click + Move Mouse");
    ImGui::BulletText("Toggle UI: F1 to hide/show menus and panels");
    ImGui::Spacing();

    ImGui::Separator();
    if (ImGui::Button("Close", ImVec2(120, 0))) {
      mShowAboutModal = false;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

} // namespace ne
