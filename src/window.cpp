#include "window.h"

#include "core/assert.h"

namespace ne {
Window::Window(int iWidth, int iHeight, const std::string &iName)
    : mWidth(iWidth), mHeight(iHeight), mFrameBufferResized(false), mWindowName(iName) {
  initWindow();
}

Window::~Window() {
  glfwDestroyWindow(mWindow);
  glfwTerminate();
}

void Window::frameBufferResizedCallback(GLFWwindow *window, int width,
                                        int height) {
  Window *veWindow =
      reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
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
  glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

  mWindow =
      glfwCreateWindow(mWidth, mHeight, mWindowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(mWindow, this);
  glfwSetFramebufferSizeCallback(mWindow, frameBufferResizedCallback);

}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
  bool success =
      glfwCreateWindowSurface(instance, mWindow, nullptr, surface) == VK_SUCCESS;
  NE_ASSERT(success, "failed to create window surface");
}
} // namespace ne
