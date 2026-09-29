#include "renderer/buffer.h"
#include "core/assert.h"
#include "core/logger.h"
#include "renderer/device.h"
#include "renderer/memory_allocator.h"
#include "renderer/utils.h"

#include <vma/vk_mem_alloc.h>

// std
#include <cstring>
#include <string>
#include <vector>

namespace ne {

namespace {

[[maybe_unused]] std::string_view storageToString(Buffer::Storage storage) {
  switch (storage) {
    case Buffer::Storage::DeviceLocal:
      return "DeviceLocal";
    case Buffer::Storage::Upload:
      return "Upload";
    case Buffer::Storage::Readback:
      return "Readback";
  }
  return "Unknown";
}

[[maybe_unused]] std::string bufferUsageToString(VkBufferUsageFlags usage) {
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

[[maybe_unused]] std::string memoryPropertiesToString(VkMemoryPropertyFlags properties) {
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

} // namespace

Buffer::Buffer(Device* iDevice, const Config& iConfig) : mDevice(iDevice), mConfig(iConfig) {
  NE_ASSERT(mConfig.size > 0, "Buffer size must be greater than 0");

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = mConfig.size;
  bufferInfo.usage = mConfig.usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocInfo{};
  if (mConfig.storage == Storage::DeviceLocal) {
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
  } else if (mConfig.storage == Storage::Upload) {
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
  } else if (mConfig.storage == Storage::Readback) {
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
  }

  VmaAllocationInfo allocationInfo{};
  VK_CHECK(vmaCreateBuffer(mDevice->getMemoryAllocator()->getHandle(), &bufferInfo, &allocInfo, &mBuffer, &mAllocation,
                           &allocationInfo));

  if (mConfig.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
    VkBufferDeviceAddressInfo addressInfo{};
    addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    addressInfo.buffer = mBuffer;
    mDeviceAddress = vkGetBufferDeviceAddress(mDevice->getDevice(), &addressInfo);
  }

  if (!mConfig.debugName.empty()) {
    vk_utils::setDebugObjectName(mDevice->getDevice(), mBuffer, mConfig.debugName);
    vmaSetAllocationName(mDevice->getMemoryAllocator()->getHandle(), mAllocation, mConfig.debugName.c_str());
  }

  VkMemoryPropertyFlags effectiveMemProps = 0;
  vmaGetAllocationMemoryProperties(mDevice->getMemoryAllocator()->getHandle(), mAllocation, &effectiveMemProps);

  NE_LOG("Allocated Buffer{}: Size: {} (Allocated: {}) | Usage: [{}] | Storage: [{}] | Type: [{}]",
         mConfig.debugName.empty() ? "" : " '" + mConfig.debugName + "'", vk_utils::formatBytes(mConfig.size),
         vk_utils::formatBytes(allocationInfo.size), bufferUsageToString(mConfig.usage), storageToString(mConfig.storage),
         memoryPropertiesToString(effectiveMemProps));
}

Buffer::~Buffer() {
  if (mMapped) {
    unmapMemory();
  }
  NE_LOG("Destroyed Buffer{}: Size: {} | Usage: [{}]", mConfig.debugName.empty() ? "" : " '" + mConfig.debugName + "'",
         vk_utils::formatBytes(mConfig.size), bufferUsageToString(mConfig.usage));
  vmaDestroyBuffer(mDevice->getMemoryAllocator()->getHandle(), mBuffer, mAllocation);
}

void Buffer::mapMemory(VkDeviceSize /*iSize*/, VkDeviceSize /*iOffset*/) {
  NE_ASSERT(mConfig.storage != Storage::DeviceLocal, "Cannot map a buffer with DeviceLocal storage!");
  NE_ASSERT(!mMapped, "Buffer is already mapped!");
  VK_CHECK(vmaMapMemory(mDevice->getMemoryAllocator()->getHandle(), mAllocation, &mMapped));
}

void Buffer::writeToBuffer(const void* iData, VkDeviceSize iSize, VkDeviceSize iOffset) {
  NE_ASSERT(mMapped, "Buffer must be mapped before writing!");
  VkDeviceSize writeSize = (iSize == VK_WHOLE_SIZE) ? mConfig.size - iOffset : iSize;
  NE_ASSERT(iOffset + writeSize <= mConfig.size, "Buffer write exceeds buffer size!");
  std::memcpy(static_cast<char*>(mMapped) + iOffset, iData, writeSize);
  vmaFlushAllocation(mDevice->getMemoryAllocator()->getHandle(), mAllocation, iOffset, writeSize);
}

void Buffer::unmapMemory() {
  NE_ASSERT(mMapped, "Buffer is not mapped!");
  vmaUnmapMemory(mDevice->getMemoryAllocator()->getHandle(), mAllocation);
  mMapped = nullptr;
}

VkDeviceSize Buffer::suballocate(VkDeviceSize iSize) {
  NE_ASSERT(canUpload(iSize), "Buffer overflow! Increase buffer size.");

  VkDeviceSize allocatedOffset = mUploadOffset;
  mUploadOffset = vk_utils::alignUp(allocatedOffset + iSize, mConfig.alignment);
  return allocatedOffset;
}

VkDeviceSize Buffer::upload(const void* iData, VkDeviceSize iSize) {
  NE_ASSERT(mMapped, "Buffer must be mapped before uploading!");
  VkDeviceSize allocatedOffset = suballocate(iSize);
  writeToBuffer(iData, iSize, allocatedOffset);
  return allocatedOffset;
}

} // namespace ne
