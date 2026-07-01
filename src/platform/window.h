#pragma once

#include <Volk/volk.h>

// std
#include <string>
#include <vector>

struct GLFWwindow;

namespace ne {

class Window {
public:
  Window(int iWidth, int iHeight, const std::string& iName);
  ~Window();

  // Remove copy constructor
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  const std::string& getWindowName() const { return mWindowName; }
  bool shouldClose();
  void processEvents();
  VkExtent2D getExtent();
  bool wasWindowResized() { return mFrameBufferResized; }
  void resetWindowResizedFlag() { mFrameBufferResized = false; }
  GLFWwindow* getGLFWwindow() const { return mWindow; }
  std::vector<const char*> getRequiredInstanceExtensions();
  VkResult createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
  static void frameBufferResizedCallback(GLFWwindow* window, int width, int height);
  void initWindow();

  int mWidth;
  int mHeight;
  bool mFrameBufferResized;

  std::string mWindowName;
  GLFWwindow* mWindow;
};
} // namespace ne
