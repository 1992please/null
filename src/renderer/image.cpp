#include "renderer/image.h"
#include "core/assert.h"
#include "core/logger.h"
#include "renderer/renderer.h"
#include "renderer/utils.h"

namespace ne {

Image::Image(Renderer* iRenderer, const Config& iConfig) : mRenderer(iRenderer), mConfig(iConfig) {
  NE_ASSERT(mRenderer, "Renderer must not be null");
  NE_ASSERT(mConfig.width > 0 && mConfig.height > 0, "Image dimensions must be greater than 0");

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = mConfig.width;
  imageInfo.extent.height = mConfig.height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = mConfig.mipLevels;
  imageInfo.arrayLayers = 1;
  imageInfo.format = mConfig.format;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = mConfig.usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  mRenderer->createImage(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mImage, mImageMemory);
  mImageView = mRenderer->createImageView(mImage, mConfig.format, VK_IMAGE_ASPECT_COLOR_BIT);

  mCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  mCurrentAccessMask = VK_ACCESS_2_NONE;
  mCurrentStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

  if (!mConfig.debugName.empty()) {
    vk_utils::setDebugObjectName(mRenderer->getDevice(), mImage, mConfig.debugName);
    vk_utils::setDebugObjectName(mRenderer->getDevice(), mImageMemory, mConfig.debugName + "_Memory");
    vk_utils::setDebugObjectName(mRenderer->getDevice(), mImageView, mConfig.debugName + "_View");
  }

  NE_LOG("Allocated Image{}: Extent {}x{} | Format: {}", mConfig.debugName.empty() ? "" : std::format(" '{}'", mConfig.debugName),
         mConfig.width, mConfig.height, string_VkFormat(mConfig.format));
}

Image::~Image() { releaseResources(); }

void Image::releaseResources() {
  if (mRenderer && mRenderer->getDevice() != VK_NULL_HANDLE) {
    VkDevice device = mRenderer->getDevice();
    if (mImageView != VK_NULL_HANDLE) {
      vkDestroyImageView(device, mImageView, nullptr);
      mImageView = VK_NULL_HANDLE;
    }
    if (mImage != VK_NULL_HANDLE) {
      vkDestroyImage(device, mImage, nullptr);
      mImage = VK_NULL_HANDLE;
    }
    if (mImageMemory != VK_NULL_HANDLE) {
      vkFreeMemory(device, mImageMemory, nullptr);
      mImageMemory = VK_NULL_HANDLE;
    }
  }
}

} // namespace ne
