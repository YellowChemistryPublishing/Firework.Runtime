#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <filesystem>
#include <map>
#include <module/sys>
#include <robin_hood.h>
#include <typeinfo>
#include <unordered_map>

namespace Firework::Internal
{
    class CoreEngine;
}

namespace Firework::PackageSystem
{
    class PackageManager;

    /// @brief Base type for any package file loaded by runtime.
    struct __firework_corelib_api PackageFile
    {
        std::wstring packagePath = L"";
        std::wstring filePath = L"";

        inline virtual ~PackageFile() = 0;

        friend class Firework::PackageSystem::PackageManager;
        friend class Firework::Internal::CoreEngine;
    protected:
        inline PackageFile() = default;
    };

    /// @brief Package file for a generic binary format file.
    class __firework_corelib_api BinaryPackageFile final : public PackageFile
    {
        std::vector<uint8_t> data;
    public:
        inline BinaryPackageFile(std::vector<uint8_t> data) : data(std::move(data))
        { }

        /// @brief Get the binary data of the file.
        /// @return Byte vector.
        /// @note Thread-safe, returned value is not.
        inline const std::vector<uint8_t>& binaryData()
        {
            return this->data;
        }
    };

    /// @brief Static class containing functionality relevant to management of runtime packages and their content.
    class __firework_corelib_api PackageManager final
    {
        struct FileSignatureQuery
        {
            std::streamoff where;
            u32 size;

            friend constexpr auto operator<=>(const FileSignatureQuery& a, const FileSignatureQuery& b)
            {
                return a.where == b.where ? +a.size <=> +b.size : a.where <=> b.where;
            }
        };
        static std::map<FileSignatureQuery, robin_hood::unordered_map<std::basic_string<byte>, std::shared_ptr<PackageFile> (*)(std::vector<byte>)>,
                        std::greater<FileSignatureQuery>>
            binFileHandlers;
        static robin_hood::unordered_map<std::wstring, std::shared_ptr<PackageFile> (*)(const std::u8string&)> textFileHandlers;

        struct PackageFileData
        {
            std::shared_ptr<PackageFile> loadedFile;
            std::wstring packagePath;
        };
        static robin_hood::unordered_map<std::wstring, PackageFileData> loadedFiles;

        static void normalizePath(std::wstring& path);
    public:
        PackageManager() = delete;

        /// @brief Installs a binary file reader for a specific file signature (a short array of bytes, usually at the start of a file).
        /// @tparam PackageFileType ```requires std::is_base_of<PackageFile, PackageFileType>::value && std::is_final<PackageFileType>::value```
        /// @param signature File signature as byte vector.
        /// @param offset Byte index location of the signature.
        /// @note Main thread only.
        template <typename PackageFileType>
        requires std::is_base_of<PackageFile, PackageFileType>::value && std::is_final<PackageFileType>::value && requires { new PackageFileType(std::vector<uint8_t>()); }
        inline static void addBinaryFileHandler(const std::vector<uint8_t>& signature, std::streamoff offset = 0)
        {
            PackageManager::binFileHandlers[FileSignatureQuery { .where = offset, .size = signature.size() }][std::basic_string<uint8_t>(signature.data(), signature.size())] =
                [](std::vector<uint8_t> data) -> std::shared_ptr<PackageFile>
            {
                return std::make_shared<PackageFileType>(std::move(data));
            };
        }
        /// @brief Remove a registered binary file reader.
        /// @param signature File signature as byte vector.
        /// @param offset Byte index location of the signature.
        /// @note Main thread only.
        [[nodiscard]] static bool removeBinaryFileHandler(const std::vector<uint8_t>& signature, std::streamoff offset);
        /// @brief Install a text file reader for a particular file extension. Text files are read as utf-8, but converted to utf-32.
        /// @tparam PackageFileType ```requires std::is_final<PackageFileType>::value && std::is_base_of<PackageFile, PackageFileType>::value```
        /// @param extension File extension.
        /// @param handler File reader function.
        /// @note Main thread only.
        template <typename PackageFileType>
        requires std::is_final<PackageFileType>::value && std::is_base_of<PackageFile, PackageFileType>::value
        inline static void addTextFileHandler(std::wstring extension, std::shared_ptr<PackageFile> (*handler)(const std::u8string&))
        {
            PackageManager::textFileHandlers[std::move(extension)] = handler;
        }
        /// @brief Remove a registered text file reader.
        /// @param extension File extension.
        /// @note Main thread only.
        inline static void removeTextFileHandler(const std::wstring& extension)
        {
            PackageManager::textFileHandlers.erase(extension);
        }

        static bool loadPackageIntoMemory(const std::filesystem::path& packagePath);
        static void freePackageInMemory(const std::filesystem::path& packagePath);

        /// @brief Retrieve a file from the list of loaded package files.
        /// @param filePath The path of the file within the package filesystem.
        /// @return Package file.
        /// @retval ```nullptr```: Package file could not be found.
        /// @retval Otherwise, pointer to loaded package file.
        /// @note Main thread only.
        static std::shared_ptr<PackageFile> lookupFileByPath(std::wstring filePath);

        friend class Firework::Internal::CoreEngine;
    };
} // namespace Firework::PackageSystem