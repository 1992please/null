#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <volk/volk.h>

namespace ne {

class Renderer;
struct ImageData;

/**
 * @class Image
 * @brief RAII management of a 2D Vulkan Image, Device Memory, and Image View.
 *
 * Serves as the Vulkan RHI wrapper for sampled images, render targets, depth attachments,
 * and storage images with modern Vulkan 1.4 Synchronization2 layout transitions.
 */
class Image {
public:
  struct Config {
    uint32_t width = 0;
    uint32_t height = 0;
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    uint32_t mipLevels = 1;
    std::string debugName = "";
  };

  Image(Renderer* iRenderer, const Config& iConfig, const void* iPixelData = nullptr, VkDeviceSize iSize = 0);
  Image(Renderer* iRenderer, const ImageData& iImageData, bool iSrgb = true, std::string iDebugName = "");
  ~Image();

  // Prevent copying
  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;

  // Move semantics
  Image(Image&& other);
  Image& operator=(Image&& other);

  void transitionLayout(VkCommandBuffer iCommandBuffer, VkImageLayout iNewLayout, VkAccessFlags2 iDstAccessMask,
                        VkPipelineStageFlags2 iDstStageMask);
  void transitionLayout(VkImageLayout iNewLayout, VkAccessFlags2 iDstAccessMask, VkPipelineStageFlags2 iDstStageMask);

  // Getters
  bool isValid() const { return mImage != VK_NULL_HANDLE; }
  VkImage getImage() const { return mImage; }
  VkImageView getImageView() const { return mImageView; }
  VkDeviceMemory getMemory() const { return mImageMemory; }
  VkImageLayout getCurrentLayout() const { return mCurrentLayout; }
  const Config& getConfig() const { return mConfig; }

private:
  void releaseResources();
  void uploadData(const void* iPixelData, VkDeviceSize iSize);

  Renderer* mRenderer = nullptr;
  Config mConfig;

  VkImage mImage = VK_NULL_HANDLE;
  VkDeviceMemory mImageMemory = VK_NULL_HANDLE;
  VkImageView mImageView = VK_NULL_HANDLE;

  VkImageLayout mCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkAccessFlags2 mCurrentAccessMask = VK_ACCESS_2_NONE;
  VkPipelineStageFlags2 mCurrentStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
};

} // namespace ne
