#pragma once

#include <Core/Application.h>
#include <Core/Debug.h>
#include <Core/HardwareExcept.h>

inline int __fw_rt_handleInitializeAndExit(int argc, char* argv[])
{
    if (::Firework::Application::run(argc, argv) != 0)
    return EXIT_FAILURE;

    #if _WIN32
    system("pause");
    #else
    system("bash -c \"read -n1 -r -p \"Press\\\\\\ any\\\\\\ key\\\\\\ to\\\\\\ continue\\\\\\ .\\\\\\ .\\\\\\ .\"\"");
    system("bash -c \"echo -e \'\\b \'\"");
    #endif

    return EXIT_SUCCESS;
}

#define main(...) \
__main(__VA_ARGS__); \
 \
int main(int argc, char* argv[]) \
{ \
    [[maybe_unused]] int _ = ::__main(__VA_OPT__(argc) __VA_OPT__(,) __VA_OPT__(argv)); \
    return ::__fw_rt_handleInitializeAndExit(__VA_OPT__(argc) __VA_OPT__(,) __VA_OPT__(argv)); \
} \
 \
int __main(__VA_ARGS__)
