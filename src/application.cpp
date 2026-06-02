#include "application.h"
#include "core/core.h"
#include "platform/window.h"
#include "renderer/pipeline.h"

namespace ne {

void Application::run() {
  NE_LOG("Application Start!");

  while(!mWindow.shouldClose()) {
    glfwPollEvents();
  }

  NE_LOG("Application Done!");
}

} // namespace ne
