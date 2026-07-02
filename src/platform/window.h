#pragma once

#include <volk/volk.h>

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

  bool shouldClose() const;
  void processEvents();
  void waitEvents();
  void getFrameBufferSize(int32_t* oWidth, int32_t* oHeight) const;
  void getWindowSize(int32_t* oWidth, int32_t* oHeight) const;
  GLFWwindow* getGLFWwindow() const { return mWindow; }
  const char* getWindowName() const;
  std::vector<const char*> getRequiredInstanceExtensions() const;
  VkResult createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
  // screen coordinates width and height
  GLFWwindow* mWindow;
};
} // namespace ne
