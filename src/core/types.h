#pragma once

#define NE_UNUSED(x) (void)(x)
#define NE_ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#if defined(_WIN32) || defined(_WIN64)
    #define NE_PLATFORM_WINDOWS 1
#elif defined(__linux__)
    #define NE_PLATFORM_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
    #define NE_PLATFORM_APPLE 1
#else
    #define NE_PLATFORM_UNKNOWN 1
#endif

#ifdef _WIN32
    #define NE_BREAK() __debugbreak()
#elif defined(__linux__) && defined(__GNUC__)
    #define NE_BREAK() __builtin_trap()
#else
    #define NE_BREAK()
#endif
