#pragma once

#include "apps/application.h"
#include "platform/window.h"
#include "renderer/device.h"
#include "renderer/pipeline.h"
#include "renderer/swap_chain.h"

// std
#include <memory>
#include <vector>

namespace ne {
class AppOld : public Application {
public:
  AppOld();
  ~AppOld();

  AppOld(const AppOld&) = delete;
  AppOld& operator=(const AppOld&) = delete;

  virtual void run() override;

private:
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void drawFrame();

  Window mWindow;
  Device mDevice;
  SwapChain mSwapChain;
  std::unique_ptr<Pipeline> mPipeline;
  VkPipelineLayout mPipelineLayout;
  std::vector<VkCommandBuffer> mCommandBuffers;
};
} // namespace ne
