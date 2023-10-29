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
    static struct __fw_rt_hw_libexcpt_init
    {
        inline __fw_rt_hw_libexcpt_init()
        {
            ::__fw_rt_hw_sighf();
        }
    } init;
}
