#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#if __has_include(<cpptrace/cpptrace.hpp>)
#include <cpptrace/cpptrace.hpp>
#endif
#if __SEH__ && __MINGW32__
#include <csetjmp>
#endif
#include <csignal>
#include <cstdint>
#include <exception>
#if __MINGW32__
#include <excpt.h>
#endif
#include <source_location>
#if _WIN32
#undef NOMINMAX
#define NOMINMAX 1
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#undef near
#undef far
#endif

#include <Core/Application.h>
#include <Firework/Config.h>

extern _fw_core_api void __fw_rt_hw_sighf();
extern _fw_core_api void __fw_rt_hw_sigh(int sig);
#if defined(__GLIBC__)
_fw_core_api extern void __fw_rt_hw_excpt_handler(int sig, siginfo_t*, void*);
#elif __MINGW32__ && !__SEH__ && INTPTR_MAX == INT32_MAX
extern "C" _fw_core_api EXCEPTION_DISPOSITION __fw_rt_hw_excpt_handler(PEXCEPTION_RECORD record, void*, PCONTEXT, void*);
#elif __MINGW32__ && !__SEH__ && INTPTR_MAX == INT64_MAX
extern "C" _fw_core_api long __fw_rt_hw_excpt_handler(PEXCEPTION_POINTERS ex);
#elif defined(_WIN32)
extern _fw_core_api LONG WINAPI __fw_rt_hw_excpt_handler(LPEXCEPTION_POINTERS ex);
#else
extern _fw_core_api void __fw_rt_hw_excpt_handler(int sig);
#endif

#if __MINGW32__ && !__SEH__
#define __hwTry                       \
    __try1(__fw_rt_hw_excpt_handler); \
    try
#define __hwCatch(...) catch (__VA_ARGS__)
#define __hwEnd() __except1
#elif __MINGW32__ && !__clang__ && __SEH__
#define __hwTry [&]() __attribute__((noinline)) -> void { try
#define __hwCatch(...) catch (__VA_ARGS__)
#define __hwEnd() \
    }             \
    ()
#elif _WIN32 && __clang__
#define __hwTry \
    __try       \
    {           \
        [&] { try
#define __hwCatch(...) catch (__VA_ARGS__)
#define __hwEnd()                                                  \
    }                                                              \
    ();                                                            \
    }                                                              \
    __except (__fw_rt_hw_excpt_handler(GetExceptionInformation())) \
    { }
#else
#define __hwTry try
#define __hwCatch(...) catch (__VA_ARGS__)
#define __hwEnd()
#endif

/*
Here's the situation with HardwareExcept.
mingw32 (dw2) - Supported. Unwinding works, and dtors are called.
mingw64 (seh) - Partial. dtors are ignored, no unwinding.
clang64 (seh) - Partial. dtors are ignored, no unwinding.
MSVC (seh) - Not supported. Has regressed, issue with exception filter across dll-boundary. ~~With /EHa throwing from filter is fine.~~
*/

namespace Firework::Internal
{
    class CoreEngine;
}

namespace Firework
{
    class Exception : public std::exception
    {
#if __has_include(<cpptrace/cpptrace.hpp>)
        cpptrace::safe_object_frame frame;
        cpptrace::frame_ptr buffer[FIREWORK_EXCEPTION_TRACE_DEPTH];
        size_t bufferLen;
        cpptrace::stacktrace trace;

        cpptrace::stacktrace resolveStacktrace() const;
#endif
    public:
        Exception(); // = default;
        inline virtual ~Exception() noexcept = 0;

        friend class Firework::Internal::CoreEngine;
    };
    Exception::~Exception() = default;

    class SegmentationFaultException : public Exception
    {
    public:
        SegmentationFaultException() = default;
        ~SegmentationFaultException() noexcept override = default;

        const char* what() const noexcept override
        {
            return "Segmentation fault.";
        }
    };
    class ArithmeticException : public Exception
    {
    public:
        ArithmeticException() = default;
        ~ArithmeticException() noexcept override = default;

        const char* what() const noexcept override
        {
            return "Arithmetic exception.";
        }
    };
} // namespace Firework
