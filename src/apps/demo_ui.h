#pragma once

#include <imgui.h>

namespace ne {

class DemoUI {
public:
  void render(float iFPS, float iFrameTime) {
    ImGui::Begin("Null Engine - ImGui Demo");
    ImGui::Text("Application average: %.2f ms/frame (%.1f FPS)", iFrameTime * 1000, iFPS);
    ImGui::Checkbox("Show ImGui Demo Window", &mShowDemoWindow);
    if (mShowDemoWindow) {
      ImGui::ShowDemoWindow(&mShowDemoWindow);
    }
    ImGui::End();
  }

private:
  bool mShowDemoWindow{false};
};

} // namespace ne
