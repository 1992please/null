#pragma once

#include <string>
#include <volk/volk.h>

namespace ne {

class Renderer;

class Buffer {
public:
  static constexpr VkDeviceSize DEFAULT_ALIGNMENT = 16;

  Buffer(Renderer* iRenderer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
         std::string iDebugName = "", VkDeviceSize iAlignment = DEFAULT_ALIGNMENT);
  ~Buffer();

  // Non-copyable and non-moveable (pinned Vulkan RAII resource)
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;
  Buffer(Buffer&&) = delete;
  Buffer& operator=(Buffer&&) = delete;

  void mapMemory(VkDeviceSize iSize = VK_WHOLE_SIZE, VkDeviceSize iOffset = 0);
  void writeToBuffer(const void* iData, VkDeviceSize iSize = VK_WHOLE_SIZE, VkDeviceSize iOffset = 0);
  void unmapMemory();

  VkDeviceSize suballocate(VkDeviceSize iSize);
  VkDeviceSize upload(const void* iData, VkDeviceSize iSize);
  bool canUpload(VkDeviceSize iSize) const { return mUploadOffset + iSize <= mBufferSize; }
  void resetUploadOffset() { mUploadOffset = 0; }
  VkDeviceSize getUploadOffset() const { return mUploadOffset; }

  VkDeviceAddress getDeviceAddress(VkDeviceSize iOffset = 0) const {
    return mDeviceAddress != 0 ? (mDeviceAddress + iOffset) : 0;
  }

  VkBuffer getBuffer() const { return mBuffer; }
  VkDeviceSize getBufferSize() const { return mBufferSize; }
  const std::string& getDebugName() const { return mDebugName; }

  bool isHostVisible() const { return (mMemoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0; }
  bool isHostCoherent() const { return (mMemoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0; }
  VkMemoryPropertyFlags getMemoryProperties() const { return mMemoryProperties; }

private:
  uint32_t findBufferMemoryType(Renderer* iRenderer, uint32_t iTypeFilter, VkMemoryPropertyFlags iProperties,
                                VkBufferUsageFlags iUsage);

  VkDevice mDevice = VK_NULL_HANDLE;

  VkBuffer mBuffer = VK_NULL_HANDLE;
  VkDeviceMemory mMemory = VK_NULL_HANDLE;
  VkDeviceAddress mDeviceAddress = 0;
  VkBufferUsageFlags mUsage = 0;
  VkMemoryPropertyFlags mMemoryProperties = 0;
  void* mMapped = nullptr;
  VkDeviceSize mBufferSize = 0;
  VkDeviceSize mUploadOffset = 0; // Added for linear allocation
  VkDeviceSize mAlignment = DEFAULT_ALIGNMENT;
  std::string mDebugName;
};

} // namespace ne
