#include "application.h"
#include "core/assert.h"

namespace ne {

void Application::run() {
  NE_LOG("Application Start!");

  while(!mWindow.shouldClose()) {
    glfwPollEvents();
  }

  NE_LOG("Application Done!");
}

} // namespace ne
