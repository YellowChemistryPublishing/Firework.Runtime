#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
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

    if (argc <= 1)
    {
        std::cerr << "[Firework.Packager | Firework.Tools] [ERROR] Not enough arguments!\n";
        return EXIT_FAILURE;
    }

    std::error_code ec;
    ssz i = 1;
    while (true)
    {
        switch (strhash(argv[+i]))
        {
        case strhash("--help"):
        case strhash("-h"):
            std::cout << "[Firework.Packager | Firework.Tools] [INFO]  Firework.Packager Help Display\n"
                         "                                any: --help, -h                           -> Display this help text.\n"
                         "                                any: --output, -o      [value]            -> Specify the output file archive.\n"
                         "                                any: --unpack, -u, -ex                    -> Flag to specify to extract the target file instead.\n"
                         "                                                       [firstArg/lastArg] -> Input path.\n";
            return 0;
        case strhash("-o"):
        case strhash("--output"):
            if (++i == argc) [[unlikely]]
            {
                std::cerr << "[Firework.Packager | Firework.Tools] [FATAL] Expected following argument to " << argv[+(i - 1_z)] << ", found end of commandline arguments.\n";
                return EXIT_FAILURE;
            }

            outPath = argv[+i];
            break;
        case strhash("-ex"):
        case strhash("-u"):
        case strhash("--unpack"): extractInstead = true; break;
        default:
            if (i != 1 && i != argc - 1) [[unlikely]]
            {
                std::cerr << "[Firework.Packager | Firework.Tools] [FATAL] Expected commandline argument parameter, found unexpected string.\n";
                return EXIT_FAILURE;
            }
            else if (!fs::exists(inPath = argv[+i], ec))
            {
                std::cerr << "[Firework.Packager | Firework.Tools] [FATAL] Input path to package doesn't exist!\n";
                return EXIT_FAILURE;
            }
        }
        if (++i == argc)
            break;
    }

    if (inPath.empty())
    {
        std::cerr << "[Firework.Packager | Firework.Tools] [FATAL] Input path hasn't been specified.\n";
        return EXIT_FAILURE;
    }
    if (outPath.empty())
    {
        std::cerr << "[Firework.Packager | Firework.Tools] [FATAL] Output path hasn't been specified.\n";
        return EXIT_FAILURE;
    }

    if (extractInstead)
    {
        std::cerr << "[Firework.Packager | Firework.Tools] [FATAL] Unpacking is currently unimplemented!\n";
        return EXIT_FAILURE;
    }
    else
    {
        if (!fs::is_directory(inPath))
        {
            std::cerr << "[Firework.Packager | Firework.Tools] [FATAL] Path for input folder isn't a folder!\n";
            return EXIT_FAILURE;
        }

        _fence_value_return(EXIT_FAILURE, !Packager::packageFolder(inPath, outPath));

        return EXIT_SUCCESS;
    }
}