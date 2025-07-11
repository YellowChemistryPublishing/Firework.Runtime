#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <module/sys>
#include <string>
#include <utility>
#include <vector>

#include <filesystem>
namespace fs = std::filesystem;

#include <Packager.h>
using namespace Firework::PackageSystem;

// Thanks random website. Very cool. https://hbfs.wordpress.com/2017/01/10/strings-in-buffer-switchcase-statements/.
constexpr uint64_t strmix(wchar_t m, uint64_t s)
{
    return ((s << 7) + ~(s >> 3)) + ~m;
}
constexpr uint64_t strhash(const wchar_t* m)
{
    return (*m) ? strmix(*m, strhash(m + 1)) : 0;
}
constexpr uint64_t strmix(char m, uint64_t s)
{
    return ((s << 7) + ~(s >> 3)) + ~m;
}
constexpr uint64_t strhash(const char* m)
{
    return (*m) ? strmix(*m, strhash(m + 1)) : 0;
}

int main(int argc, char* argv[])
{
    fs::path inPath;
    fs::path outPath;
    bool extractInstead = false;

    std::error_code ec;
    ssz i = 1;
    while (true)
    {
        switch (strhash(argv[+i]))
        {
        case strhash("--help"):
        case strhash("-h"):
            std::cout << "[Firework.Packager.CLI] [INFO]  Firework.Packager Help Display\n"
                         "                                any: --help, -h                           -> Display this help text.\n"
                         "                                any: --output, -o      [value]            -> Specify the output file archive.\n"
                         "                                any: --unpack, -u, -ex                    -> Flag to specify to extract the target file instead.\n"
                         "                                                       [firstArg/lastArg] -> Input path.\n";
            return 0;
        case strhash("-o"):
        case strhash("--output"):
            if (++i == argc) [[unlikely]]
            {
                std::cerr << "[Firework.Packager.CLI] [FATAL] Expected following argument to " << argv[+(i - 1_z)] << ", found end of commandline arguments.\n";
                return 1;
            }

            outPath = argv[+i];
            break;
        case strhash("-ex"):
        case strhash("-u"):
        case strhash("--unpack"): extractInstead = true; break;
        default:
            if (i != 1 && i != argc - 1) [[unlikely]]
            {
                std::cerr << "[Firework.Packager.CLI] [FATAL] Expected commandline argument parameter, found unexpected string.\n";
                return 1;
            }
            else if (!fs::exists(inPath = argv[+i], ec))
            {
                std::cerr << "[Firework.Packager.CLI] [FATAL] Input path to package doesn't exist!\n";
                return 1;
            }
        }
        if (++i == argc)
            break;
    }

    if (inPath.empty())
    {
        std::cerr << "[Firework.Packager.CLI] [FATAL] Input path hasn't been specified.\n";
        return 1;
    }
    if (outPath.empty())
    {
        std::cerr << "[Firework.Packager.CLI] [FATAL] Output path hasn't been specified.\n";
        return 1;
    }

    if (extractInstead)
    {
        std::cerr << "[Firework.Packager.CLI] [FATAL] Unpacking is currently unimplemented!\n";
        return 1;
    }
    else
    {
        if (!fs::is_directory(inPath))
        {
            std::cerr << "[Firework.Packager.CLI] [FATAL] Path for input folder isn't a folder!\n";
            return 1;
        }

        Packager::packageFolder(inPath, outPath.c_str());
    }
}