#include "Packager.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <module/sys>

namespace fs = std::filesystem;

using namespace Firework::PackageSystem;

bool Packager::packageFolder(fs::path folder, fs::path outFile)
{
    std::error_code ec;

    fs::path outDir(outFile.parent_path());
    _fence_value_return(false, !fs::exists(outDir) && !fs::create_directories(outDir, ec));
    std::ofstream package(fs::path(outFile), std::ios::binary);
    _fence_value_return(false, !package);

    std::wstring curPath(fs::current_path(ec).wstring());
    curPath.append(L"/Package");

    for (auto& ent : fs::recursive_directory_iterator(folder))
    {
        fs::path p = ent.path();
        if (fs::is_directory(p))
            continue;

        std::wcout << L"[Firework.Packager | Firework.Tools] [INFO]  File - \"" << p.wstring() << L"\".\n";

        std::wstring filePath = folder.stem().append(fs::relative(p, folder, ec).wstring()).wstring();
        _fence_value_return(false, ec);

        u32 len = toEndianness(+u32(filePath.size()), std::endian::native, std::endian::big);
        _fence_value_return(false, !package.write(reinterpret_cast<char*>(&len), sizeof(u32)));
        len = toEndianness(+len, std::endian::big, std::endian::native);
        _fence_value_return(false, !package.write(reinterpret_cast<char*>(&filePath.front()), +(sizeof(wchar_t) * len)));
        std::wcout << L"[Firework.Packager | Firework.Tools] [INFO]  Relative File Path - \"" << filePath << L"\".\n";

        std::ifstream infile(p, std::ios::binary);

        infile.seekg(0, std::ios::beg);
        infile.ignore(std::numeric_limits<std::streamsize>::max());
        len = u32(infile.gcount());
        infile.clear();
        infile.seekg(0, std::ios::beg);
        std::wcout << L"[Firework.Packager | Firework.Tools] [INFO]  Filesize - " << +len << L".\n";

        len = toEndianness(+len, std::endian::native, std::endian::big);
        package.write(reinterpret_cast<char*>(&len), sizeof(u32));

        constexpr ssz bufferSize = 1048576;
        std::unique_ptr<byte[]> buffer = std::make_unique<byte[]>(+bufferSize);

        while (infile)
        {
            infile.read(reinterpret_cast<char*>(buffer.get()), +bufferSize);
            std::streamsize toWrite = infile.gcount();
            std::wcout << L"[Firework.Packager | Firework.Tools] [INFO]  Read " << toWrite << " bytes.\n";
            package.write(reinterpret_cast<char*>(buffer.get()), toWrite);
            std::wcout << L"[Firework.Packager | Firework.Tools] [INFO]  Written " << toWrite << " bytes.\n";
        }
    }
}