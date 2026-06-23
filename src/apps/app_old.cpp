#include "apps/app_old.h"
#include "core/core.h"
#include "platform/window.h"
#include "renderer/pipeline.h"
#include "renderer/utils.h"

// std
#include <array>

namespace ne {

  AppOld::AppOld()
    : mWindow(mWidth, mHeight, "Hello Vulkan")
    , mDevice(mWindow)
    , mSwapChain(mDevice, mWindow.getExtent()) {

    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
  }

  AppOld::~AppOld() {
    vkDestroyPipelineLayout(mDevice.device(), mPipelineLayout, nullptr);
  }

  void AppOld::run() {
    NE_LOG("AppOld Start!");

    while (!mWindow.shouldClose()) {
      mWindow.processEvents();
      drawFrame();
    }
    VK_CHECK(vkDeviceWaitIdle(mDevice.device()));

    NE_LOG("AppOld Done!");
  }

  void AppOld::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    VK_CHECK(vkCreatePipelineLayout(mDevice.device(), &pipelineLayoutInfo, nullptr, &mPipelineLayout));
  }

  void AppOld::createPipeline() {
    auto pipelineConfig = Pipeline::defaultConfigInfo(mSwapChain.width(), mSwapChain.height());
    pipelineConfig.mRenderPass = mSwapChain.getRenderPass();
    pipelineConfig.mPipelineLayout = mPipelineLayout;
    mPipeline = std::make_unique<Pipeline>(mDevice, "compiled_shaders/triangle.vert.spv",
      "compiled_shaders/triangle.frag.spv",
      pipelineConfig);
  }

  void AppOld::createCommandBuffers() {
    mCommandBuffers.resize(mSwapChain.imageCount());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = mDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

    VK_CHECK(vkAllocateCommandBuffers(mDevice.device(), &allocInfo, mCommandBuffers.data()));

    for (size_t i = 0; i < mCommandBuffers.size(); i++)
    {
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      VK_CHECK(vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo));

      VkRenderPassBeginInfo renderPassInfo{};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassInfo.renderPass = mSwapChain.getRenderPass();
      renderPassInfo.framebuffer = mSwapChain.getFrameBuffer(i);

      renderPassInfo.renderArea.offset = { 0, 0 };
      renderPassInfo.renderArea.extent = mSwapChain.getSwapChainExtent();
      std::array<VkClearValue, 2> clearValues{};
      clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
      clearValues[1].depthStencil = { 1.0f, 0 };
      renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
      renderPassInfo.pClearValues = clearValues.data();

      vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

      mPipeline->bind(mCommandBuffers[i]);
      vkCmdDraw(mCommandBuffers[i], 3, 1, 0, 0);

      vkCmdEndRenderPass(mCommandBuffers[i]);

      VK_CHECK(vkEndCommandBuffer(mCommandBuffers[i]));
    }
  }

  void AppOld::drawFrame() {
    uint32_t imageIndex;
    VK_CHECK(mSwapChain.acquireNextImage(&imageIndex));

    VK_CHECK(mSwapChain.submitCommandBuffers(&mCommandBuffers[imageIndex], &imageIndex));
  }

} // namespace ne
