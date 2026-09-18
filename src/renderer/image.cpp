#include "renderer/image.h"
#include "core/assert.h"
#include "core/logger.h"
#include "renderer/renderer.h"
#include "renderer/utils.h"

namespace ne {

VkImageAspectFlags Image::deduceAspectFlags(VkFormat format) {
  switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
      return VK_IMAGE_ASPECT_DEPTH_BIT;
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
      return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    case VK_FORMAT_S8_UINT:
      return VK_IMAGE_ASPECT_STENCIL_BIT;
    default:
      return VK_IMAGE_ASPECT_COLOR_BIT;
  }
}

Image::Image(Renderer* iRenderer, const Config& iConfig)
    : mDevice(iRenderer->getDevice()), mConfig(iConfig) {
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

  VK_CHECK(vkCreateImage(mDevice, &imageInfo, nullptr, &mImage));

  VkImageMemoryRequirementsInfo2 memReqsInfo2{};
  memReqsInfo2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
  memReqsInfo2.image = mImage;

  VkMemoryRequirements2 memReqs2{};
  memReqs2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
  vkGetImageMemoryRequirements2(mDevice, &memReqsInfo2, &memReqs2);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memReqs2.memoryRequirements.size;
  allocInfo.memoryTypeIndex =
      iRenderer->findMemoryType(memReqs2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  NE_ASSERT(allocInfo.memoryTypeIndex != ~0U, "Failed to find suitable memory type for Image!");

  VK_CHECK(vkAllocateMemory(mDevice, &allocInfo, nullptr, &mImageMemory));

  VkBindImageMemoryInfo bindImageInfo{};
  bindImageInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
  bindImageInfo.image = mImage;
  bindImageInfo.memory = mImageMemory;
  bindImageInfo.memoryOffset = 0;
  VK_CHECK(vkBindImageMemory2(mDevice, 1, &bindImageInfo));

  VkImageAspectFlags aspectMask =
      mConfig.aspectMask != 0 ? mConfig.aspectMask : deduceAspectFlags(mConfig.format);

  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = mImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = mConfig.format;
  viewInfo.subresourceRange.aspectMask = aspectMask;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = mConfig.mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VK_CHECK(vkCreateImageView(mDevice, &viewInfo, nullptr, &mImageView));

  mCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  mCurrentAccessMask = VK_ACCESS_2_NONE;
  mCurrentStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

  if (!mConfig.debugName.empty()) {
    vk_utils::setDebugObjectName(mDevice, mImage, mConfig.debugName);
    vk_utils::setDebugObjectName(mDevice, mImageMemory, mConfig.debugName + "_Memory");
    vk_utils::setDebugObjectName(mDevice, mImageView, mConfig.debugName + "_View");
  }

  NE_LOG("Allocated Image{}: Extent {}x{} | Format: {}", mConfig.debugName.empty() ? "" : std::format(" '{}'", mConfig.debugName),
         mConfig.width, mConfig.height, string_VkFormat(mConfig.format));
}

Image::~Image() { releaseResources(); }

void Image::releaseResources() {
  if (mDevice != VK_NULL_HANDLE) {
    if (mImageView != VK_NULL_HANDLE) {
      vkDestroyImageView(mDevice, mImageView, nullptr);
      mImageView = VK_NULL_HANDLE;
    }
    if (mImage != VK_NULL_HANDLE) {
      vkDestroyImage(mDevice, mImage, nullptr);
      mImage = VK_NULL_HANDLE;
    }
    if (mImageMemory != VK_NULL_HANDLE) {
      vkFreeMemory(mDevice, mImageMemory, nullptr);
      mImageMemory = VK_NULL_HANDLE;
    }
  }
}

} // namespace ne
