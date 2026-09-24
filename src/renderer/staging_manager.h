#pragma once

#include "core/defines.h"
#include "renderer/utils.h"

#include <memory>
#include <string>
#include <vector>
#include <volk/volk.h>

namespace ne {

class Device;
class Buffer;
class Image;

/**
 * @class StagingManager
 * @brief Manages a persistent host-visible staging memory arena and orchestrates
 * batched or immediate transfers to GPU buffers and images with modern Vulkan 1.4 Synchronization2.
 */
class StagingManager {
public:
  explicit StagingManager(Device* iDevice);
  ~StagingManager();

  StagingManager(const StagingManager&) = delete;
  StagingManager& operator=(const StagingManager&) = delete;
  StagingManager(StagingManager&&) = delete;
  StagingManager& operator=(StagingManager&&) = delete;

  // Batched workflow
  void beginBatch();
  void stageBufferCopy(VkBuffer dstBuffer, const void* data, VkDeviceSize size, VkDeviceSize dstOffset = 0);
  void stageImageUpload(Image& dstImage, const void* pixelData, VkDeviceSize size);
  void endBatch();

  bool hasPendingUploads() const { return !mPendingBufferCopies.empty() || !mPendingImageUploads.empty(); }
  bool isBatching() const { return mIsBatching; }

private:
  struct PendingBufferCopy {
    VkBuffer dstBuffer = VK_NULL_HANDLE;
    VkDeviceSize srcOffset = 0;
    VkDeviceSize dstOffset = 0;
    VkDeviceSize size = 0;
  };

  struct PendingImageUpload {
    Image* image = nullptr;
    VkDeviceSize srcOffset = 0;
    uint32_t width = 0;
    uint32_t height = 0;
  };

  VkDeviceSize stageData(const void* data, VkDeviceSize size);
  void flushBatch();

  void recordBufferCopy(VkCommandBuffer cmd, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size,
                        VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);
  void recordImageCopy(VkCommandBuffer cmd, VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height,
                       VkDeviceSize srcOffset = 0, uint32_t mipLevel = 0);

  Device* mDevice = nullptr;
  std::unique_ptr<Buffer> mStagingBuffer;

  bool mIsBatching = false;
  std::vector<PendingBufferCopy> mPendingBufferCopies;
  std::vector<PendingImageUpload> mPendingImageUploads;
};

} // namespace ne
