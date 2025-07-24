#include <Core/HardwareExcept.h>

#include <csignal>

using namespace Firework;
using namespace Firework::Internal;

_fw_core_api void __fw_rt_hw_sighf()
{
    signal(SIGSEGV, __fw_rt_hw_excpt_handler);
    signal(SIGFPE, __fw_rt_hw_excpt_handler);
}
_fw_core_api void __fw_rt_hw_sigh(int sig)
{
    signal(sig, __fw_rt_hw_excpt_handler);
}
// This will almost never work properly.
_fw_core_api void __fw_rt_hw_excpt_handler(int sig)
{
    switch (sig)
    {
    case SIGSEGV:
        __fw_rt_hw_sigh(sig);
        throw SegmentationFaultException();
    case SIGFPE:
        __fw_rt_hw_sigh(sig);
        throw ArithmeticException();
    }
}
