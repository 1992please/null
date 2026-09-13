#include "renderer/image.h"
#include "core/assert.h"
#include "core/image_data.h"
#include "core/logger.h"
#include "renderer/buffer.h"
#include "renderer/renderer.h"
#include "renderer/utils.h"

namespace ne {

Image::Image(Renderer* iRenderer, const Config& iConfig, const void* iPixelData, VkDeviceSize iSize)
    : mRenderer(iRenderer), mConfig(iConfig) {
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

  if (iPixelData && iSize > 0) {
    uploadData(iPixelData, iSize);
  }
}

Image::Image(Renderer* iRenderer, const ImageData& iImageData, bool iSrgb, std::string iDebugName)
    : Image(iRenderer,
            Config{
                .width = iImageData.mWidth,
                .height = iImageData.mHeight,
                .format = iSrgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM,
                .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                .mipLevels = 1,
                .debugName = std::move(iDebugName),
            },
            iImageData.mPixels, iImageData.getSizeInBytes()) {}

Image::~Image() {
  releaseResources();
}

Image::Image(Image&& other)
    : mRenderer(other.mRenderer),
      mConfig(std::move(other.mConfig)),
      mImage(other.mImage),
      mImageMemory(other.mImageMemory),
      mImageView(other.mImageView),
      mCurrentLayout(other.mCurrentLayout),
      mCurrentAccessMask(other.mCurrentAccessMask),
      mCurrentStageMask(other.mCurrentStageMask) {
  other.mImage = VK_NULL_HANDLE;
  other.mImageMemory = VK_NULL_HANDLE;
  other.mImageView = VK_NULL_HANDLE;
  other.mRenderer = nullptr;
  other.mCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  other.mCurrentAccessMask = VK_ACCESS_2_NONE;
  other.mCurrentStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
}

Image& Image::operator=(Image&& other) {
  if (this != &other) {
    releaseResources();

    mRenderer = other.mRenderer;
    mConfig = std::move(other.mConfig);
    mImage = other.mImage;
    mImageMemory = other.mImageMemory;
    mImageView = other.mImageView;
    mCurrentLayout = other.mCurrentLayout;
    mCurrentAccessMask = other.mCurrentAccessMask;
    mCurrentStageMask = other.mCurrentStageMask;

    other.mImage = VK_NULL_HANDLE;
    other.mImageMemory = VK_NULL_HANDLE;
    other.mImageView = VK_NULL_HANDLE;
    other.mRenderer = nullptr;
    other.mCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    other.mCurrentAccessMask = VK_ACCESS_2_NONE;
    other.mCurrentStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  }
  return *this;
}

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

void Image::transitionLayout(VkCommandBuffer iCommandBuffer, VkImageLayout iNewLayout, VkAccessFlags2 iDstAccessMask,
                             VkPipelineStageFlags2 iDstStageMask) {
  if (mCurrentLayout == iNewLayout) {
    return;
  }

  mRenderer->transitionImageLayout(iCommandBuffer, mImage, VK_IMAGE_ASPECT_COLOR_BIT, mCurrentLayout, iNewLayout,
                                   mCurrentAccessMask, iDstAccessMask, mCurrentStageMask, iDstStageMask);

  mCurrentLayout = iNewLayout;
  mCurrentAccessMask = iDstAccessMask;
  mCurrentStageMask = iDstStageMask;
}

void Image::transitionLayout(VkImageLayout iNewLayout, VkAccessFlags2 iDstAccessMask, VkPipelineStageFlags2 iDstStageMask) {
  if (mCurrentLayout == iNewLayout) {
    return;
  }

  VkCommandBuffer cmd = mRenderer->beginOneTimeCommand();
  transitionLayout(cmd, iNewLayout, iDstAccessMask, iDstStageMask);
  mRenderer->endOneTimeCommand(cmd);
}

void Image::uploadData(const void* iPixelData, VkDeviceSize iSize) {
  NE_ASSERT(iPixelData && iSize > 0, "Invalid pixel data for image upload");

  Buffer stagingBuffer(mRenderer, iSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       mConfig.debugName.empty() ? "Image_Staging" : mConfig.debugName + "_Staging");
  stagingBuffer.writeToBuffer(iPixelData, iSize, 0);

  VkCommandBuffer cmd = mRenderer->beginOneTimeCommand();

  // 1. Transition image to TRANSFER_DST_OPTIMAL
  transitionLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_COPY_BIT);

  // 2. Execute buffer-to-image copy via modern Vulkan 1.3/1.4 CopyBufferToImage2
  mRenderer->copyBufferToImage(cmd, stagingBuffer.getBuffer(), mImage, mConfig.width, mConfig.height);

  // 3. Transition image to SHADER_READ_ONLY_OPTIMAL
  transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_SHADER_READ_BIT,
                   VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

  mRenderer->endOneTimeCommand(cmd);
}

} // namespace ne
