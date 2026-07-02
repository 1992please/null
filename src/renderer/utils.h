#pragma once

#include "core/defines.h"
#include "core/logger.h"

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

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
