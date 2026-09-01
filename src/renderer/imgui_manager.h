#pragma once

#include <volk/volk.h>
#include <string>

namespace ne {

class Window;
class Renderer;

class ImGuiManager {
public:
  ImGuiManager(Window* iWindow, Renderer* iRenderer);
  ~ImGuiManager();

  ImGuiManager(const ImGuiManager&) = delete;
  ImGuiManager& operator=(const ImGuiManager&) = delete;
  ImGuiManager(ImGuiManager&&) = delete;
  ImGuiManager& operator=(ImGuiManager&&) = delete;

  void beginFrame();
  void endFrame(VkCommandBuffer iCommandBuffer);

private:
  void setupIO();
  void setupStyle();

  Window* mWindow{nullptr};
  Renderer* mRenderer{nullptr};
  std::string mIniPath;
};

} // namespace ne
