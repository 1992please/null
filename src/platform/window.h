#pragma once

#include <GLFW/glfw3.h>
#include <string>

namespace ne {
class Window {
public:
  Window(int iWidth, int iHeight, const std::string &iName);
  ~Window();

  // Remove copy constructor
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  bool shouldClose() { return glfwWindowShouldClose(mWindow); }
  VkExtent2D getExtent() {
    return {static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight)};
  }
  bool wasWindowResized() { return mFrameBufferResized; }
  void resetWindowResizedFlag() { mFrameBufferResized = false; }
  GLFWwindow *getGLFWwindow() const { return mWindow; }

  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

private:
  static void frameBufferResizedCallback(GLFWwindow *window, int width,
                                         int height);
  void initWindow();

  int mWidth;
  int mHeight;
  bool mFrameBufferResized;

  std::string mWindowName;
  GLFWwindow *mWindow;
};
} // namespace ne
