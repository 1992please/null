#pragma once

#include "core/defines.h"
#include "core/logger.h"

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include <cstdlib>

#define VK_CHECK(fn)                                                                                                   \
  do {                                                                                                                 \
    VkResult result = (fn);                                                                                            \
    if (result != VK_SUCCESS) {                                                                                        \
      ne::Logger::get().log(ne::Logger::LogType_Fatal,                                                                 \
                            "VULKAN ERROR\n"                                                                           \
                            "  Function: {}\n"                                                                         \
                            "  Result: {} ({})\n"                                                                      \
                            "  File: {}\n"                                                                             \
                            "  Line: {}",                                                                              \
                            #fn, string_VkResult(result), static_cast<int>(result), __FILE__, __LINE__);               \
      NE_BREAK();                                                                                                      \
      std::abort();                                                                                                    \
    }                                                                                                                  \
  } while (false)
