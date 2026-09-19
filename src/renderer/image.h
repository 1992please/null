#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <volk/volk.h>

namespace ne {

struct ImageData;

/**
 * @class Image
 * @brief RAII management of a 2D Vulkan Image, Device Memory, and Image View.
 *
 * Serves as the Vulkan RHI wrapper for sampled images, render targets, depth attachments,
 * and storage images. Pinned GPU resource (non-copyable, non-moveable).
 */
class Image {
public:
  struct Config {
    uint32_t width = 0;
    uint32_t height = 0;
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    uint32_t mipLevels = 1;
    VkImageAspectFlags aspectMask = 0; // 0 = auto-deduce from format
    std::string debugName = "";
  };

  Image(VkDevice iDevice, VkPhysicalDevice iPhysicalDevice, const Config& iConfig);
  ~Image();

  // Non-copyable and non-moveable (pinned Vulkan RAII resource)
  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;
  Image(Image&&) = delete;
  Image& operator=(Image&&) = delete;

  // Synchronization State
  VkImageLayout getCurrentLayout() const { return mCurrentLayout; }
  VkAccessFlags2 getCurrentAccessMask() const { return mCurrentAccessMask; }
  VkPipelineStageFlags2 getCurrentStageMask() const { return mCurrentStageMask; }

  void setLayoutState(VkImageLayout iLayout, VkAccessFlags2 iAccessMask, VkPipelineStageFlags2 iStageMask) {
    mCurrentLayout = iLayout;
    mCurrentAccessMask = iAccessMask;
    mCurrentStageMask = iStageMask;
  }

  // Static helpers
  static VkImageAspectFlags deduceAspectFlags(VkFormat format);

  // Getters
  bool isValid() const { return mImage != VK_NULL_HANDLE; }
  VkImage getImage() const { return mImage; }
  VkImageView getImageView() const { return mImageView; }
  VkDeviceMemory getMemory() const { return mImageMemory; }
  const Config& getConfig() const { return mConfig; }

private:
  void releaseResources();

  VkDevice mDevice = VK_NULL_HANDLE;
  Config mConfig;

  VkImage mImage = VK_NULL_HANDLE;
  VkDeviceMemory mImageMemory = VK_NULL_HANDLE;
  VkImageView mImageView = VK_NULL_HANDLE;

  VkImageLayout mCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkAccessFlags2 mCurrentAccessMask = VK_ACCESS_2_NONE;
  VkPipelineStageFlags2 mCurrentStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
};

} // namespace ne
