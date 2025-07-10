#include "PackageManager.h"

#include <bit>
#include <cctype>
#include <cmath>
#include <fstream>
#include <module/sys>

#include <Core/Application.h>
#include <Core/Debug.h>

using namespace Firework;
using namespace Firework::PackageSystem;
namespace fs = std::filesystem;

PackageFile::~PackageFile() = default;

constexpr auto toEndianness = [](auto intType, std::endian from, std::endian to) -> decltype(intType)
{
    if (from == to)
        return intType;
    else
    {
        decltype(intType) _int = 0;
        for (size_t i = 0; i < sizeof(decltype(intType)); i++) reinterpret_cast<int8_t*>(&_int)[i] = reinterpret_cast<int8_t*>(&intType)[sizeof(decltype(intType)) - 1 - i];
        return _int;
    }
};

bool PackageManager::removeBinaryFileHandler(const std::vector<uint8_t>& signature, std::streamoff offset)
{
    auto handlersWithOffsetIt = PackageManager::binFileHandlers.find(offset);
    _fence_value_return(false, handlersWithOffsetIt == PackageManager::binFileHandlers.end());

    auto handlersAlsoWithSigLenIt = handlersWithOffsetIt->second.find(signature.size());
    _fence_value_return(false, handlersAlsoWithSigLenIt == handlersWithOffsetIt->second.end());

    _fence_value_return(false, !handlersAlsoWithSigLenIt->second.erase(std::basic_string<uint8_t>(signature.data(), signature.size())));

    if (!handlersAlsoWithSigLenIt->second.empty())
        goto EarlyReturn;

    handlersWithOffsetIt->second.erase(handlersAlsoWithSigLenIt);

    if (!handlersWithOffsetIt->second.empty())
        goto EarlyReturn;

    PackageManager::binFileHandlers.erase(handlersWithOffsetIt);
EarlyReturn:
    return true;
}

std::map<std::streamoff, std::map<size_t, robin_hood::unordered_map<std::basic_string<uint8_t>, PackageFile* (*)(std::vector<uint8_t>)>, std::greater<size_t>>>
    PackageManager::binFileHandlers;
robin_hood::unordered_map<std::wstring, PackageFile* (*)(std::u32string)> PackageManager::textFileHandlers;

robin_hood::unordered_map<std::wstring, std::pair<PackageFile*, std::wstring>> PackageManager::loadedFiles;

bool PackageManager::loadPackageIntoMemory(const fs::path& packagePath)
{
    std::wstring packagePathNormalized = packagePath.wstring();
    std::ifstream packageFile(packagePath, std::ios::binary);

    _fence_value_return(false, !packageFile);

    packageFile.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize length = packageFile.gcount();
    packageFile.clear();
    packageFile.seekg(0, std::ios_base::beg);

    while (packageFile.tellg() < length)
    {
        uint32_t filePathLength_endianUnconverted;
        packageFile.read(reinterpret_cast<char*>(&filePathLength_endianUnconverted), sizeof(uint32_t));
        uint32_t filePathLength = toEndianness(filePathLength_endianUnconverted, std::endian::big, std::endian::native);

        std::wstring filePath;
        filePath.resize(filePathLength);
        packageFile.read(reinterpret_cast<char*>(&filePath[0]), sizeof(wchar_t) * filePathLength);
        PackageManager::normalizePath(filePath);
        Debug::logTrace("Package File - ", filePath, ".");

        uint32_t fileLen_endianUnconverted;
        packageFile.read(reinterpret_cast<char*>(&fileLen_endianUnconverted), sizeof(uint32_t));
        uint32_t fileLen = toEndianness(fileLen_endianUnconverted, std::endian::big, std::endian::native);
        Debug::logTrace("Package File Size - ", fileLen, "B.");

        std::vector<uint8_t> fileBytes(fileLen);
        packageFile.read(reinterpret_cast<char*>(fileBytes.data()), fileLen);

        for (auto it1 = PackageManager::binFileHandlers.begin(); it1 != PackageManager::binFileHandlers.end(); ++it1)
        {
            for (auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
            {
                std::basic_string<uint8_t> sig(fileBytes.data() + it1->first, fileBytes.data() + it1->first + it2->first);
                auto it3 = it2->second.find(sig);
                if (it3 != it2->second.end())
                {
                    PackageFile* file = it3->second(std::move(fileBytes));
                    file->fileLocalPath = filePath;
                    PackageManager::loadedFiles.emplace(filePath, std::make_pair(file, packagePathNormalized));
                    goto FileHandled;
                }
            }
        }
        if (auto it = PackageManager::textFileHandlers.find(fs::path(filePath).extension().wstring().c_str()); it != PackageManager::textFileHandlers.end())
        {
            // TODO: Implement.
            Debug::logError("Text file loading has not been implemented. This file handler will not be run.");
        }
        else
            PackageManager::loadedFiles.emplace(filePath, std::make_pair(new BinaryPackageFile(std::move(fileBytes), filePath), packagePathNormalized));

    FileHandled:;
    }

    return true;
}
void PackageManager::freePackageInMemory(const std::filesystem::path& packagePath)
{
    std::wstring packagePathNormalized = packagePath.wstring();
    PackageManager::normalizePath(packagePathNormalized);
    for (auto& [_, packageFileInfo] : PackageManager::loadedFiles)
    {
        if (packageFileInfo.second == packagePathNormalized)
            delete packageFileInfo.first;
    }
}

void PackageManager::normalizePath(std::wstring& path)
{
    bool extensionEnded = false;
    for (auto it = path.rbegin(); it != path.rend(); ++it)
    {
        switch (*it)
        {
        case L'\\': *it = L'/'; break;
        case L'.': extensionEnded = true; break;
        }
        if (!extensionEnded)
            *it = std::tolower(*it);
    }
}
PackageFile* PackageManager::lookupFileByPath(std::wstring filePath)
{
    PackageManager::normalizePath(filePath);
    if (auto it = PackageManager::loadedFiles.find(filePath); it != PackageManager::loadedFiles.end())
        return it->second.first;
    else
        return nullptr;
}