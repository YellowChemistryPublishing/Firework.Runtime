#pragma once

#define GL_QUEUE_OVERBURDENED_THRESHOLD 8
#define Config_EntitiesPerChunk 4096

#define FIREWORK_BUILD_ARCH_UNKNOWN 0
#define FIREWORK_BUILD_ARCH_X86_64 1
#define FIREWORK_BUILD_ARCH_X86 2

#define FIREWORK_BUILD_PLATFORM_UNKNOWN 0
#define FIREWORK_BUILD_PLATFORM_WEB 1
#define FIREWORK_BUILD_PLATFORM_WINDOWS 2
#define FIREWORK_BUILD_PLATFORM_MACOS 3
#define FIREWORK_BUIDL_PLATFORM_LINUX 4

#if __EMSCRIPTEN__
#define FIREWORK_BUILD_PLATFORM FIREWORK_BUILD_PLATFORM_WEB
#elif _WIN32
#define FIREWORK_BUILD_PLATFORM FIREWORK_BUILD_PLATFORM_WINDOWS
#elif __APPLE__
#define FIREWORK_BUILD_PLATFORM FIREWORK_BUILD_PLATFORM_MACOS
#elif __linux__
#define FIREWORK_BUILD_PLATFORM FIREWORK_BUILD_PLATFORM_LINUX
#else
#define FIREWORK_BUILD_PLATFORM FIREWORK_BUILD_PLATFORM_UNKNOWN
#endif

/// @see https://sourceforge.net/p/predef/wiki/Architectures/
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64)
#define FIREWORK_BUILD_ARCH FIREWORK_BUILD_ARCH_X86_64
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(__IA32__) || defined(_M_IX86) || defined(__X86__) || defined(_X86_) || defined(__THW_INTEL__) || defined(__I86__) || defined(__INTEL__) || defined(__386)
#define FIREWORK_BUILD_ARCH FIREWORK_BUILD_ARCH_X86
#else
#define FIREWORK_BUILD_ARCH FIREWORK_BUILD_ARCH_UNKNOWN
#endif