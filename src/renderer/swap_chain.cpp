#include "renderer/swap_chain.h"
#include "renderer/utils.h"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>

namespace ne {

SwapChain::SwapChain(Device &deviceRef, VkExtent2D extent)
    : mDevice{deviceRef}, mWindowExtent{extent} {
  init();
}

SwapChain::SwapChain(Device &deviceRef, VkExtent2D extent,
                     std::shared_ptr<SwapChain> previous)
    : mDevice{deviceRef}, mWindowExtent{extent}, mOldSwapChain(previous) {
  init();

  // clean up old swap chain since it's no longer needed
  mOldSwapChain.reset();
}

void SwapChain::init() {
  createSwapChain();
  createImageViews();
  createRenderPass();
  createDepthResources();
  createFramebuffers();
  createSyncObjects();
}

SwapChain::~SwapChain() {
  for (auto imageView : mSwapChainImageViews) {
    vkDestroyImageView(mDevice.device(), imageView, nullptr);
  }
  mSwapChainImageViews.clear();

  if (mSwapChain != nullptr) {
    vkDestroySwapchainKHR(mDevice.device(), mSwapChain, nullptr);
    mSwapChain = nullptr;
  }

  for (size_t i = 0; i < mDepthImages.size(); i++) {
    vkDestroyImageView(mDevice.device(), mDepthImageViews[i], nullptr);
    vkDestroyImage(mDevice.device(), mDepthImages[i], nullptr);
    vkFreeMemory(mDevice.device(), mDepthImageMemorys[i], nullptr);
  }

  for (auto framebuffer : mSwapChainFramebuffers) {
    vkDestroyFramebuffer(mDevice.device(), framebuffer, nullptr);
  }

  vkDestroyRenderPass(mDevice.device(), mRenderPass, nullptr);

  // cleanup synchronization objects
  for (size_t i = 0; i < mImageAvailableSemaphores.size(); i++) {
    vkDestroySemaphore(mDevice.device(), mImageAvailableSemaphores[i], nullptr);
  }
  for (size_t i = 0; i < mRenderFinishedSemaphores.size(); i++) {
    vkDestroySemaphore(mDevice.device(), mRenderFinishedSemaphores[i], nullptr);
  }
  for (size_t i = 0; i < mInFlightFences.size(); i++) {
    vkDestroyFence(mDevice.device(), mInFlightFences[i], nullptr);
  }
}

VkResult SwapChain::acquireNextImage(uint32_t *iImageIndex) {
  vkWaitForFences(mDevice.device(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE,
                  std::numeric_limits<uint64_t>::max());

  VkResult result = vkAcquireNextImageKHR(
      mDevice.device(), mSwapChain, std::numeric_limits<uint64_t>::max(),
      mImageAvailableSemaphores[mCurrentFrame], // must be a not signaled
                                              // semaphore
      VK_NULL_HANDLE, iImageIndex);

  return result;
}

VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer *iBuffers,
                                         uint32_t *iImageIndex) {
  if (mImagesInFlight[*iImageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(mDevice.device(), 1, &mImagesInFlight[*iImageIndex], VK_TRUE,
                    UINT64_MAX);
  }
  mImagesInFlight[*iImageIndex] = mInFlightFences[mCurrentFrame];

  VkSemaphore acquireSemaphore = mImageAvailableSemaphores[mCurrentFrame];
  VkSemaphore submitSemaphore = mRenderFinishedSemaphores[*iImageIndex];

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &acquireSemaphore;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = iBuffers;

  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &submitSemaphore;

  vkResetFences(mDevice.device(), 1, &mInFlightFences[mCurrentFrame]);
  VK_CHECK(vkQueueSubmit(mDevice.graphicsQueue(), 1, &submitInfo, mInFlightFences[mCurrentFrame]));

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &submitSemaphore;

  VkSwapchainKHR swapChains[] = {mSwapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = iImageIndex;

  auto result = vkQueuePresentKHR(mDevice.presentQueue(), &presentInfo);

  mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
}

void SwapChain::createSwapChain() {
  SwapChainSupportDetails swapChainSupport = mDevice.getSwapChainSupport();

  VkSurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.mFormats);
  VkPresentModeKHR presentMode =
      chooseSwapPresentMode(swapChainSupport.mPresentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.mCapabilities);

  uint32_t imageCount = swapChainSupport.mCapabilities.minImageCount + 1;
  if (swapChainSupport.mCapabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.mCapabilities.maxImageCount) {
    imageCount = swapChainSupport.mCapabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = mDevice.surface();

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = mDevice.findPhysicalQueueFamilies();
  uint32_t queueFamilyIndices[] = {indices.mGraphicsFamily,
                                   indices.mPresentFamily};

  if (indices.mGraphicsFamily != indices.mPresentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;     // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  }

  createInfo.preTransform = swapChainSupport.mCapabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain =
      mOldSwapChain ? mOldSwapChain->mSwapChain : VK_NULL_HANDLE;

  VK_CHECK(
      vkCreateSwapchainKHR(mDevice.device(), &createInfo, nullptr, &mSwapChain));

  // we only specified a minimum number of images in the swap chain, so the
  // implementation is allowed to create a swap chain with more. That's why
  // we'll first query the final number of images with vkGetSwapchainImagesKHR,
  // then resize the container and finally call it again to retrieve the
  // handles.
  vkGetSwapchainImagesKHR(mDevice.device(), mSwapChain, &imageCount, nullptr);
  mSwapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(mDevice.device(), mSwapChain, &imageCount,
                          mSwapChainImages.data());

  mSwapChainImageFormat = surfaceFormat.format;
  mSwapChainExtent = extent;
}

void SwapChain::createImageViews() {
  mSwapChainImageViews.resize(mSwapChainImages.size());
  for (size_t i = 0; i < mSwapChainImages.size(); i++) {
    mSwapChainImageViews[i] =
        mDevice.createImageView(mSwapChainImages[i], mSwapChainImageFormat);
  }
}

void SwapChain::createRenderPass() {
  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = findDepthFormat();
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = getSwapChainImageFormat();
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.srcAccessMask = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstSubpass = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment,
                                                        depthAttachment};
  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  VK_CHECK(vkCreateRenderPass(mDevice.device(), &renderPassInfo, nullptr,
                              &mRenderPass));
}

void SwapChain::createFramebuffers() {
  mSwapChainFramebuffers.resize(imageCount());
  for (size_t i = 0; i < imageCount(); i++) {
    std::array<VkImageView, 2> attachments = {mSwapChainImageViews[i],
                                              mDepthImageViews[i]};

    VkExtent2D swapChainExtent = getSwapChainExtent();
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = mRenderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(mDevice.device(), &framebufferInfo, nullptr,
                                 &mSwapChainFramebuffers[i]));
  }
}

void SwapChain::createDepthResources() {
  VkFormat depthFormat = findDepthFormat();
  mSwapChainDepthFormat = depthFormat;
  VkExtent2D swapChainExtent = getSwapChainExtent();

  mDepthImages.resize(imageCount());
  mDepthImageMemorys.resize(imageCount());
  mDepthImageViews.resize(imageCount());

  for (size_t i = 0; i < mDepthImages.size(); i++) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    mDevice.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               mDepthImages[i], mDepthImageMemorys[i]);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = mDepthImages[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(mDevice.device(), &viewInfo, nullptr,
                               &mDepthImageViews[i]));
  }
}

void SwapChain::createSyncObjects() {
  mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  mRenderFinishedSemaphores.resize(imageCount());
  mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  mImagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < mImageAvailableSemaphores.size(); i++) {
    VK_CHECK(vkCreateSemaphore(mDevice.device(), &semaphoreInfo, nullptr,
                               &mImageAvailableSemaphores[i]));
  }
  for (size_t i = 0; i < mRenderFinishedSemaphores.size(); i++) {
    VK_CHECK(vkCreateSemaphore(mDevice.device(), &semaphoreInfo, nullptr,
                               &mRenderFinishedSemaphores[i]));
  }
  for (size_t i = 0; i < mInFlightFences.size(); i++) {
    VK_CHECK(vkCreateFence(mDevice.device(), &fenceInfo, nullptr,
                           &mInFlightFences[i]));
  }
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &iAvailableFormats) {
  for (const auto &availableFormat : iAvailableFormats) {
    // https://docs.google.com/document/d/1DsppK1HviSpwJ_sg-npY9zQsOwH-VPGQyvYRh5_eKXA/edit?usp=sharing
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return iAvailableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &iAvailablePresentModes) {
  for (const auto &availablePresentMode : iAvailablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      NE_LOG("Present mode: Mailbox");
      return availablePresentMode;
    }
  }

  // for (const auto &availablePresentMode : availablePresentModes) {
  //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
  //     std::cout << "Present mode: Immediate" << std::endl;
  //     return availablePresentMode;
  //   }
  // }

  NE_LOG("Present mode: V-Sync");
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &iCapabilities) {
  if (iCapabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return iCapabilities.currentExtent;
  } else {
    VkExtent2D actualExtent = mWindowExtent;
    actualExtent.width = std::max(
        iCapabilities.minImageExtent.width,
        std::min(iCapabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
        iCapabilities.minImageExtent.height,
        std::min(iCapabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

VkFormat SwapChain::findDepthFormat() {
  return mDevice.findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

} // namespace ne
