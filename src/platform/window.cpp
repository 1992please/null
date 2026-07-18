#include "platform/window.h"
#include "core/defines.h"
#include "core/logger.h"

#include <GLFW/glfw3.h>

namespace ne {

Window::Window(int iWidth, int iHeight, const std::string& iName) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  // NOTE: NADER remove this when you support resizable
  // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  // make sure the window is floating for now
  // #if defined(NE_PLATFORM_LINUX)
  //   glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
  // #endif

  mWindow = glfwCreateWindow(iWidth, iHeight, iName.c_str(), nullptr, nullptr);
  NE_LOG("Initialized GLFW and created window '{}' ({}x{})", iName, iWidth, iHeight);

  glfwSetWindowUserPointer(mWindow, this);
  glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
}

Window::~Window() {
  glfwDestroyWindow(mWindow);
  glfwTerminate();
  NE_LOG("Destroyed window and terminated GLFW");
}

void Window::processEvents() { glfwPollEvents(); }

void Window::waitEvents() { glfwWaitEvents(); }

const char* Window::getWindowName() const { return glfwGetWindowTitle(mWindow); }

bool Window::shouldClose() const { return glfwWindowShouldClose(mWindow); }

void Window::getFrameBufferSize(int32_t* oWidth, int32_t* oHeight) const { glfwGetFramebufferSize(mWindow, oWidth, oHeight); }

void Window::getWindowSize(int32_t* oWidth, int32_t* oHeight) const { glfwGetWindowSize(mWindow, oWidth, oHeight); }

std::vector<const char*> Window::getRequiredInstanceExtensions() const {
  uint32_t count = 0;
  const char** extensions = glfwGetRequiredInstanceExtensions(&count);
  std::vector<const char*> result(extensions, extensions + count);
  return result;
}

VkResult Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
  return glfwCreateWindowSurface(instance, mWindow, nullptr, surface);
}

void Window::framebufferResizeCallback(GLFWwindow* iGLFWindow, int iWidth, int iHeight) {
  Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(iGLFWindow));
  NE_UNUSED(iWidth);
  NE_UNUSED(iHeight);
  if(window->mFrameBufferResizeCallback) {
    window->mFrameBufferResizeCallback();
  }
}

} // namespace ne
