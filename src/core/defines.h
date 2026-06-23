#pragma once

#define NE_UNUSED(x) (void)(x)
#define NE_ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#if defined(NE_PLATFORM_WINDOWS)
    #define NE_BREAK() __debugbreak()
#elif (defined(NE_PLATFORM_LINUX) || defined(NE_PLATFORM_APPLE)) && (defined(__GNUC__) || defined(__clang__))
    #define NE_BREAK() __builtin_trap()
#else
  #include <cstdlib>
  #define NE_BREAK() std::abort()
#endif
