#pragma once

#include "window.h"

namespace ne {
class Application {
public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  void run();

private:
  Window mWindow{WIDTH, HEIGHT, "Hello Vulkan"};
};
} // namespace ne
