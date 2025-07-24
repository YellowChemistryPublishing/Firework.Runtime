#include "PackageManager.h"

#include <bit>
#include <cctype>
#include <cmath>
#include <fstream>

#include <Core/Application.h>
#include <Core/Debug.h>

using namespace Firework;
using namespace Firework::PackageSystem;
namespace fs = std::filesystem;

constexpr auto toEndianness = [](auto intType, std::endian from, std::endian to) -> decltype(intType)
{
    if (from == to)
        return intType;
    else
        return std::byteswap(intType);
};

bool PackageManager::removeBinaryFileHandler(const std::vector<uint8_t>& signature, std::streamoff offset)
{
    auto handlersWithOffsetIt = PackageManager::binFileHandlers.find(FileSignatureQuery { .where = offset, .size = signature.size() });
    _fence_value_return(false, handlersWithOffsetIt == PackageManager::binFileHandlers.end());

    _fence_value_return(false, !handlersWithOffsetIt->second.erase(std::basic_string<uint8_t>(signature.data(), signature.size())));

    if (handlersWithOffsetIt->second.empty())
        PackageManager::binFileHandlers.erase(handlersWithOffsetIt);

    return true;
}

std::map<PackageManager::FileSignatureQuery, robin_hood::unordered_map<std::basic_string<byte>, std::shared_ptr<PackageFile> (*)(std::vector<byte>)>,
         std::greater<PackageManager::FileSignatureQuery>>
    PackageManager::binFileHandlers;
robin_hood::unordered_map<std::wstring, std::shared_ptr<PackageFile> (*)(std::u8string)> PackageManager::textFileHandlers;

robin_hood::unordered_map<std::wstring, PackageManager::PackageFileData> PackageManager::loadedFiles;

bool PackageManager::loadPackageIntoMemory(const fs::path& packagePath)
{
    std::wstring packagePathNormalized = packagePath.wstring();
    std::ifstream packageFile(packagePath, std::ios::binary);

    _fence_value_return(false, !packageFile);

    do
    {
        u32 filePathLength;
        _fence_value_return(false, !packageFile.read(reinterpret_cast<char*>(&filePathLength), sizeof(u32)));
        filePathLength = toEndianness(uint32_t(+filePathLength), std::endian::big, std::endian::native);

        std::wstring filePath(+filePathLength, L' ');
        _fence_value_return(false, !packageFile.read(reinterpret_cast<char*>(filePath.data()), +(sizeof(wchar_t) * filePathLength)));
        PackageManager::normalizePath(filePath);
        Debug::logTrace("Package File - ", filePath, ".");

        u32 fileLen;
        _fence_value_return(false, !packageFile.read(reinterpret_cast<char*>(&fileLen), sizeof(u32)));
        fileLen = toEndianness(uint32_t(+fileLen), std::endian::big, std::endian::native);
        Debug::logTrace("Package File Size - ", +fileLen, "B.");

        std::vector<byte> fileBytes(+fileLen);
        _fence_value_return(false, !packageFile.read(reinterpret_cast<char*>(fileBytes.data()), +fileLen));

        auto pushLoadedFile = [&](std::shared_ptr<PackageFile> file)
        {
            file->packagePath = packagePathNormalized;
            file->filePath = filePath;
            PackageManager::loadedFiles.emplace(filePath, PackageFileData { .loadedFile = std::move(file), .packagePath = packagePathNormalized });
        };

        if (auto handlerIt = PackageManager::textFileHandlers.find(fs::path(filePath).extension().wstring()); handlerIt != PackageManager::textFileHandlers.end())
        {
            std::u8string fileContents(fileBytes.begin(), fileBytes.end());
            pushLoadedFile(handlerIt->second(std::move(fileContents)));
            goto Done;
        }

        for (auto& [sigData, handlers] : PackageManager::binFileHandlers)
        {
            const auto& [off, sigLen] = sigData;
            if (off + sigLen > fileLen)
                continue;

            std::basic_string<uint8_t> sig(fileBytes.data() + off, fileBytes.data() + +(off + sigLen));
            auto handlerIt = handlers.find(sig);
            if (handlerIt != handlers.end())
            {
                pushLoadedFile(handlerIt->second(std::move(fileBytes)));
                goto Done;
            }
        }

        PackageManager::loadedFiles.emplace(filePath,
                                            PackageFileData { .loadedFile = std::make_shared<BinaryPackageFile>(std::move(fileBytes)), .packagePath = packagePathNormalized });
    Done:;
    }
    while (packageFile);

    return true;
}
void PackageManager::freePackageInMemory(const std::filesystem::path& packagePath)
{
    std::wstring packagePathNormalized = packagePath.wstring();
    PackageManager::normalizePath(packagePathNormalized);
    for (auto loadedFileIt = PackageManager::loadedFiles.begin(); loadedFileIt != PackageManager::loadedFiles.end();)
    {
        const auto& [file, packagePath] = loadedFileIt->second;
        if (packagePath == packagePathNormalized)
            loadedFileIt = PackageManager::loadedFiles.erase(loadedFileIt);
        else
            ++loadedFileIt;
    }
}

void PackageManager::normalizePath(std::wstring& path)
{
    bool extensionEnded = false;
    for (auto it = path.rbegin(); it != path.rend(); ++it)
    {
        switch (*it)
        {
        case L'\\':
            *it = L'/';
            break;
        case L'.':
            extensionEnded = true;
            break;
        }
        if (!extensionEnded)
            *it = wchar_t(std::tolower(*it));
    }
}
std::shared_ptr<PackageFile> PackageManager::lookupFileByPath(std::wstring filePath)
{
    PackageManager::normalizePath(filePath);
    if (auto it = PackageManager::loadedFiles.find(filePath); it != PackageManager::loadedFiles.end())
        return it->second.loadedFile;
    else
        return nullptr;
}
