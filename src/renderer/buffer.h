#pragma once

#include <string>
#include <volk/volk.h>

namespace ne {

class Renderer;

class Buffer {
public:
  Buffer(Renderer* iRenderer, VkDeviceSize size, VkBufferUsageFlags usage,
         VkMemoryPropertyFlags properties, std::string iDebugName = "");
  ~Buffer();

  // Prevent copying to avoid double-freeing Vulkan handles
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  void mapMemory(VkDeviceSize iSize = VK_WHOLE_SIZE, VkDeviceSize iOffset = 0);
  void writeToBuffer(const void* iData, VkDeviceSize iSize = VK_WHOLE_SIZE, VkDeviceSize iOffset = 0);
  void unmapMemory();


  VkDeviceAddress upload(const void* iData, VkDeviceSize iSize, VkDeviceSize iAlignment = 16);
  void resetUploadOffset() { mUploadOffset = 0; }

  VkDeviceAddress getDeviceAddress() const;

  VkBuffer getBuffer() const { return mBuffer; }
  VkDeviceSize getBufferSize() const { return mBufferSize; }
  const std::string& getDebugName() const { return mDebugName; }

private:
  uint32_t findBufferMemoryType(Renderer* iRenderer, uint32_t iTypeFilter, VkMemoryPropertyFlags iProperties,
                              VkBufferUsageFlags iUsage);

  VkDevice mDevice = VK_NULL_HANDLE;

  VkBuffer mBuffer = VK_NULL_HANDLE;
  VkDeviceMemory mMemory = VK_NULL_HANDLE;
  VkBufferUsageFlags mUsage = 0;
  void* mMapped = nullptr;
  VkDeviceSize mBufferSize = 0;
  VkDeviceSize mUploadOffset = 0; // Added for linear allocation
  std::string mDebugName;
};

} // namespace ne
