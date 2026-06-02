#pragma once

#include "platform/window.h"
#include "renderer/device.h"
#include "renderer/pipeline.h"

namespace ne {
class Application {
public:
  static constexpr int WIDTH = 1200;
  static constexpr int HEIGHT = 1000;

  void run();

private:
  Window mWindow{WIDTH, HEIGHT, "Hello Vulkan"};
  Device mDevice{mWindow};
  Pipeline mPipeline{mDevice, "shaders/triangle.vert.spv",
                       "shaders/triangle.frag.spv",
                       Pipeline::defaultConfigInfo(WIDTH, HEIGHT)};
};
} // namespace ne
