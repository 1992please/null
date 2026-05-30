#pragma once

#include "window.h"

namespace ne {
class Application {
public:
  static constexpr int WIDTH = 1200;
  static constexpr int HEIGHT = 1000;

  void run();

private:
  Window mWindow{WIDTH, HEIGHT, "Hello Vulkan"};
};
} // namespace ne
