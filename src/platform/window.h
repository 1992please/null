#pragma once

#include <volk/volk.h>
#include "core/event.h"
#include "platform/input_types.h"

// std
#include <string>
#include <vector>

struct GLFWwindow;
struct GLFWmonitor;

namespace ne {

class Window {
public:
  using FrameBufferResizeEvent = Event<int32_t, int32_t>;
  using WindowFocusEvent = Event<bool>;

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

  // Window Cursor & Hardware State
  void getCursorPos(double* oXpos, double* oYpos) const;
  void setCursorMode(CursorMode mode);

  // Encapsulated Callback Subscriptions
  CallbackId addFrameBufferResizeCallback(FrameBufferResizeEvent::Callback iCallback) { return mFrameBufferResizeEvent.add(std::move(iCallback)); }
  CallbackId addWindowFocusCallback(WindowFocusEvent::Callback iCallback) { return mWindowFocusEvent.add(std::move(iCallback)); }

  void removeFrameBufferResizeCallback(CallbackId iId) { mFrameBufferResizeEvent.remove(iId); }
  void removeWindowFocusCallback(CallbackId iId) { mWindowFocusEvent.remove(iId); }

private:
  static void framebufferResizeCallback(GLFWwindow* iWindow, int iWidth, int iHeight);
  static void windowFocusCallback(GLFWwindow* iWindow, int iFocused);

  // screen coordinates width and height
  GLFWwindow* mWindow;

  FrameBufferResizeEvent mFrameBufferResizeEvent;
  WindowFocusEvent mWindowFocusEvent;
};

} // namespace ne
