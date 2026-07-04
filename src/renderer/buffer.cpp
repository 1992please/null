#include "renderer/buffer.h"
#include "core/core.h"
#include "renderer/utils.h"

#include <cstring>

namespace ne {

Buffer::Buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage,
               VkMemoryPropertyFlags properties)
    : mDevice(device), mPhysicalDevice(physicalDevice), mBufferSize(size) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VK_CHECK(vkCreateBuffer(mDevice, &bufferInfo, nullptr, &mBuffer));

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(mDevice, mBuffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

  VK_CHECK(vkAllocateMemory(mDevice, &allocInfo, nullptr, &mMemory));
  VK_CHECK(vkBindBufferMemory(mDevice, mBuffer, mMemory, 0));
}

Buffer::~Buffer() {
  unmap();
  if (mBuffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(mDevice, mBuffer, nullptr);
  }
  if (mMemory != VK_NULL_HANDLE) {
    vkFreeMemory(mDevice, mMemory, nullptr);
  }
}

void Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
  if (mMapped == nullptr) {
    VK_CHECK(vkMapMemory(mDevice, mMemory, offset, size, 0, &mMapped));
  }
}

void Buffer::unmap() {
  if (mMapped != nullptr) {
    vkUnmapMemory(mDevice, mMemory);
    mMapped = nullptr;
  }
}

void Buffer::writeToBuffer(const void* data, VkDeviceSize size, VkDeviceSize offset) {
  VkDeviceSize writeSize = (size == VK_WHOLE_SIZE) ? mBufferSize - offset : size;
  if (mMapped == nullptr) {
    map(writeSize, offset);
  }
  std::memcpy(static_cast<char*>(mMapped) + offset, data, writeSize);
}

uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  NE_ASSERT(false, "Failed to find suitable memory type!");
  return ~0U;
}

} // namespace ne
