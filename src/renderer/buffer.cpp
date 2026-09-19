#include "renderer/buffer.h"
#include "core/assert.h"
#include "core/logger.h"
#include "renderer/utils.h"

// std
#include <cstring>
#include <string>
#include <vector>

namespace ne {

namespace {

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

Buffer::Buffer(VkDevice iDevice, VkPhysicalDevice iPhysicalDevice, const Config& iConfig) : mDevice(iDevice), mConfig(iConfig) {
  NE_ASSERT(mConfig.size > 0, "Buffer size must be greater than 0");
  NE_ASSERT(mDevice != VK_NULL_HANDLE, "Device must not be null");
  NE_ASSERT(iPhysicalDevice != VK_NULL_HANDLE, "Physical device must not be null");

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = mConfig.size;
  bufferInfo.usage = mConfig.usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VK_CHECK(vkCreateBuffer(mDevice, &bufferInfo, nullptr, &mBuffer));

  VkBufferMemoryRequirementsInfo2 memReqsInfo2{};
  memReqsInfo2.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
  memReqsInfo2.buffer = mBuffer;

  VkMemoryRequirements2 memReqs2{};
  memReqs2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
  vkGetBufferMemoryRequirements2(mDevice, &memReqsInfo2, &memReqs2);

  VkMemoryAllocateFlagsInfo allocateFlagsInfo{};
  allocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
  allocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

  VkMemoryAllocateInfo memoryAllocateInfo{};
  memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memoryAllocateInfo.allocationSize = memReqs2.memoryRequirements.size;
  memoryAllocateInfo.memoryTypeIndex =
      findBufferMemoryType(iPhysicalDevice, memReqs2.memoryRequirements.memoryTypeBits, mConfig.properties, mConfig.usage);
  memoryAllocateInfo.pNext = (mConfig.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) ? &allocateFlagsInfo : nullptr;

  VK_CHECK(vkAllocateMemory(mDevice, &memoryAllocateInfo, nullptr, &mMemory));

  VkBindBufferMemoryInfo bindInfo{};
  bindInfo.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
  bindInfo.buffer = mBuffer;
  bindInfo.memory = mMemory;
  bindInfo.memoryOffset = 0;
  VK_CHECK(vkBindBufferMemory2(mDevice, 1, &bindInfo));

  if (mConfig.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
    VkBufferDeviceAddressInfo addressInfo{};
    addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    addressInfo.buffer = mBuffer;
    mDeviceAddress = vkGetBufferDeviceAddress(mDevice, &addressInfo);
  }

  if (!mConfig.debugName.empty()) {
    vk_utils::setDebugObjectName(mDevice, mBuffer, mConfig.debugName);
    vk_utils::setDebugObjectName(mDevice, mMemory, mConfig.debugName + "_Memory");
  }

  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(iPhysicalDevice, &memProperties);
  mMemoryProperties = memProperties.memoryTypes[memoryAllocateInfo.memoryTypeIndex].propertyFlags;

  NE_LOG("Allocated Buffer{}: Size: {} (Allocated: {}) | Usage: [{}] | Memory Type: [Index: {}, Properties: {}]",
         mConfig.debugName.empty() ? "" : std::format(" '{}'", mConfig.debugName), vk_utils::formatBytes(mConfig.size),
         vk_utils::formatBytes(memoryAllocateInfo.allocationSize), bufferUsageToString(mConfig.usage),
         memoryAllocateInfo.memoryTypeIndex, memoryPropertiesToString(mMemoryProperties));
}

Buffer::~Buffer() {
  if (mMapped) {
    unmapMemory();
  }
  NE_LOG("Destroyed Buffer{}: Size: {} | Usage: [{}]", mConfig.debugName.empty() ? "" : std::format(" '{}'", mConfig.debugName),
         vk_utils::formatBytes(mConfig.size), bufferUsageToString(mConfig.usage));
  vkDestroyBuffer(mDevice, mBuffer, nullptr);
  vkFreeMemory(mDevice, mMemory, nullptr);
}

void Buffer::mapMemory(VkDeviceSize iSize, VkDeviceSize iOffset) {
  NE_ASSERT(isHostVisible(), "Cannot map a buffer that is not host-visible!");
  NE_ASSERT(!mMapped, "Buffer is already mapped!");
  VK_CHECK(vkMapMemory(mDevice, mMemory, iOffset, iSize, 0, &mMapped));
}

void Buffer::writeToBuffer(const void* iData, VkDeviceSize iSize, VkDeviceSize iOffset) {
  NE_ASSERT(mMapped, "Buffer must be mapped before writing!");
  NE_ASSERT(isHostCoherent(), "Buffer::writeToBuffer requires HOST_COHERENT memory!");
  VkDeviceSize writeSize = (iSize == VK_WHOLE_SIZE) ? mConfig.size - iOffset : iSize;
  NE_ASSERT(iOffset + writeSize <= mConfig.size, "Buffer write exceeds buffer size!");
  std::memcpy(static_cast<char*>(mMapped) + iOffset, iData, writeSize);
}

void Buffer::unmapMemory() {
  NE_ASSERT(mMapped, "Buffer is not mapped!");
  vkUnmapMemory(mDevice, mMemory);
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

uint32_t Buffer::findBufferMemoryType(VkPhysicalDevice iPhysicalDevice, uint32_t iTypeFilter, VkMemoryPropertyFlags iProperties,
                                      VkBufferUsageFlags iUsage) {
  uint32_t memoryTypeIndex = vk_utils::findMemoryType(iPhysicalDevice, iTypeFilter, iProperties);
  // If host-visible and coherent memory is requested, try to find a heap that is ALSO device-local (Resizable BAR)
  // Pure staging buffers (usage = TRANSFER_SRC_BIT only) should NOT be allocated in Resizable BAR VRAM.
  if ((iUsage != VK_BUFFER_USAGE_TRANSFER_SRC_BIT) && (iProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
      (iProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
    uint32_t barMemoryTypeIndex =
        vk_utils::findMemoryType(iPhysicalDevice, iTypeFilter, iProperties | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (barMemoryTypeIndex != ~0U) {
      memoryTypeIndex = barMemoryTypeIndex;
    }
  }
  NE_ASSERT(memoryTypeIndex != ~0U, "Failed to find suitable memory type!");
  return memoryTypeIndex;
}

} // namespace ne
