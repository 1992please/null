#pragma once

#include <volk/volk.h>

namespace ne {

class Renderer;

class Buffer {
public:
  Buffer(Renderer* iRenderer, VkDeviceSize size, VkBufferUsageFlags usage,
         VkMemoryPropertyFlags properties);
  ~Buffer();

  // Prevent copying to avoid double-freeing Vulkan handles
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  void mapMemory(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  void writeToBuffer(const void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  void unmapMemory();

  VkBuffer getBuffer() const { return mBuffer; }
  VkDeviceSize getBufferSize() const { return mBufferSize; }

private:
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

  VkDevice mDevice = VK_NULL_HANDLE;
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;

  VkBuffer mBuffer = VK_NULL_HANDLE;
  VkDeviceMemory mMemory = VK_NULL_HANDLE;
  void* mMapped = nullptr;
  VkDeviceSize mBufferSize = 0;
};

} // namespace ne
