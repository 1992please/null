#pragma once

#include <volk/volk.h>
#include "core/event.h"
#include "platform/input_types.h"

// std
#include <functional>
#include <string>
#include <vector>

struct GLFWwindow;
struct GLFWmonitor;

namespace ne {

class Window {
public:
  using FrameBufferResizeEvent = Event<int32_t, int32_t>;
  using KeyEvent = Event<KeyCode, int32_t, InputAction, KeyMods>;
  using CharEvent = Event<uint32_t>;
  using MouseButtonEvent = Event<MouseButton, InputAction, KeyMods>;
  using CursorPosEvent = Event<double, double>;
  using ScrollEvent = Event<double, double>;
  using CursorEnterEvent = Event<bool>;

  Window(int32_t iWidth, int32_t iHeight, const std::string& iName);
  ~Window();

  // Remove copy constructor
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  bool shouldClose() const;
  void setShouldClose(bool iClose = true);
  void processEvents();
  void waitEvents();
  void getFrameBufferSize(int32_t* oWidth, int32_t* oHeight) const;
  void getWindowSize(int32_t* oWidth, int32_t* oHeight) const;
  GLFWwindow* getGLFWwindow() const { return mWindow; }
  GLFWmonitor* getPrimaryMonitor() const;
  const char* getWindowName() const;
  void setTitle(const std::string& iTitle);
  std::vector<const char*> getRequiredInstanceExtensions() const;
  VkResult createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

  // Input Polling & State
  bool isKeyPressed(KeyCode key) const;
  bool isMouseButtonPressed(MouseButton button) const;
  void getCursorPos(double* oXpos, double* oYpos) const;
  void setCursorMode(CursorMode mode);

  // Encapsulated Callback Subscriptions
  CallbackId addFrameBufferResizeCallback(FrameBufferResizeEvent::Callback iCallback) { return mFrameBufferResizeEvent.add(std::move(iCallback)); }
  CallbackId addKeyCallback(KeyEvent::Callback iCallback) { return mKeyEvent.add(std::move(iCallback)); }
  CallbackId addCharCallback(CharEvent::Callback iCallback) { return mCharEvent.add(std::move(iCallback)); }
  CallbackId addMouseButtonCallback(MouseButtonEvent::Callback iCallback) { return mMouseButtonEvent.add(std::move(iCallback)); }
  CallbackId addCursorPosCallback(CursorPosEvent::Callback iCallback) { return mCursorPosEvent.add(std::move(iCallback)); }
  CallbackId addScrollCallback(ScrollEvent::Callback iCallback) { return mScrollEvent.add(std::move(iCallback)); }
  CallbackId addCursorEnterCallback(CursorEnterEvent::Callback iCallback) { return mCursorEnterEvent.add(std::move(iCallback)); }

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

  FrameBufferResizeEvent mFrameBufferResizeEvent;
  KeyEvent mKeyEvent;
  CharEvent mCharEvent;
  MouseButtonEvent mMouseButtonEvent;
  CursorPosEvent mCursorPosEvent;
  ScrollEvent mScrollEvent;
  CursorEnterEvent mCursorEnterEvent;
};
} // namespace ne

