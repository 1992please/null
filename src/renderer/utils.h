#pragma once

#include "core/assert.h"
#include "core/defines.h"
#include "core/logger.h"

#include <volk/volk.h>
#ifndef NE_BUILD_SHIPPING
#include <vulkan/vk_enum_string_helper.h>
#endif

#include <cstdlib>
#include <string>
#include <type_traits>

#ifndef NE_BUILD_SHIPPING

// Debug & Development: Verbose error logging with file, line, and string enum conversion
#define VK_CHECK(fn)                                                                                                             \
  do {                                                                                                                           \
    VkResult vkCheckResult = (fn);                                                                                               \
    if (vkCheckResult != VK_SUCCESS) {                                                                                           \
      ne::Logger::get().log(ne::Logger::LogType_Fatal,                                                                           \
                            "VULKAN ERROR\n"                                                                                     \
                            "  Function: {}\n"                                                                                   \
                            "  Result: {} ({})\n"                                                                                \
                            "  File: {}\n"                                                                                       \
                            "  Line: {}",                                                                                        \
                            #fn, string_VkResult(vkCheckResult), static_cast<int>(vkCheckResult), __FILE__, __LINE__);           \
      NE_BREAK();                                                                                                                \
      std::abort();                                                                                                              \
    }                                                                                                                            \
  } while (false)

#else

// Shipping: Execute the Vulkan call without string formatting or logging overhead
#define VK_CHECK(fn)                                                                                                             \
  do {                                                                                                                           \
    VkResult vkCheckResult = (fn);                                                                                               \
    if (vkCheckResult != VK_SUCCESS) {                                                                                           \
      std::abort();                                                                                                              \
    }                                                                                                                            \
  } while (false)

#endif

namespace ne::vk_utils {

constexpr VkDeviceSize VERTEX_POOL_SIZE = 64 * 1024 * 1024;    // 64 MB
constexpr VkDeviceSize INDEX_POOL_SIZE = 32 * 1024 * 1024;     // 32 MB
constexpr VkDeviceSize UPLOAD_BUFFER_SIZE = 16 * 1024 * 1024;  // 16 MB
constexpr VkDeviceSize STAGING_BUFFER_SIZE = 64 * 1024 * 1024; // 64 MB
constexpr uint32_t MAX_SAMPLED_IMAGES = 4096;
constexpr const char* DEFAULT_SHADER = "base_shader";

inline void setDebugUtilsObjectName(VkDevice device, VkObjectType objectType, uint64_t objectHandle, const char* name) {
#ifndef NE_BUILD_SHIPPING
  if (vkSetDebugUtilsObjectNameEXT && device != VK_NULL_HANDLE && objectHandle != 0 && name != nullptr) {
    VkDebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType = objectType;
    nameInfo.objectHandle = objectHandle;
    nameInfo.pObjectName = name;
    vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
  }
#else
  NE_UNUSED(device);
  NE_UNUSED(objectType);
  NE_UNUSED(objectHandle);
  NE_UNUSED(name);
#endif
}

inline void setDebugUtilsObjectName(VkDevice device, VkObjectType objectType, uint64_t objectHandle, const std::string& name) {
  setDebugUtilsObjectName(device, objectType, objectHandle, name.c_str());
}

template <typename T>
inline void setDebugObjectName(VkDevice device, T handle, const char* name) {
  VkObjectType type = VK_OBJECT_TYPE_UNKNOWN;
  if constexpr (std::is_same_v<T, VkBuffer>)
    type = VK_OBJECT_TYPE_BUFFER;
  else if constexpr (std::is_same_v<T, VkDeviceMemory>)
    type = VK_OBJECT_TYPE_DEVICE_MEMORY;
  else if constexpr (std::is_same_v<T, VkImage>)
    type = VK_OBJECT_TYPE_IMAGE;
  else if constexpr (std::is_same_v<T, VkImageView>)
    type = VK_OBJECT_TYPE_IMAGE_VIEW;
  else if constexpr (std::is_same_v<T, VkPipeline>)
    type = VK_OBJECT_TYPE_PIPELINE;
  else if constexpr (std::is_same_v<T, VkPipelineLayout>)
    type = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
  else if constexpr (std::is_same_v<T, VkSampler>)
    type = VK_OBJECT_TYPE_SAMPLER;
  else if constexpr (std::is_same_v<T, VkCommandPool>)
    type = VK_OBJECT_TYPE_COMMAND_POOL;
  else if constexpr (std::is_same_v<T, VkCommandBuffer>)
    type = VK_OBJECT_TYPE_COMMAND_BUFFER;
  else if constexpr (std::is_same_v<T, VkQueue>)
    type = VK_OBJECT_TYPE_QUEUE;
  else if constexpr (std::is_same_v<T, VkSemaphore>)
    type = VK_OBJECT_TYPE_SEMAPHORE;
  else if constexpr (std::is_same_v<T, VkFence>)
    type = VK_OBJECT_TYPE_FENCE;
  else if constexpr (std::is_same_v<T, VkShaderModule>)
    type = VK_OBJECT_TYPE_SHADER_MODULE;
  else if constexpr (std::is_same_v<T, VkSwapchainKHR>)
    type = VK_OBJECT_TYPE_SWAPCHAIN_KHR;
  else if constexpr (std::is_same_v<T, VkDescriptorSetLayout>)
    type = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
  else if constexpr (std::is_same_v<T, VkDescriptorPool>)
    type = VK_OBJECT_TYPE_DESCRIPTOR_POOL;
  else if constexpr (std::is_same_v<T, VkDescriptorSet>)
    type = VK_OBJECT_TYPE_DESCRIPTOR_SET;
  else {
    static_assert(!sizeof(T), "Unsupported Vulkan object type passed to setDebugObjectName!");
  }

  setDebugUtilsObjectName(device, type, static_cast<uint64_t>(reinterpret_cast<uintptr_t>(handle)), name);
}

template <typename T>
inline void setDebugObjectName(VkDevice device, T handle, const std::string& name) {
  setDebugObjectName(device, handle, name.c_str());
}

template <typename T>
constexpr T alignUp(T value, T alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

inline std::string formatBytes(VkDeviceSize bytes) {
  constexpr double KB = 1024.0;
  constexpr double MB = 1024.0 * 1024.0;
  constexpr double GB = 1024.0 * 1024.0 * 1024.0;

  if (bytes >= GB) {
    return std::format("{:.2f} GB", bytes / GB);
  } else if (bytes >= MB) {
    return std::format("{:.2f} MB", bytes / MB);
  } else if (bytes >= KB) {
    return std::format("{:.2f} KB", bytes / KB);
  } else {
    return std::format("{} B", bytes);
  }
}

inline uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  NE_ASSERT(false, "Failed to find suitable memory type!");
  return ~0U;
}

inline void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask,
                                  VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags2 srcAccessMask,
                                  VkAccessFlags2 dstAccessMask, VkPipelineStageFlags2 srcStageMask,
                                  VkPipelineStageFlags2 dstStageMask) {
  VkImageMemoryBarrier2 imageMemoryBarrier{};
  imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  imageMemoryBarrier.srcStageMask = srcStageMask;
  imageMemoryBarrier.srcAccessMask = srcAccessMask;
  imageMemoryBarrier.dstStageMask = dstStageMask;
  imageMemoryBarrier.dstAccessMask = dstAccessMask;
  imageMemoryBarrier.oldLayout = oldLayout;
  imageMemoryBarrier.newLayout = newLayout;
  imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.image = image;
  imageMemoryBarrier.subresourceRange = {
      .aspectMask = aspectMask, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};

  VkDependencyInfo dependencyInfo{};
  dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  dependencyInfo.dependencyFlags = 0;
  dependencyInfo.imageMemoryBarrierCount = 1;
  dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;

  vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}

} // namespace ne::vk_utils
