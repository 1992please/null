#include "platform/window.h"
#include "core/defines.h"
#include "core/logger.h"

#include <GLFW/glfw3.h>

namespace ne {

Window::Window(int32_t iWidth, int32_t iHeight, const std::string& iName) {
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
  glfwSetKeyCallback(mWindow, keyCallback);
  glfwSetCharCallback(mWindow, charCallback);
  glfwSetMouseButtonCallback(mWindow, mouseButtonCallback);
  glfwSetCursorPosCallback(mWindow, cursorPosCallback);
  glfwSetScrollCallback(mWindow, scrollCallback);
  glfwSetCursorEnterCallback(mWindow, cursorEnterCallback);
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

bool Window::isKeyPressed(KeyCode key) const {
  int state = glfwGetKey(mWindow, static_cast<int>(key));
  return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Window::isMouseButtonPressed(MouseButton button) const {
  int state = glfwGetMouseButton(mWindow, static_cast<int>(button));
  return state == GLFW_PRESS;
}

void Window::getCursorPos(double* oXpos, double* oYpos) const {
  glfwGetCursorPos(mWindow, oXpos, oYpos);
}

void Window::setCursorMode(CursorMode mode) {
  int glfwMode = GLFW_CURSOR_NORMAL;
  switch (mode) {
    case CursorMode::Normal:   glfwMode = GLFW_CURSOR_NORMAL; break;
    case CursorMode::Hidden:   glfwMode = GLFW_CURSOR_HIDDEN; break;
    case CursorMode::Disabled: glfwMode = GLFW_CURSOR_DISABLED; break;
  }
  glfwSetInputMode(mWindow, GLFW_CURSOR, glfwMode);
}

void Window::framebufferResizeCallback(GLFWwindow* iGLFWindow, int iWidth, int iHeight) {
  Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(iGLFWindow));
  if (window) {
    window->mFrameBufferResizeEvent.broadcast(static_cast<int32_t>(iWidth), static_cast<int32_t>(iHeight));
  }
}

void Window::keyCallback(GLFWwindow* iGLFWindow, int iKey, int iScancode, int iAction, int iMods) {
  Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(iGLFWindow));
  if (window) {
    window->mKeyEvent.broadcast(static_cast<KeyCode>(iKey), static_cast<int32_t>(iScancode), static_cast<InputAction>(iAction),
                                static_cast<KeyMods>(iMods));
  }
}

void Window::charCallback(GLFWwindow* iGLFWindow, unsigned int iCodepoint) {
  Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(iGLFWindow));
  if (window) {
    window->mCharEvent.broadcast(static_cast<uint32_t>(iCodepoint));
  }
}

void Window::mouseButtonCallback(GLFWwindow* iGLFWindow, int iButton, int iAction, int iMods) {
  Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(iGLFWindow));
  if (window) {
    window->mMouseButtonEvent.broadcast(static_cast<MouseButton>(iButton), static_cast<InputAction>(iAction),
                                        static_cast<KeyMods>(iMods));
  }
}

void Window::cursorPosCallback(GLFWwindow* iGLFWindow, double iXpos, double iYpos) {
  Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(iGLFWindow));
  if (window) {
    window->mCursorPosEvent.broadcast(iXpos, iYpos);
  }
}

void Window::scrollCallback(GLFWwindow* iGLFWindow, double iXoffset, double iYoffset) {
  Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(iGLFWindow));
  if (window) {
    window->mScrollEvent.broadcast(iXoffset, iYoffset);
  }
}

void Window::cursorEnterCallback(GLFWwindow* iGLFWindow, int iEntered) {
  Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(iGLFWindow));
  if (window) {
    window->mCursorEnterEvent.broadcast(iEntered != 0);
  }
}

} // namespace ne

