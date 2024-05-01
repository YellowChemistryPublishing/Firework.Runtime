#include "HardwareExcept.h"

#include <csignal>

using namespace Firework;
using namespace Firework::Internal;

#if defined(__GLIBC__)
#include <ExceptionHandler/Signal_glibc.h>
#elif __MINGW32__ && !__SEH__
#include <ExceptionHandler/Signal_mingw.h>
#elif defined(_WIN32)
#include <ExceptionHandler/Signal_win32.h>
#else
#include <ExceptionHandler/Signal_dfl.h>
#endif

namespace Firework::Internal
{
    static struct __firework_corelib_api __fw_rt_hw_libexcpt_init final
    {
        __fw_rt_hw_libexcpt_init()
        {
            ::__fw_rt_hw_sighf();
        }
    } init;
}

#if __has_include(<cpptrace/cpptrace.hpp>)
// Should check cpptrace::can_signal_safe_unwind, but it's probably fine without(TM).
// Exception::Exception() // : trace(cpptrace::stacktrace::current()) // bufferLen(cpptrace::safe_generate_raw_trace(buffer, FIREWORK_EXCEPTION_TRACE_DEPTH))
// { }

cpptrace::stacktrace Exception::resolveStacktrace() const
{
    return std::move(this->trace);
    // cpptrace::object_trace ret; ret.frames.reserve(this->bufferLen);
    // for (std::size_t i = 0; i < this->bufferLen; i++)
    // {
    //     cpptrace::safe_object_frame frame;
    //     cpptrace::get_safe_object_frame(this->buffer[i], &frame);
    //     ret.frames.push_back(frame.resolve());
    // }
    // return ret.resolve();
}
#endif