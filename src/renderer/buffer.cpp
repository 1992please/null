#include "renderer/buffer.h"
#include "core/assert.h"
#include "core/logger.h"
#include "renderer/renderer.h"
#include "renderer/utils.h"

// std
#include <cstring>
#include <string>
#include <vector>

namespace ne {

namespace {

#if !NE_BUILD_SHIPPING
std::string bufferUsageToString(VkBufferUsageFlags usage) {
  std::vector<std::string> flags;
  if (usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) flags.push_back("TRANSFER_SRC");
  if (usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) flags.push_back("TRANSFER_DST");
  if (usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) flags.push_back("UNIFORM_TEXEL");
  if (usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) flags.push_back("STORAGE_TEXEL");
  if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) flags.push_back("UNIFORM");
  if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) flags.push_back("STORAGE");
  if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) flags.push_back("INDEX");
  if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) flags.push_back("VERTEX");
  if (usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) flags.push_back("INDIRECT");
  if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) flags.push_back("SHADER_DEVICE_ADDRESS");
  if (flags.empty()) return "NONE";
  std::string result;
  for (size_t i = 0; i < flags.size(); ++i) {
    if (i > 0) result += " | ";
    result += flags[i];
  }
  return result;
}

std::string memoryPropertiesToString(VkMemoryPropertyFlags properties) {
  std::vector<std::string> flags;
  if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) flags.push_back("DEVICE_LOCAL");
  if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) flags.push_back("HOST_VISIBLE");
  if (properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) flags.push_back("HOST_COHERENT");
  if (properties & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) flags.push_back("HOST_CACHED");
  if (properties & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) flags.push_back("LAZILY_ALLOCATED");
  if (properties & VK_MEMORY_PROPERTY_PROTECTED_BIT) flags.push_back("PROTECTED");
  if (flags.empty()) return "NONE";
  std::string result;
  for (size_t i = 0; i < flags.size(); ++i) {
    if (i > 0) result += " | ";
    result += flags[i];
  }
  return result;
}
#endif // !NE_BUILD_SHIPPING

} // namespace

Buffer::Buffer(Renderer* iRenderer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    : mDevice(iRenderer->getDevice()), mPhysicalDevice(iRenderer->getPhysicalDevice()), mUsage(usage), mBufferSize(size) {
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
  memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties, usage);
  memoryAllocateInfo.pNext = (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) ? &allocateFlagsInfo : nullptr;

  VK_CHECK(vkAllocateMemory(mDevice, &memoryAllocateInfo, nullptr, &mMemory));
  VK_CHECK(vkBindBufferMemory(mDevice, mBuffer, mMemory, 0));

#if !NE_BUILD_SHIPPING
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);
  VkMemoryPropertyFlags allocatedProperties = memProperties.memoryTypes[memoryAllocateInfo.memoryTypeIndex].propertyFlags;

  NE_LOG("Allocated Buffer: Size: {} (Allocated: {}) | Usage: [{}] | Memory Type: [Index: {}, Properties: {}]",
         formatBytes(mBufferSize),
         formatBytes(memoryAllocateInfo.allocationSize),
         bufferUsageToString(usage),
         memoryAllocateInfo.memoryTypeIndex,
         memoryPropertiesToString(allocatedProperties));
#endif
}

Buffer::~Buffer() {
  if (mMapped) {
    unmapMemory();
  }
  if (mBuffer != VK_NULL_HANDLE) {
#if !NE_BUILD_SHIPPING
    NE_LOG("Destroyed Buffer: Size: {} | Usage: [{}]", formatBytes(mBufferSize), bufferUsageToString(mUsage));
#endif
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

uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

  // If host-visible and coherent memory is requested, try to find a heap that is ALSO device-local (Resizable BAR)
  // Pure staging buffers (usage = TRANSFER_SRC_BIT only) should NOT be allocated in Resizable BAR VRAM.
  if ((usage != VK_BUFFER_USAGE_TRANSFER_SRC_BIT) && (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
    VkMemoryPropertyFlags preferredProperties = properties | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & preferredProperties) == preferredProperties) {
        return i;
      }
    }
  }

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
