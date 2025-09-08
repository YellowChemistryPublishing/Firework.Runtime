#pragma once

_push_nowarn_clang(_clWarn_clang_zero_as_nullptr);
#include <SDL3/SDL_main.h>
_pop_nowarn_clang();

#undef main
#ifdef SDL_main_impl_h_
#define __entryPoint SDL_main
#else
#define __entryPoint main
#endif

#include <Core/Application.h>
#include <Core/Debug.h>
#include <Core/HardwareExcept.h>

inline int _fw_rt_handleInitializeAndExit(int argc, char* argv[])
{
    if (Firework::Application::run(argc, argv) != EXIT_SUCCESS)
        return EXIT_FAILURE;

#if 0 // We don't do this anymore, because we've stopped adding a console by default, but it's here if we ever need it.
#if _WIN32
    std::system("pause");
#else
    std::system("bash -c \"read -n1 -r -p \"Press\\\\\\ any\\\\\\ key\\\\\\ to\\\\\\ continue\\\\\\ .\\\\\\ .\\\\\\ .\"\"");
    std::system("bash -c \"echo -e \'\\b \'\"");
#endif
#endif

    return EXIT_SUCCESS;
}

static_assert((static_cast<unsigned char>("💩"[0]) == 0xF0) && (static_cast<unsigned char>("💩"[1]) == 0x9F) && (static_cast<unsigned char>("💩"[2]) == 0x92) &&
                  (static_cast<unsigned char>("💩"[3]) == 0xA9),
              "Source or compiler not UTF-8 compliant! Well this is bad... File bug report.");

namespace Firework::Internal
{
    extern _fw_core_api int _fw_rt_fwd_main_invoc(int argc, char* argv[], SDL_main_func mainFunction, void* reserved);
}
inline int SDL_RunApp(int argc, char* argv[], SDL_main_func mainFunction, void* reserved)
{
    return Firework::Internal::_fw_rt_fwd_main_invoc(argc, argv, mainFunction, reserved);
}

#define main(...)                                                             \
    __main(__VA_ARGS__);                                                      \
                                                                              \
    int __entryPoint(int argc, char* argv[])                                  \
    {                                                                         \
        int ret = ::__main(__VA_OPT__(argc) __VA_OPT__(, ) __VA_OPT__(argv)); \
        return !ret ? ::_fw_rt_handleInitializeAndExit(argc, argv) : ret;     \
    }                                                                         \
                                                                              \
    int __main(__VA_ARGS__)
