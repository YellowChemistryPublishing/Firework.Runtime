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

extern __firework_corelib_api void __fw_rt_hw_sighf();
extern __firework_corelib_api void __fw_rt_hw_sigh(int sig);
#if defined(__GLIBC__)
__firework_corelib_api extern void __fw_rt_hw_excpt_handler(int sig, siginfo_t*, void*);
#elif __MINGW32__ && !__SEH__ && INTPTR_MAX == INT32_MAX
extern "C" __firework_corelib_api EXCEPTION_DISPOSITION __fw_rt_hw_excpt_handler(PEXCEPTION_RECORD record, void*, PCONTEXT, void*);
#elif __MINGW32__ && !__SEH__ && INTPTR_MAX == INT64_MAX
extern "C" __firework_corelib_api long __fw_rt_hw_excpt_handler(PEXCEPTION_POINTERS ex);
#elif defined(_WIN32)
extern __firework_corelib_api LONG WINAPI __fw_rt_hw_excpt_handler(LPEXCEPTION_POINTERS ex);
#else
extern __firework_corelib_api void __fw_rt_hw_excpt_handler(int sig);
#endif

#if __MINGW32__ && !__SEH__
#define __hwTry __try1(__fw_rt_hw_excpt_handler);
#define __hwExcept() __except1
#elif _WIN32 && __clang__
#define __hwTry __try
#define __hwExcept() __except(__fw_rt_hw_excpt_handler(GetExceptionInformation())) { }
#else
#define __hwTry
#define __hwExcept()
#endif

/*
Here's the situation with HardwareExcept.
mingw32 (dw2) - Supported. Unwinding works, and dtors are called.
mingw64 (seh) - Partial. Sometimes it works, sometimes it doesn't. dtors are ignored.
                TODO: WOOOOOOOO I FOUND SOMETHING longjmp uses RtlUnwind in the mingw crt, if I'm correct, this unwinds the stack unlike regular longjmp!
                unimplemented
clang64 (seh) - Partial. Hardware exceptions are caught, dtors are ignored.
MSVC (seh) - Supported. With /EHa throwing from filter is fine.
This was a mistake. Just let it regress so I can huck it into a dumpster fire where it belongs.
*/

namespace Firework
{
    namespace Internal
    {
        class CoreEngine;
    }

    class Exception : public std::exception
    {
    public:
        #if __has_include(<cpptrace/cpptrace.hpp>)
        cpptrace::stacktrace trace = cpptrace::stacktrace::current();
        #endif
    public:
        inline Exception() = default;
        inline virtual ~Exception() noexcept = 0;

        friend class Firework::Internal::CoreEngine;
    };
    Exception::~Exception() noexcept = default;
    class SegmentationFaultException : public Exception
    {
    public:
        inline SegmentationFaultException() = default;
        inline ~SegmentationFaultException() noexcept override = default;

        inline const char* what() const noexcept override
        {
            return "Segmentation fault.";
        }
    };
    class ArithmeticException : public Exception
    {
    public:
        inline ArithmeticException() = default;
        inline ~ArithmeticException() noexcept override = default;

        inline const char* what() const noexcept override
        {
            return "Arithmetic exception.";
        }
    };
}
