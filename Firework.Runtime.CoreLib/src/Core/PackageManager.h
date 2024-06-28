#pragma once

#include "Firework.Runtime.CoreLib.Exports.h"

#include <filesystem>
#include <map>
#include <robin_hood.h>
#include <unordered_map>

#include <Library/Hash.h>
#include <Library/TypeInfo.h>

namespace Firework
{
    namespace Internal
    {
        class CoreEngine;
    }

    namespace PackageSystem
    {
        class BinaryPackageFile;
        class PackageManager;

        /// @brief Base type for any package file loaded by runtime.
        struct __firework_corelib_api PackageFile
        {
            struct Reflection {
                uint64_t typeID;
            } reflection;

            std::filesystem::path fileLocalPath;

            virtual ~PackageFile() = 0;

            friend class Firework::PackageSystem::PackageManager;
            friend class Firework::PackageSystem::BinaryPackageFile;
            friend class Firework::Internal::CoreEngine;
        protected:
            inline PackageFile() = default;
        private:
            inline PackageFile(uint64_t typeID, std::filesystem::path localPath) :
            reflection { typeID }, fileLocalPath(std::move(localPath))
            { }
        };

        /// @brief Package file for a generic binary format file.
        class __firework_corelib_api BinaryPackageFile final : public PackageFile
        {
            std::vector<uint8_t> data;
            
            inline BinaryPackageFile(std::vector<uint8_t> data, std::filesystem::path localPath) :
            PackageFile(__typeid(BinaryPackageFile).qualifiedNameHash(), std::move(localPath)), data(std::move(data))
            { }
        public:
            /// @brief Get the binary data of the file.
            /// @return Byte vector.
            /// @note Thread-safe, returned value is not.
            inline const std::vector<uint8_t>& binaryData()
            {
                return this->data;
            }

            friend class Firework::PackageSystem::PackageManager;
        };

        /// @brief Downcasts a package file to another more specific type.
        /// @tparam T ```requires std::is_base_of<PackageFile, T>::value```
        /// @param file Package file to cast.
        /// @return Downcasted package file.
        /// @retval - ```nullptr```: The package file is not of type ```T```.
        /// @retval - Otherwise, a pointer to a package file of type ```T```.
        /// @note Thread-safe, returned value is not.
        template <typename T>
        requires std::is_base_of<PackageFile, T>::value
        inline T* file_cast(PackageFile* file)
        {
            return (file && file->reflection.typeID == __typeid(T).qualifiedNameHash()) ? static_cast<T*>(file) : nullptr;
        }
        
        /// @brief Describes which endian this platform is.
        enum class Endianness : uint_fast8_t
        {
            Big = 0,
            Little = 1
        };
        /// @brief Static class containing functionality relevant to management of runtime packages and their content.
        class __firework_corelib_api PackageManager final
        {
            static Endianness endianness;

            static std::map<std::streamoff, std::map<size_t, robin_hood::unordered_map<std::basic_string<uint8_t>, PackageFile* (*)(std::vector<uint8_t>)>, std::greater<size_t>>> binFileHandlers;
            static robin_hood::unordered_map<std::wstring, PackageFile* (*)(std::u32string)> textFileHandlers;

            //                                             v Package file.
            //                               v File path.                          v Package path.
            static robin_hood::unordered_map<std::wstring, std::pair<PackageFile*, std::wstring>> loadedFiles;
            
            static void normalizePath(std::wstring& path);
        public:
            PackageManager() = delete;

            /// @brief Installs a binary file reader for a specific file signature (a short array of bytes, usually at the start of a file).
            /// @tparam PackageFileType ```requires std::is_base_of<PackageFile, PackageFileType>::value && std::is_final<PackageFileType>::value```
            /// @param signature File signature as byte vector.
            /// @param offset Byte index location of the signature.
            /// @note Main thread only.
            template <typename PackageFileType>
            requires std::is_base_of<PackageFile, PackageFileType>::value && std::is_final<PackageFileType>::value && requires
            {
                new PackageFileType(std::vector<uint8_t>());
            }
            inline static void addBinaryFileHandler(const std::vector<uint8_t>& signature, std::streamoff offset = 0)
            {
                PackageManager::binFileHandlers[offset][signature.size()].emplace
                (
                    std::basic_string<uint8_t>(signature.data(), signature.size()),
                    [](std::vector<uint8_t> data) -> PackageFile*
                    {
                        PackageFileType* ret = new PackageFileType(std::move(data));
                        ret->reflection.typeID = __typeid(PackageFileType).qualifiedNameHash();
                        return ret;
                    }
                );
            }
            /// @brief Remove a registered binary file reader.
            /// @param signature File signature as byte vector.
            /// @param offset Byte index location of the signature.
            /// @note Main thread only.
            static void removeBinaryFileHandler(const std::vector<uint8_t>& signature, std::streamoff offset);
            /// @brief Install a text file reader for a particular file extension. Text files are read as utf-8, but converted to utf-32.
            /// @tparam PackageFileType ```requires std::is_final<PackageFileType>::value && std::is_base_of<PackageFile, PackageFileType>::value```
            /// @param extension File extension.
            /// @param handler File reader function.
            /// @note Main thread only.
            template <typename PackageFileType>
            requires std::is_final<PackageFileType>::value && std::is_base_of<PackageFile, PackageFileType>::value
            inline static void addTextFileHandler(std::wstring extension, PackageFile* (*handler)(std::u32string))
            {
                PackageManager::textFileHandlers.insert({ std::move(extension), handler });
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
            static PackageFile* lookupFileByPath(std::wstring filePath);
            
            friend class Firework::Internal::CoreEngine;
        };
    }
}