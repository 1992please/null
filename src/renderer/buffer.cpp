#include "renderer/buffer.h"
#include "core/assert.h"
#include "renderer/renderer.h"
#include "renderer/utils.h"

// std
#include <cstring>

namespace ne {

Buffer::Buffer(Renderer* iRenderer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    : mDevice(iRenderer->getDevice()), mPhysicalDevice(iRenderer->getPhysicalDevice()), mBufferSize(size) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VK_CHECK(vkCreateBuffer(mDevice, &bufferInfo, nullptr, &mBuffer));

  VkMemoryRequirements memoryRequirements;
  vkGetBufferMemoryRequirements(mDevice, mBuffer, &memoryRequirements);

  VkMemoryAllocateFlagsInfo allocateFlagsInfo{};
  allocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
  allocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

  VkMemoryAllocateInfo memoryAllocateInfo{};
  memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memoryAllocateInfo.allocationSize = memoryRequirements.size;
  memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);
  memoryAllocateInfo.pNext = (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) ? &allocateFlagsInfo : nullptr;

  VK_CHECK(vkAllocateMemory(mDevice, &memoryAllocateInfo, nullptr, &mMemory));
  VK_CHECK(vkBindBufferMemory(mDevice, mBuffer, mMemory, 0));
}

Buffer::~Buffer() {
  if (mMapped) {
    unmapMemory();
  }
  if (mBuffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(mDevice, mBuffer, nullptr);
  }
  if (mMemory != VK_NULL_HANDLE) {
    vkFreeMemory(mDevice, mMemory, nullptr);
  }
}

void Buffer::mapMemory(VkDeviceSize size, VkDeviceSize offset) {
  NE_ASSERT(!mMapped);
  VK_CHECK(vkMapMemory(mDevice, mMemory, offset, size, 0, &mMapped));
}

void Buffer::writeToBuffer(const void* data, VkDeviceSize size, VkDeviceSize offset) {
  NE_ASSERT(mMapped);
  VkDeviceSize writeSize = (size == VK_WHOLE_SIZE) ? mBufferSize - offset : size;
  std::memcpy(static_cast<char*>(mMapped) + offset, data, writeSize);
}

void Buffer::unmapMemory() {
  NE_ASSERT(mMapped);
  vkUnmapMemory(mDevice, mMemory);
  mMapped = nullptr;
}

VkDeviceAddress Buffer::getDeviceAddress() const {
  VkBufferDeviceAddressInfo addressInfo{};
  addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
  addressInfo.buffer = mBuffer;
  return vkGetBufferDeviceAddress(mDevice, &addressInfo);
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

VkDeviceAddress Buffer::upload(const void* data, VkDeviceSize size, VkDeviceSize alignment) {
  NE_ASSERT(mMapped, "Buffer must be mapped before uploading!");
  VkDeviceSize alignedOffset = alignUp(mUploadOffset, alignment);
  NE_ASSERT(alignedOffset + size <= mBufferSize, "Buffer overflow! Increase buffer size.");

  mUploadOffset = alignedOffset;
  std::memcpy(static_cast<char*>(mMapped) + mUploadOffset, data, size);

  VkDeviceAddress address = getDeviceAddress() + mUploadOffset;
  mUploadOffset += size;

  return address;
}

} // namespace ne
