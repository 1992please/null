#pragma once

#include "core/defines.h"
#include "core/logger.h"

#include <volk/volk.h>
#include <vulkan/vk_enum_string_helper.h>

#include <cstdlib>

#define VK_CHECK(fn)                                                                                                   \
  do {                                                                                                                 \
    VkResult vkCheckResult = (fn);                                                                                            \
    if (vkCheckResult != VK_SUCCESS) {                                                                                        \
      ne::Logger::get().log(ne::Logger::LogType_Fatal,                                                                 \
                            "VULKAN ERROR\n"                                                                           \
                            "  Function: {}\n"                                                                         \
                            "  Result: {} ({})\n"                                                                      \
                            "  File: {}\n"                                                                             \
                            "  Line: {}",                                                                              \
                            #fn, string_VkResult(vkCheckResult), static_cast<int>(vkCheckResult), __FILE__, __LINE__);               \
      NE_BREAK();                                                                                                      \
      std::abort();                                                                                                    \
    }                                                                                                                  \
  } while (false)

namespace ne {

namespace config {
constexpr VkDeviceSize VERTEX_POOL_SIZE = 64 * 1024 * 1024;          // 64 MB
constexpr VkDeviceSize INDEX_POOL_SIZE = 32 * 1024 * 1024;           // 32 MB
constexpr VkDeviceSize DEFAULT_UPLOAD_BUFFER_SIZE = 16 * 1024 * 1024; // 16 MB
constexpr VkDeviceSize DEFAULT_STAGING_BUFFER_SIZE = 4 * 1024 * 1024; // 4 MB
} // namespace config

template <typename T>
constexpr T alignUp(T value, T alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

} // namespace ne
