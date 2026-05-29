#include "application.h"

void Application::run() {
  if(mWindow.shouldClose()) {
    glfwPollEvents();
  }
}
} // namespace ne
