#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
#include <string>

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
    {
        uint16_t word = 0x0001;
        if (((uint8_t*)&word)[0])
            Packager::endianness = Packager::Endianness::Little;
        else Packager::endianness = Packager::Endianness::Big;
    }

    fs::path inPath;
    fs::path outPath;
    bool extractInstead = false;

    int i = 1;
    while (true)
    {
        switch (strhash(argv[i]))
        {
        case strhash("--help"):
        case strhash("-h"):
            std::cout <<
            "[Firework.Packager.CLI] [INFO]  Firework.Packager Help Display\n"
            "                                any: --help, -h                           -> Display this help text.\n"
            "                                any: --output, -o      [value]            -> Specify the output file archive.\n"
            "                                any: --unpack, -u, -ex                    -> Flag to specify to extract the target file instead.\n"
            "                                                       [firstArg/lastArg] -> Input path.\n";
            return 0;
        case strhash("-o"):
        case strhash("--output"):
            if (++i == argc) [[unlikely]]
            {
                std::cerr << "[Firework.Packager.CLI] [FATAL] Expected following argument to " << argv[i - 1] << ", found end of commandline arguments.\n";
                return 1;
            }
            outPath = argv[i];
            break;
        case strhash("-ex"):
        case strhash("-u"):
        case strhash("--unpack"):
            extractInstead = true;
            break;
        default:
            if (i == 1 || i == argc - 1)
            {
                if (!fs::exists(inPath = argv[i])) [[unlikely]]
                {
                    std::cerr << "[Firework.Packager.CLI] [FATAL] Input path to package doesn't exist!\n";
                    return 1;
                }
            }
            else
            {
                std::cerr << "[Firework.Packager.CLI] [FATAL] Expected commandline argument parameter, found unexpected string.\n";
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
        throw "unimplemented";
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

    return 0;

    // Old code.
    std::unreachable();

    if (argc > 1)
        fputws(L"Warning: Firework.Packager.CLI is not invoked with commandline arguments!", stderr);

    fputws(L"Welcome to packager! Type \"help\" to get help.\n", stdout);
    wprintf(L"Current path is %ls%ls", fs::current_path().wstring().c_str(), L".\n");

    std::wstring input;   
    while (true)
    {
        std::getline(std::wcin, input);
        std::vector<std::wstring> args { L"" };
        bool quoteReached = false;
        for (auto it = input.begin(); it != input.end(); ++it)
        {
            if (*it == L'\'' || *it == L'"')
                quoteReached = !quoteReached;
            if (*it == L' ' && !quoteReached)
            {
                args.push_back(L"");
                continue;
            }
            args.back().push_back(*it);
        }
        args.erase(std::remove_if(args.begin(), args.end(), [](std::wstring& str) { return str.empty(); }), args.end());

        if (args.size() > 0)
        {
            switch (strhash(args[0].c_str()))
            {
            case strhash(L"exit"):
                {
                    fputws(L"Alrighty! Exiting...\n", stdout);
                    goto Exit;
                }
                break;
            case strhash(L"help"):
                {
                    fputws
                    (
LR"(Commands:
    pack [folder] - Will pack [folder] into a formatted file for the Firework Runtime Environment, and place it in the current directory.
    unpack [package] - Will unpack a package into in this directory, in a folder called "Package_Unpacked".
    exit - Quits this utility.
)",
                        stdout
                    );
                }
                break;
            case strhash(L"pack"):
                if (args.size() > 1)
                {
                    args[1].erase(std::remove_if(args[1].begin(), args[1].end(), [](wchar_t c) { return c == L'"' || c == L'\''; }), args[1].end());
                    if (args[1] == L".")
                    {
                        fputws(L"Stop trying to break my code! For some reason fs::exists doesn't think this path is invalid...\n", stdout);
                        break;
                    }
                    if (args[1].size() > 2)
                    {
                        bool invalidPath = true;
                        for (auto it = args[1].begin() + 2; it != args[1].end(); ++it)
                        {
                            if (*it != L'.')
                                invalidPath = false;
                        }
                        if (invalidPath)
                        {
                            fputws(L"Stop trying to break my code! For some reason fs::exists doesn't think this path is invalid...\n", stdout);
                            break;
                        }
                    }

                    fputws(L"Alrighty! Packaging folder into a binary file.\n", stdout);
                    if (!fs::exists(args[1].c_str()))
                    {
                        fputws(L"[folder] doesn't exist, create it!\n", stderr);
                        break;
                    }

                    Packager::packageFolder(args[1].c_str(), L"");

                    fputws(L"Done.\n", stdout);
                }
                else fputws(L"The command [pack] requires a folder argument!\n", stderr);
                break;
            case strhash(L"unpack"):
                if (args.size() > 1)
                {
                    args[1].erase(std::remove_if(args[1].begin(), args[1].end(), [](wchar_t c) { return c == L'"' || c == L'\''; }), args[1].end());
                    if (args[1] == L".")
                    {
                        fputws(L"Stop trying to break my code! For some reason fs::exists doesn't think this path is invalid...\n", stdout);
                        break;
                    }
                    if (args[1].size() > 2)
                    {
                        bool invalidPath = true;
                        for (auto it = args[1].begin() + 2; it != args[1].end(); ++it)
                        {
                            if (*it != L'.')
                                invalidPath = false;
                        }
                        if (invalidPath)
                        {
                            fputws(L"Stop trying to break my code! For some reason fs::exists doesn't think this path is invalid...\n", stdout);
                            break;
                        }
                    }

                    fputws(L"Alrighty! Unpackaging folder into a folder.\n", stdout);

                    if (fs::exists(args[1].c_str()))
                    {
                        if (fs::is_directory(args[1].c_str()))
                        {
                            fputws(L"[file] is a directory!\n", stdout);
                            break;
                        }

                        std::wstring unpackDir(fs::current_path().wstring() + L"/Package_Unpacked/");
                        if (fs::exists(unpackDir.c_str()))
                        {
                            fputws(L"The folder \"Package_Unpacked\" exists, removing...\n", stdout);
                            fs::remove_all(unpackDir.c_str());
                            fputws(L"Removed.\n", stdout);
                        }
                        fs::create_directories(unpackDir);

                        std::ifstream package(fs::path(args[1]), std::ios::binary);
                        package.seekg(0, std::ios::end);
                        uint32_t length = package.tellg();
                        package.seekg(0, std::ios::beg);

                        char* buffer = new char[1048576];

                        while (package.tellg() < length)
                        {
                            uint32_t filePathLength_endianUnconverted;
                            package.read(reinterpret_cast<char*>(&filePathLength_endianUnconverted), sizeof(uint32_t));
                            uint32_t filePathLength = toEndianness(filePathLength_endianUnconverted, Packager::Endianness::Big, Packager::endianness);

                            std::wstring filePath(unpackDir);
                            std::wstring relPath;
                            relPath.resize(filePathLength);
                            package.read(reinterpret_cast<char*>(&relPath[0]), sizeof(wchar_t) * filePathLength);
                            filePath.append(relPath);

                            wprintf(L"File Path - %ls.\n", filePath.c_str());

                            uint32_t fileLen_endianUnconverted;
                            package.read(reinterpret_cast<char*>(&fileLen_endianUnconverted), sizeof(uint32_t));
                            uint32_t fileLen = toEndianness(fileLen_endianUnconverted, Packager::Endianness::Big, Packager::endianness);
                            
                            fs::create_directories(fs::path(filePath.c_str()).parent_path());
                            std::ofstream outFile(fs::path(filePath.c_str()), std::ios::binary);

                            for (uint32_t i = 0; i < fileLen / 1048576; i++)
                            {
                                fputws(L"Writing 1048576 bytes.\n", stdout);
                                package.read(buffer, 1048576);
                                outFile.write(buffer, sizeof(char) * 1048576);
                                fputws(L"Written 1048576 bytes.\n", stdout);
                            }

                            uint32_t rem = fileLen % 1048576;
                            if (rem != 0)
                            {
                                wprintf(L"Writing %u remaining bytes.\n", rem);
                                package.read(buffer, rem);
                                outFile.write(buffer, sizeof(char) * rem);
                                wprintf(L"Written %u remaining bytes.\n", rem);
                            }
                        }

                        delete[] buffer;

                        fputws(L"Done.\n", stdout);
                    }
                    else fputws(L"Packaged file doesn't exist. Use the \"pack\" command to create one!\n", stderr);
                }
                else fputws(L"The command [unpack] requires a file argument!\n", stderr);
                break;
            default:
                fputws(L"Unknown command, use \"help\" to get a list of commands.\n", stdout);
            }
        }
    }

    Exit:;
}