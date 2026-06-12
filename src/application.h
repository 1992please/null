#pragma once

#include "platform/window.h"
#include "renderer/device.h"
#include "renderer/pipeline.h"
#include "renderer/swap_chain.h"

// std
#include <memory>
#include <vector>

namespace ne {
class Application {
public:
  static constexpr int WIDTH = 1200;
  static constexpr int HEIGHT = 1000;

  Application();
  ~Application();

  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

  void run();

private:
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void drawFrame();

  Window mWindow{WIDTH, HEIGHT, "Hello Vulkan"};
  Device mDevice{mWindow};
  SwapChain mSwapChain{mDevice, mWindow.getExtent()};
  std::unique_ptr<Pipeline> mPipeline;
  VkPipelineLayout mPipelineLayout;
  std::vector<VkCommandBuffer> mCommandBuffers;
};
} // namespace ne
