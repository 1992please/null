#include "platform/window.h"
#include "core/core.h"

#include <GLFW/glfw3.h>

namespace ne {
Window::Window(int iWidth, int iHeight, const std::string& iName)
    : mWidth(iWidth), mHeight(iHeight), mFrameBufferResized(false), mWindowName(iName) {
  initWindow();
}

Window::~Window() {
  glfwDestroyWindow(mWindow);
  glfwTerminate();
}

bool Window::shouldClose() { return glfwWindowShouldClose(mWindow); }

void Window::processEvents() { glfwPollEvents(); }

VkExtent2D Window::getExtent() {
  int width, height;
  glfwGetFramebufferSize(mWindow, &width, &height);
  return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

void Window::frameBufferResizedCallback(GLFWwindow* window, int width, int height) {
  Window* veWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  veWindow->mFrameBufferResized = true;
  veWindow->mWidth = width;
  veWindow->mHeight = height;
}

void Window::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  // NOTE: NADER remove this when you support resizable
  // make sure the window is floating for now
#if defined(NE_PLATFORM_LINUX)
  glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
#endif

  mWindow = glfwCreateWindow(mWidth, mHeight, mWindowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(mWindow, this);
  glfwSetFramebufferSizeCallback(mWindow, frameBufferResizedCallback);
}

std::vector<const char*> Window::getRequiredInstanceExtensions() {
  uint32_t count = 0;
  const char** extensions = glfwGetRequiredInstanceExtensions(&count);
  std::vector<const char*> result(extensions, extensions + count);
  return result;
}

VkResult Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
  return glfwCreateWindowSurface(instance, mWindow, nullptr, surface);
}
} // namespace ne
