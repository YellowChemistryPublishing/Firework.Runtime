#pragma once

#include <Core/Application.h>
#include <Core/Debug.h>
#include <Core/HardwareExcept.h>

inline int __fw_rt_handleInitializeAndExit(int argc, char* argv[])
{
    if (::Firework::Application::run(argc, argv) != 0)
    return EXIT_FAILURE;

    #if _WIN32
    ::std::system("pause");
    #else
    system("bash -c \"read -n1 -r -p \"Press\\\\\\ any\\\\\\ key\\\\\\ to\\\\\\ continue\\\\\\ .\\\\\\ .\\\\\\ .\"\"");
    system("bash -c \"echo -e \'\\b \'\"");
    #endif

    return EXIT_SUCCESS;
}

static_assert
(
    (static_cast<unsigned char>("💩"[0]) == 0xF0) &&
    (static_cast<unsigned char>("💩"[1]) == 0x9F) &&
    (static_cast<unsigned char>("💩"[2]) == 0x92) &&
    (static_cast<unsigned char>("💩"[3]) == 0xA9),
    "Source or compiler not UTF-8 compliant! Well this is bad... File bug report."
);

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
