#include <Core/HardwareExcept.h>

#include <csignal>

using namespace Firework;
using namespace Firework::Internal;

_fw_core_api void __fw_rt_hw_sighf()
{
    sigset_t ub;
    sigemptyset(&ub);
    sigaddset(&ub, SIGSEGV);
    sigaddset(&ub, SIGFPE);
    sigprocmask(SIG_UNBLOCK, &ub, nullptr);
    struct sigaction act;
    act.sa_sigaction = __fw_rt_hw_excpt_handler;
    sigaction(SIGSEGV, &act, nullptr);
    sigaction(SIGFPE, &act, nullptr);
}
_fw_core_api void __fw_rt_hw_sigh(int sig)
{
    struct sigaction act;
    act.sa_sigaction = __fw_rt_hw_excpt_handler;
    sigaction(sig, &act, nullptr);
}
_fw_core_api void __fw_rt_hw_excpt_handler(int sig, siginfo_t*, void*)
{
    sigset_t ub;
    sigemptyset(&ub);
    sigaddset(&ub, sig);
    sigprocmask(SIG_UNBLOCK, &ub, nullptr);
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