#include "Packager.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

using namespace Firework::PackageSystem;

Packager::Endianness Packager::endianness;

void Packager::packageFolder(fs::path folder, fs::path outFile)
{
    fs::path outDir(outFile.parent_path());
    if (!fs::exists(outDir))
        fs::create_directory(outDir);
    std::ofstream package(fs::path(outFile), std::ios::binary);

    for (auto& path : fs::recursive_directory_iterator(folder))
    {
        if (!fs::is_directory(path.path()))
        {
            std::wcout << L"[Firework.Packager.CLI] [INFO]  File - \"" << path.path().wstring() << L"\".\n";
            std::wstring curPath(fs::current_path().wstring());
            curPath.append(L"/Package");

            std::wstring filePath = fs::relative(path.path(), folder).wstring();
            filePath = folder.stem().append(filePath).wstring();
            uint32_t len = filePath.size();
            uint32_t len_endianConverted = toEndianness(len, Packager::endianness, Packager::Endianness::Big);
            package.write(reinterpret_cast<char*>(&len_endianConverted), sizeof(uint32_t));
            package.write(reinterpret_cast<char*>(&filePath[0]), sizeof(wchar_t) * len);
            std::wcout << L"[Firework.Packager.CLI] [INFO]  Relative File Path - \"" << filePath.c_str() << L"\".\n";
            
            std::ifstream infile(path.path(), std::ios::binary);

            infile.seekg(0, std::ios::end);
            uint32_t length = infile.tellg();
            std::wcout << L"[Firework.Packager.CLI] [INFO]  Filesize - " << length << L".\n";
            uint32_t length_endianConverted = toEndianness(length, Packager::endianness, Packager::Endianness::Big);
            package.write(reinterpret_cast<char*>(&length_endianConverted), sizeof(uint32_t));
            infile.seekg(0, std::ios::beg);

            char* buffer = new char[1048576];

            for (uint32_t i = 0; i < length / 1048576; i++)
            {
                std::wcout << L"[Firework.Packager.CLI] [INFO]  Writing 1048576 bytes.\n";
                infile.read(buffer, 1048576);
                package.write(buffer, sizeof(char) * 1048576);
                std::wcout << L"[Firework.Packager.CLI] [INFO]  Written 1048576 bytes.\n";
            }

            uint32_t rem = length % 1048576;
            if (rem != 0)
            {
                std::wcout << L"[Firework.Packager.CLI] [INFO]  Writing " << rem << " remaining bytes.\n";
                infile.read(buffer, rem);
                package.write(buffer, sizeof(char) * rem);
                std::wcout << L"[Firework.Packager.CLI] [INFO]  Written " << rem << " remaining bytes.\n";
            }

            delete[] buffer;
        }
    }
}