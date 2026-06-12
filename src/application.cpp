#include "application.h"
#include "core/core.h"
#include "platform/window.h"
#include "renderer/pipeline.h"
#include "renderer/utils.h"

namespace ne {

Application::Application() {
  createPipelineLayout();
  createPipeline();
  createCommandBuffers();
}

Application::~Application() {
  vkDestroyPipelineLayout(mDevice.device(), mPipelineLayout, nullptr);
}

void Application::run() {
  NE_LOG("Application Start!");

  while(!mWindow.shouldClose()) {
    glfwPollEvents();
  }

  NE_LOG("Application Done!");
}

void Application::createPipelineLayout() {
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;

  VK_CHECK(vkCreatePipelineLayout(mDevice.device(), &pipelineLayoutInfo, nullptr, &mPipelineLayout));
}

void Application::createPipeline() {
  auto pipelineConfig = Pipeline::defaultConfigInfo(mSwapChain.width(), mSwapChain.height());
  pipelineConfig.mRenderPass = mSwapChain.getRenderPass();
  pipelineConfig.mPipelineLayout = mPipelineLayout;
  mPipeline = std::make_unique<Pipeline>(mDevice, "shaders/triangle.vert.spv",
                                         "shaders/triangle.frag.spv",
                                         pipelineConfig);
}

void Application::createCommandBuffers() {

}

void Application::drawFrame() {

}

} // namespace ne
