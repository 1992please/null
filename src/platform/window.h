#pragma once

#include <volk/volk.h>
#include "core/event.h"
#include "platform/input_types.h"

// std
#include <functional>
#include <string>
#include <vector>

struct GLFWwindow;

namespace ne {

class Window {
public:
  using FrameBufferResizeCallback = Event<int32_t, int32_t>::CallbackType;
  using KeyCallback = Event<KeyCode, int32_t, InputAction, KeyMods>::CallbackType;
  using CharCallback = Event<uint32_t>::CallbackType;
  using MouseButtonCallback = Event<MouseButton, InputAction, KeyMods>::CallbackType;
  using CursorPosCallback = Event<double, double>::CallbackType;
  using ScrollCallback = Event<double, double>::CallbackType;
  using CursorEnterCallback = Event<bool>::CallbackType;

  Window(int32_t iWidth, int32_t iHeight, const std::string& iName);
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

  // Input Polling & State
  bool isKeyPressed(KeyCode key) const;
  bool isMouseButtonPressed(MouseButton button) const;
  void getCursorPos(double* oXpos, double* oYpos) const;
  void setCursorMode(CursorMode mode);

  // Encapsulated Callback Subscriptions
  CallbackId addFrameBufferResizeCallback(FrameBufferResizeCallback iCallback) { return mFrameBufferResizeEvent.add(std::move(iCallback)); }
  CallbackId addKeyCallback(KeyCallback iCallback) { return mKeyEvent.add(std::move(iCallback)); }
  CallbackId addCharCallback(CharCallback iCallback) { return mCharEvent.add(std::move(iCallback)); }
  CallbackId addMouseButtonCallback(MouseButtonCallback iCallback) { return mMouseButtonEvent.add(std::move(iCallback)); }
  CallbackId addCursorPosCallback(CursorPosCallback iCallback) { return mCursorPosEvent.add(std::move(iCallback)); }
  CallbackId addScrollCallback(ScrollCallback iCallback) { return mScrollEvent.add(std::move(iCallback)); }
  CallbackId addCursorEnterCallback(CursorEnterCallback iCallback) { return mCursorEnterEvent.add(std::move(iCallback)); }

  void removeFrameBufferResizeCallback(CallbackId iId) { mFrameBufferResizeEvent.remove(iId); }
  void removeKeyCallback(CallbackId iId) { mKeyEvent.remove(iId); }
  void removeCharCallback(CallbackId iId) { mCharEvent.remove(iId); }
  void removeMouseButtonCallback(CallbackId iId) { mMouseButtonEvent.remove(iId); }
  void removeCursorPosCallback(CallbackId iId) { mCursorPosEvent.remove(iId); }
  void removeScrollCallback(CallbackId iId) { mScrollEvent.remove(iId); }
  void removeCursorEnterCallback(CallbackId iId) { mCursorEnterEvent.remove(iId); }

private:
  static void framebufferResizeCallback(GLFWwindow* iWindow, int iWidth, int iHeight);
  static void keyCallback(GLFWwindow* iWindow, int iKey, int iScancode, int iAction, int iMods);
  static void charCallback(GLFWwindow* iWindow, unsigned int iCodepoint);
  static void mouseButtonCallback(GLFWwindow* iWindow, int iButton, int iAction, int iMods);
  static void cursorPosCallback(GLFWwindow* iWindow, double iXpos, double iYpos);
  static void scrollCallback(GLFWwindow* iWindow, double iXoffset, double iYoffset);
  static void cursorEnterCallback(GLFWwindow* iWindow, int iEntered);

  // screen coordinates width and height
  GLFWwindow* mWindow;

  Event<int32_t, int32_t> mFrameBufferResizeEvent;
  Event<KeyCode, int32_t, InputAction, KeyMods> mKeyEvent;
  Event<uint32_t> mCharEvent;
  Event<MouseButton, InputAction, KeyMods> mMouseButtonEvent;
  Event<double, double> mCursorPosEvent;
  Event<double, double> mScrollEvent;
  Event<bool> mCursorEnterEvent;
};
} // namespace ne

