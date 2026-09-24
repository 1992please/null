#include "renderer/renderer.h"
#include "core/assert.h"
#include "core/defines.h"
#include "core/logger.h"
#include "platform/window.h"
#include "renderer/buffer.h"
#include "renderer/device.h"
#include "renderer/image.h"
#include "renderer/instance.h"
#include "renderer/utils.h"

// std
#include <algorithm>
#include <cstring>

namespace ne {

Renderer::Renderer(Window* iWindow, const std::string& iEngineName, const std::string& iAppName)
    : mWindow(iWindow), mEngineName(iEngineName), mAppName(iAppName) {
  NE_ASSERT(mWindow);
  mFrameBufferResizeCallbackId = mWindow->addFrameBufferResizeCallback([this](int32_t width, int32_t height) {
    NE_UNUSED(width);
    NE_UNUSED(height);
    mFrameBufferResized = true;
  });

  // 1. Initialize Vulkan Instance runtime
  Instance::Config instanceConfig{
      .engineName = mEngineName,
      .appName = mAppName,
      .requiredExtensions = mWindow->getRequiredInstanceExtensions(),
  };
  mInstance = std::make_unique<Instance>(instanceConfig);

  // 2. Create Surface from Window & Instance
  VK_CHECK(mWindow->createWindowSurface(mInstance->getInstance(), &mSurface));

  // 3. Initialize Hardware Device passing Surface via Config
  Device::Config deviceConfig{
      .surface = mSurface,
  };
  mDevice = std::make_unique<Device>(mInstance.get(), deviceConfig);

  createSwapChain();
  createDepthImage();
  createFramesResources();
}

Renderer::~Renderer() {
  NE_LOG("Destroying Vulkan Renderer and deallocating resources...");

  if (mWindow && mFrameBufferResizeCallbackId != 0)
    mWindow->removeFrameBufferResizeCallback(mFrameBufferResizeCallbackId);

  for (FrameResources& frame : mFrames) {
    vkDestroyCommandPool(mDevice->getDevice(), frame.mCommandPool, nullptr);
    vkDestroySemaphore(mDevice->getDevice(), frame.mPresentCompleteSemaphore, nullptr);
    vkDestroyFence(mDevice->getDevice(), frame.mDrawFence, nullptr);
    frame.mUploadBuffer.reset();
  }

  mDepthImage.reset();

  for (SwapchainImageResources& image : mSwapChainImages) {
    vkDestroyImageView(mDevice->getDevice(), image.mImageView, nullptr);
    vkDestroySemaphore(mDevice->getDevice(), image.mRenderFinishedSemaphore, nullptr);
  }
  mSwapChainImages.clear();

  vkDestroySwapchainKHR(mDevice->getDevice(), mSwapChain, nullptr);

  mDevice.reset();

  if (mSurface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(mInstance->getInstance(), mSurface, nullptr);
    mSurface = VK_NULL_HANDLE;
  }

  mInstance.reset();

  NE_LOG("Vulkan Renderer destroyed successfully.");
}

void Renderer::createSwapChain(VkSwapchainKHR iOldSwapchain) {
  Device::SwapChainSupportDetails swapChainSupport = mDevice->querySwapChainSupport(mSurface);
  VkSurfaceCapabilitiesKHR surfaceCapabilities = swapChainSupport.mCapabilities;

  // Surface format
  NE_ASSERT(!swapChainSupport.mFormats.empty());
  VkSurfaceFormatKHR selectedSurfaceFormat = swapChainSupport.mFormats[0];
  for (const auto& surfaceFormat : swapChainSupport.mFormats) {
    if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      selectedSurfaceFormat = surfaceFormat;
      break;
    }
  }
  // Present Mode
  NE_ASSERT(!swapChainSupport.mPresentModes.empty());
  VkPresentModeKHR selectedPresentMode = swapChainSupport.mPresentModes[0];
  for (const auto& presentMode : swapChainSupport.mPresentModes) {
    if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      selectedPresentMode = presentMode;
      break;
    }

    if (presentMode == VK_PRESENT_MODE_FIFO_KHR) {
      selectedPresentMode = presentMode;
    }
  }
  NE_ASSERT(selectedPresentMode == VK_PRESENT_MODE_MAILBOX_KHR || selectedPresentMode == VK_PRESENT_MODE_FIFO_KHR);

  VkExtent2D selectedSwapExtent = surfaceCapabilities.currentExtent;
  if (surfaceCapabilities.currentExtent.width == UINT32_MAX) // are we allow to differ?
  {
    int32_t frameBufferWidth, frameBufferHeight;
    mWindow->getFrameBufferSize(&frameBufferWidth, &frameBufferHeight);
    selectedSwapExtent = {.width = std::clamp<uint32_t>(frameBufferWidth, surfaceCapabilities.minImageExtent.width,
                                                        surfaceCapabilities.maxImageExtent.width),
                          .height = std::clamp<uint32_t>(frameBufferHeight, surfaceCapabilities.minImageExtent.height,
                                                         surfaceCapabilities.maxImageExtent.height)};
  }
  // Minimum image count
  uint32_t minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount)) {
    minImageCount = surfaceCapabilities.maxImageCount;
  }
  VkSwapchainCreateInfoKHR swapchainCreateInfo{};
  swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.surface = mSurface;
  swapchainCreateInfo.minImageCount = minImageCount;
  swapchainCreateInfo.imageFormat = selectedSurfaceFormat.format;
  swapchainCreateInfo.imageColorSpace = selectedSurfaceFormat.colorSpace;
  swapchainCreateInfo.imageExtent = selectedSwapExtent;
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode = selectedPresentMode;
  swapchainCreateInfo.clipped = VK_TRUE;
  swapchainCreateInfo.oldSwapchain = iOldSwapchain;

  VK_CHECK(vkCreateSwapchainKHR(mDevice->getDevice(), &swapchainCreateInfo, nullptr, &mSwapChain));
  vk_utils::setDebugObjectName(mDevice->getDevice(), mSwapChain, "Main_Swapchain");

  mSwapChainSurfaceFormat = selectedSurfaceFormat;
  mSwapChainExtent = selectedSwapExtent;

  // Clear old SwapChainResources if it exists
  for (SwapchainImageResources& image : mSwapChainImages) {
    vkDestroyImageView(mDevice->getDevice(), image.mImageView, nullptr);
    vkDestroySemaphore(mDevice->getDevice(), image.mRenderFinishedSemaphore, nullptr);
  }
  mSwapChainImages.clear();

  // SwapChain images
  uint32_t swapchainImagesCount = 0;
  vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &swapchainImagesCount, nullptr);
  std::vector<VkImage> swapChainImages(swapchainImagesCount);
  vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &swapchainImagesCount, swapChainImages.data());

  mSwapChainImages.resize(swapChainImages.size());
  for (size_t i = 0; i < swapChainImages.size(); i++) {
    mSwapChainImages[i].mImage = swapChainImages[i];

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = swapChainImages[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = mSwapChainSurfaceFormat.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &mSwapChainImages[i].mImageView));
    // Create Semaphore for starting display to the image
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VK_CHECK(
        vkCreateSemaphore(mDevice->getDevice(), &semaphoreCreateInfo, nullptr, &mSwapChainImages[i].mRenderFinishedSemaphore));

    vk_utils::setDebugObjectName(mDevice->getDevice(), mSwapChainImages[i].mImage, std::format("Swapchain_Image_{}", i).c_str());
    vk_utils::setDebugObjectName(mDevice->getDevice(), mSwapChainImages[i].mImageView,
                                 std::format("Swapchain_ImageView_{}", i).c_str());
    vk_utils::setDebugObjectName(mDevice->getDevice(), mSwapChainImages[i].mRenderFinishedSemaphore,
                                 std::format("RenderFinished_Semaphore_{}", i).c_str());
  }

  NE_LOG("Created new swapChain, Present mode: {}, Image count: {}, Image size: {} x {}",
         selectedPresentMode == VK_PRESENT_MODE_FIFO_KHR ? "V-Sync" : "Mailbox", swapChainImages.size(), selectedSwapExtent.width,
         selectedSwapExtent.height);
}

std::unique_ptr<Buffer> Renderer::createUploadBuffer(VkDeviceSize size, std::string iDebugName) {
  Buffer::Config config{
      .size = size,
      .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
      .properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      .debugName = std::move(iDebugName),
  };
  auto uploadBuffer = std::make_unique<Buffer>(mDevice.get(), config);
  uploadBuffer->mapMemory();
  return uploadBuffer;
}

void Renderer::createFramesResources() {
  NE_ASSERT(mFrames.empty());
  mFrames.resize(MAX_FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < mFrames.size(); i++) {
    // We create a command Pool per frame because reseting it
    // Reclaims all command memory in one bulk operation, eliminating fragmentation
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = mDevice->getQueueFamilyIndex();
    VK_CHECK(vkCreateCommandPool(mDevice->getDevice(), &commandPoolCreateInfo, nullptr, &mFrames[i].mCommandPool));

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = mFrames[i].mCommandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;
    VK_CHECK(vkAllocateCommandBuffers(mDevice->getDevice(), &commandBufferAllocateInfo, &mFrames[i].mCommandBuffer));

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VK_CHECK(vkCreateSemaphore(mDevice->getDevice(), &semaphoreCreateInfo, nullptr, &mFrames[i].mPresentCompleteSemaphore));

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK(vkCreateFence(mDevice->getDevice(), &fenceCreateInfo, nullptr, &mFrames[i].mDrawFence));

    mFrames[i].mUploadBuffer = createUploadBuffer(vk_utils::UPLOAD_BUFFER_SIZE, std::format("UploadBuffer_Frame_{}", i));

    vk_utils::setDebugObjectName(mDevice->getDevice(), mFrames[i].mCommandPool, std::format("Frame_CommandPool_{}", i).c_str());
    vk_utils::setDebugObjectName(mDevice->getDevice(), mFrames[i].mCommandBuffer,
                                 std::format("Frame_CommandBuffer_{}", i).c_str());
    vk_utils::setDebugObjectName(mDevice->getDevice(), mFrames[i].mPresentCompleteSemaphore,
                                 std::format("PresentComplete_Semaphore_{}", i).c_str());
    vk_utils::setDebugObjectName(mDevice->getDevice(), mFrames[i].mDrawFence, std::format("Draw_Fence_{}", i).c_str());
  }
}

VkCommandBuffer Renderer::beginFrame() {
  auto& currentFrame = mFrames[mFrameIndex];

  VK_CHECK(vkWaitForFences(mDevice->getDevice(), 1, &currentFrame.mDrawFence, VK_TRUE, UINT64_MAX));

  VkResult result = vkAcquireNextImageKHR(mDevice->getDevice(), mSwapChain, UINT64_MAX, currentFrame.mPresentCompleteSemaphore,
                                          VK_NULL_HANDLE, &mSwapChainImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain(true);
    return VK_NULL_HANDLE;
  }
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    NE_ASSERT(false, "failed to acquire swap chain image!");
  }

  vkResetFences(mDevice->getDevice(), 1, &currentFrame.mDrawFence);

  vkResetCommandPool(mDevice->getDevice(), currentFrame.mCommandPool, 0);

  // Reset transient uniform allocations for this frame
  currentFrame.mUploadBuffer->resetUploadOffset();

  VkCommandBufferBeginInfo commandBufferBeginInfo{};
  commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  VK_CHECK(vkBeginCommandBuffer(currentFrame.mCommandBuffer, &commandBufferBeginInfo));

  return currentFrame.mCommandBuffer;
}

void Renderer::recreateUploadBuffer(VkDeviceSize newSize) {
  auto& currentFrame = mFrames[mFrameIndex];

  NE_LOG("Upload buffer resizing from {} to {} bytes", currentFrame.mUploadBuffer->getConfig().size, newSize);

  currentFrame.mUploadBuffer = createUploadBuffer(newSize, std::format("UploadBuffer_Frame_{}", mFrameIndex));
}

void Renderer::endFrame() {
  auto& currentFrame = mFrames[mFrameIndex];
  VK_CHECK(vkEndCommandBuffer(currentFrame.mCommandBuffer));

  VkSemaphoreSubmitInfo waitSemaphoreInfo{};
  waitSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  waitSemaphoreInfo.semaphore = currentFrame.mPresentCompleteSemaphore;
  waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkCommandBufferSubmitInfo commandBufferInfo{};
  commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  commandBufferInfo.commandBuffer = currentFrame.mCommandBuffer;

  VkSemaphoreSubmitInfo signalSemaphoreInfo{};
  signalSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  signalSemaphoreInfo.semaphore = mSwapChainImages[mSwapChainImageIndex].mRenderFinishedSemaphore;
  signalSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

  VkSubmitInfo2 submitInfo2{};
  submitInfo2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  submitInfo2.pNext = NULL;
  submitInfo2.flags = 0;
  submitInfo2.waitSemaphoreInfoCount = 1;
  submitInfo2.pWaitSemaphoreInfos = &waitSemaphoreInfo;
  submitInfo2.commandBufferInfoCount = 1;
  submitInfo2.pCommandBufferInfos = &commandBufferInfo;
  submitInfo2.signalSemaphoreInfoCount = 1;
  submitInfo2.pSignalSemaphoreInfos = &signalSemaphoreInfo;
  VK_CHECK(vkQueueSubmit2(mDevice->getQueue(), 1, &submitInfo2, currentFrame.mDrawFence));

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &mSwapChainImages[mSwapChainImageIndex].mRenderFinishedSemaphore;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &mSwapChain;
  presentInfo.pImageIndices = &mSwapChainImageIndex;
  VkResult result = vkQueuePresentKHR(mDevice->getQueue(), &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain(true);
  } else if (result == VK_SUBOPTIMAL_KHR || mFrameBufferResized) {
    mFrameBufferResized = false;
    recreateSwapChain(false);
  } else if (result != VK_SUCCESS) {
    NE_ASSERT(false, "failed to present swap chain image!");
  }

  mFrameIndex = (mFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::recreateSwapChain(bool iForceRecreate) {
  int32_t width = 0, height = 0;
  mWindow->getFrameBufferSize(&width, &height);
  while (width == 0 || height == 0) {
    mWindow->waitEvents();
    mWindow->getFrameBufferSize(&width, &height);
  }

  if (!iForceRecreate && width == static_cast<int32_t>(mSwapChainExtent.width) &&
      height == static_cast<int32_t>(mSwapChainExtent.height)) {
    return;
  }

  mDevice->waitIdle();
  NE_LOG("Recreating SwapChain... New resolution: {}x{}", width, height);

  VkSwapchainKHR oldSwapChain = mSwapChain;
  mSwapChain = VK_NULL_HANDLE;

  // Create the new swapchain, passing the old swapchain for resource recycling
  createSwapChain(oldSwapChain);
  createDepthImage();

  // Safely destroy the old swapchain now that the new one is created
  if (oldSwapChain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(mDevice->getDevice(), oldSwapChain, nullptr);
  }
}

void Renderer::createDepthImage() {
  VkFormat depthFormat = mDevice->findDepthFormat();

  Image::Config depthConfig{
      .width = mSwapChainExtent.width,
      .height = mSwapChainExtent.height,
      .format = depthFormat,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .mipLevels = 1,
      .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
      .debugName = "Depth_Image",
  };

  mDepthImage = std::make_unique<Image>(mDevice.get(), depthConfig);

  NE_LOG("Created Depth Attachment resources: Format {}, Extent {}x{}", string_VkFormat(depthFormat), mSwapChainExtent.width,
         mSwapChainExtent.height);
}

} // namespace ne
