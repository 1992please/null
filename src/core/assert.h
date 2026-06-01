#pragma once

#include "core/types.h"
#include "logger.h"
#include <cstdlib>

#if !NE_BUILD_SHIPPING
#define NE_ASSERT(condition, ...)                                              \
  do {                                                                         \
    if (!(condition)) {                                                        \
      std::string user_msg;                                                    \
      __VA_OPT__(user_msg = std::format(__VA_ARGS__);)                         \
      ne::Logger::get().log(                                                   \
          ne::Logger::LogType_Fatal,                                           \
          "ASSERTION FAILED: {}\n  Condition: {}\n  File: {}\n  Line: {}",     \
          user_msg, #condition, __FILE__, __LINE__);                           \
      NE_BREAK();                                                              \
      std::abort();                                                            \
    }                                                                          \
  } while (false)

#else
#define NE_ASSERT(condition, ...)                                              \
  do {                                                                         \
    (void)(condition); /* Eat the unused variable warning */                   \
  } while (false)
#endif
