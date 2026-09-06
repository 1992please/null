#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <volk/volk.h>

namespace ne {

class Renderer;

/**
 * @class Texture
 * @brief RAII management of a 2D Vulkan Image, Device Memory, and Image View.
 *
 * Implements modern Vulkan 1.4 Synchronization2 layout transitions and staging buffer
 * pixel uploads via vkCmdCopyBufferToImage2.
 */
class Texture {
public:
  struct Config {
    uint32_t width = 0;
    uint32_t height = 0;
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    uint32_t mipLevels = 1;
    std::string debugName = "";
  };

  Texture(Renderer* iRenderer, const Config& iConfig, const void* iPixelData = nullptr, VkDeviceSize iSize = 0);
  ~Texture();

  // Static 1x1 fallback textures
  static std::unique_ptr<Texture> createWhite1x1(Renderer* iRenderer, std::string iDebugName = "Texture_White1x1");
  static std::unique_ptr<Texture> createFlatNormal1x1(Renderer* iRenderer, std::string iDebugName = "Texture_FlatNormal1x1");
  static std::unique_ptr<Texture> createBlack1x1(Renderer* iRenderer, std::string iDebugName = "Texture_Black1x1");

  // Prevent copying
  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  // Move semantics
  Texture(Texture&& other);
  Texture& operator=(Texture&& other);

  void uploadData(const void* iPixelData, VkDeviceSize iSize);

  void transitionLayout(VkCommandBuffer iCommandBuffer, VkImageLayout iNewLayout, VkAccessFlags2 iDstAccessMask,
                        VkPipelineStageFlags2 iDstStageMask);
  void transitionLayout(VkImageLayout iNewLayout, VkAccessFlags2 iDstAccessMask, VkPipelineStageFlags2 iDstStageMask);

  // Getters
  VkImage getImage() const { return mImage; }
  VkImageView getImageView() const { return mImageView; }
  VkDeviceMemory getMemory() const { return mImageMemory; }
  VkFormat getFormat() const { return mConfig.format; }
  uint32_t getWidth() const { return mConfig.width; }
  uint32_t getHeight() const { return mConfig.height; }
  uint32_t getMipLevels() const { return mConfig.mipLevels; }
  VkImageUsageFlags getUsage() const { return mConfig.usage; }
  VkImageLayout getCurrentLayout() const { return mCurrentLayout; }
  const std::string& getDebugName() const { return mConfig.debugName; }
  const Config& getConfig() const { return mConfig; }

private:
  void initResources();
  void releaseResources();

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
