#include <Core/HardwareExcept.h>

#include <eh.h>
#if __has_include(<cpptrace/cpptrace.hpp>)
#include <cpptrace/cpptrace.hpp>
#endif

using namespace Firework;
using namespace Firework::Internal;

_fw_core_api void __fw_rt_hw_sighf()
{ }
_fw_core_api void __fw_rt_hw_sigh(int)
{ }
#if INTPTR_MAX == INT32_MAX
extern "C" EXCEPTION_DISPOSITION __fw_rt_hw_excpt_handler(PEXCEPTION_RECORD record, void*, PCONTEXT, void*)
{
#elif INTPTR_MAX == INT64_MAX
extern "C" long __fw_rt_hw_excpt_handler(PEXCEPTION_POINTERS ex)
{
    PEXCEPTION_RECORD record = ex->ExceptionRecord;
#else
#error "what"
#endif
    switch (record->ExceptionCode)
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