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

#ifndef NE_BUILD_SHIPPING
std::string bufferUsageToString(VkBufferUsageFlags usage) {
  std::vector<std::string> flags;
  if (usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
    flags.push_back("TRANSFER_SRC");
  if (usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT)
    flags.push_back("TRANSFER_DST");
  if (usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)
    flags.push_back("UNIFORM_TEXEL");
  if (usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
    flags.push_back("STORAGE_TEXEL");
  if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
    flags.push_back("UNIFORM");
  if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
    flags.push_back("STORAGE");
  if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
    flags.push_back("INDEX");
  if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    flags.push_back("VERTEX");
  if (usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
    flags.push_back("INDIRECT");
  if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    flags.push_back("SHADER_DEVICE_ADDRESS");
  if (flags.empty())
    return "NONE";
  std::string result;
  for (size_t i = 0; i < flags.size(); ++i) {
    if (i > 0)
      result += " | ";
    result += flags[i];
  }
  return result;
}

std::string memoryPropertiesToString(VkMemoryPropertyFlags properties) {
  std::vector<std::string> flags;
  if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    flags.push_back("DEVICE_LOCAL");
  if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    flags.push_back("HOST_VISIBLE");
  if (properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    flags.push_back("HOST_COHERENT");
  if (properties & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
    flags.push_back("HOST_CACHED");
  if (properties & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
    flags.push_back("LAZILY_ALLOCATED");
  if (properties & VK_MEMORY_PROPERTY_PROTECTED_BIT)
    flags.push_back("PROTECTED");
  if (flags.empty())
    return "NONE";
  std::string result;
  for (size_t i = 0; i < flags.size(); ++i) {
    if (i > 0)
      result += " | ";
    result += flags[i];
  }
  return result;
}
#endif // !NE_BUILD_SHIPPING

} // namespace

Buffer::Buffer(Renderer* iRenderer, VkDeviceSize iSize, VkBufferUsageFlags iUsage, VkMemoryPropertyFlags iProperties)
    : mDevice(iRenderer->getDevice()), mUsage(iUsage), mBufferSize(iSize) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = iSize;
  bufferInfo.usage = iUsage;
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
  memoryAllocateInfo.memoryTypeIndex = findBufferMemoryType(iRenderer, memoryRequirements.memoryTypeBits, iProperties, iUsage);
  memoryAllocateInfo.pNext = (iUsage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) ? &allocateFlagsInfo : nullptr;

  VK_CHECK(vkAllocateMemory(mDevice, &memoryAllocateInfo, nullptr, &mMemory));
  VK_CHECK(vkBindBufferMemory(mDevice, mBuffer, mMemory, 0));

#ifndef NE_BUILD_SHIPPING
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(iRenderer->getPhysicalDevice(), &memProperties);
  VkMemoryPropertyFlags allocatedProperties = memProperties.memoryTypes[memoryAllocateInfo.memoryTypeIndex].propertyFlags;

  NE_LOG("Allocated Buffer: Size: {} (Allocated: {}) | Usage: [{}] | Memory Type: [Index: {}, Properties: {}]",
         vk_utils::formatBytes(mBufferSize), vk_utils::formatBytes(memoryAllocateInfo.allocationSize),
         bufferUsageToString(iUsage), memoryAllocateInfo.memoryTypeIndex, memoryPropertiesToString(allocatedProperties));
#endif
}

Buffer::~Buffer() {
  if (mMapped) {
    unmapMemory();
  }
  if (mBuffer != VK_NULL_HANDLE) {
#ifndef NE_BUILD_SHIPPING
    NE_LOG("Destroyed Buffer: Size: {} | Usage: [{}]", vk_utils::formatBytes(mBufferSize), bufferUsageToString(mUsage));
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

void Buffer::writeToBuffer(const void* iData, VkDeviceSize iSize, VkDeviceSize iOffset) {
  NE_ASSERT(mMapped);
  VkDeviceSize writeSize = (iSize == VK_WHOLE_SIZE) ? mBufferSize - iOffset : iSize;
  std::memcpy(static_cast<char*>(mMapped) + iOffset, iData, writeSize);
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

VkDeviceAddress Buffer::upload(const void* iData, VkDeviceSize iSize, VkDeviceSize iAlignment) {
  NE_ASSERT(mMapped, "Buffer must be mapped before uploading!");
  VkDeviceSize alignedOffset = vk_utils::alignUp(mUploadOffset, iAlignment);
  NE_ASSERT(alignedOffset + iSize <= mBufferSize, "Buffer overflow! Increase buffer size.");

  mUploadOffset = alignedOffset;
  std::memcpy(static_cast<char*>(mMapped) + mUploadOffset, iData, iSize);

  VkDeviceAddress address = getDeviceAddress() + mUploadOffset;
  mUploadOffset += iSize;

  return address;
}

uint32_t Buffer::findBufferMemoryType(Renderer* iRenderer, uint32_t iTypeFilter, VkMemoryPropertyFlags iProperties,
                                      VkBufferUsageFlags iUsage) {
  uint32_t memoryTypeIndex = iRenderer->findMemoryType(iTypeFilter, iProperties);
  // If host-visible and coherent memory is requested, try to find a heap that is ALSO device-local (Resizable BAR)
  // Pure staging buffers (usage = TRANSFER_SRC_BIT only) should NOT be allocated in Resizable BAR VRAM.
  if ((iUsage != VK_BUFFER_USAGE_TRANSFER_SRC_BIT) && (iProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
      (iProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
    uint32_t barMemoryTypeIndex =
        iRenderer->findMemoryType(iTypeFilter, iProperties | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (barMemoryTypeIndex != ~0U) {
      memoryTypeIndex = barMemoryTypeIndex;
    }
  }
  NE_ASSERT(memoryTypeIndex != ~0U, "Failed to find suitable memory type!");
  return memoryTypeIndex;
}

} // namespace ne
