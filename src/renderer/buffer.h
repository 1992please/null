#pragma once

#include <string>
#include <volk/volk.h>

namespace ne {

class Buffer {
public:
  static constexpr VkDeviceSize DEFAULT_ALIGNMENT = 16;

  struct Config {
    VkDeviceSize size = 0;
    VkBufferUsageFlags usage = 0;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    std::string debugName = "";
    VkDeviceSize alignment = DEFAULT_ALIGNMENT;
  };

  Buffer(VkDevice iDevice, VkPhysicalDevice iPhysicalDevice, const Config& iConfig);
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
  bool canUpload(VkDeviceSize iSize) const { return mUploadOffset + iSize <= mConfig.size; }
  void resetUploadOffset() { mUploadOffset = 0; }
  VkDeviceSize getUploadOffset() const { return mUploadOffset; }

  VkDeviceAddress getDeviceAddress(VkDeviceSize iOffset = 0) const {
    return mDeviceAddress != 0 ? (mDeviceAddress + iOffset) : 0;
  }

  const Config& getConfig() const { return mConfig; }
  VkBuffer getBuffer() const { return mBuffer; }

  bool isHostVisible() const { return (mMemoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0; }
  bool isHostCoherent() const { return (mMemoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0; }
  VkMemoryPropertyFlags getMemoryProperties() const { return mMemoryProperties; }

private:
  static uint32_t findBufferMemoryType(VkPhysicalDevice iPhysicalDevice, uint32_t iTypeFilter, VkMemoryPropertyFlags iProperties,
                                       VkBufferUsageFlags iUsage);

  VkDevice mDevice = VK_NULL_HANDLE;

  Config mConfig;
  VkBuffer mBuffer = VK_NULL_HANDLE;
  VkDeviceMemory mMemory = VK_NULL_HANDLE;
  VkDeviceAddress mDeviceAddress = 0;
  VkMemoryPropertyFlags mMemoryProperties = 0;
  void* mMapped = nullptr;
  VkDeviceSize mUploadOffset = 0; // Added for linear allocation
};

} // namespace ne
