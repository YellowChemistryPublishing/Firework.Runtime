#include <Core/HardwareExcept.h>

using namespace Firework;
using namespace Firework::Internal;

_fw_core_api void __fw_rt_hw_sighf()
{
    SetUnhandledExceptionFilter(__fw_rt_hw_excpt_handler);
}
_fw_core_api void __fw_rt_hw_sigh(int)
{ }
// This also hardly ever works outside of MSVC.
_fw_core_api LONG WINAPI __fw_rt_hw_excpt_handler(LPEXCEPTION_POINTERS ex)
{
    switch (ex->ExceptionRecord->ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_STACK_OVERFLOW:
    case EXCEPTION_STACK_INVALID:
        throw SegmentationFaultException();
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_OVERFLOW:
    case EXCEPTION_FLT_UNDERFLOW:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_INVALID_OPERATION:
        throw ArithmeticException();
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
