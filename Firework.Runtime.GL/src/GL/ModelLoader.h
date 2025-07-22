#pragma once

#include "Firework.Runtime.GL.Exports.h"

#include <cstdint>
#include <string>
#include <vector>

#include <GL/Geometry.h>

namespace Assimp
{
    class Importer;
}

namespace Firework
{
    namespace GL
    {
        class SceneAsset;
        class ModelLoader;

        class StaticMeshHandle;
        
        struct MeshVertex
        {
            float x, y, z;
            float nx, ny, nz;
            float r, g, b, a;
            float tc0x, tc0y, tc0z;
            // float tc1x, tc1y, tc1z;
            // float tc2x, tc2y, tc2z;
            // float tc3x, tc3y, tc3z;
            // float tc4x, tc4y, tc4z;
            // float tc5x, tc5y, tc5z;
            // float tc6x, tc6y, tc6z;
            // float tc7x, tc7y, tc7z;

            constexpr bool operator==(const MeshVertex&) const noexcept = default;
        };
        class _fw_gl_api SceneMesh final
        {
            std::string _name;
            std::vector<MeshVertex> _vertices;
            std::vector<uint16_t> _indices;

            inline SceneMesh(const char* meshName) : _name(meshName)
            { }
        public:
            inline SceneMesh() = default;

            inline const std::string& name() const noexcept
            {
                return this->_name;
            }

            inline const std::vector<MeshVertex>& vertices() const noexcept
            {
                return this->_vertices;
            }
            inline const std::vector<uint16_t>& indices() const noexcept
            {
                return this->_indices;
            }

            StaticMeshHandle createRenderableMesh() const;
            DynamicMeshHandle createRenderableMesh(std::nullptr_t) const;

            friend class Firework::GL::SceneAsset;
            friend class Firework::GL::ModelLoader;
        };
        class _fw_gl_api SceneNode final
        {
            std::vector<SceneMesh> _meshes;
            std::vector<SceneNode> _children;
        public:
            inline const std::vector<SceneMesh>& meshes() const noexcept
            {
                return this->_meshes;
            }
            inline const std::vector<SceneNode>& children() const noexcept
            {
                return this->_children;
            }

            friend class Firework::GL::SceneAsset;
            friend class Firework::GL::ModelLoader;
        };
        class _fw_gl_api SceneAsset final
        {
            SceneNode _root;
        public:
            inline const std::vector<SceneMesh>& meshes() const noexcept
            {
                return this->_root._meshes;
            }
            inline const std::vector<SceneNode>& children() const noexcept
            {
                return this->_root._children;
            }

            inline const SceneNode& root() const noexcept
            {
                return this->_root;
            }

            std::string generateSceneStructureDescription();

            friend class Firework::GL::ModelLoader;
        };

        class _fw_gl_api ModelLoader final
        {
            static Assimp::Importer importer;
        public:
            ModelLoader() = delete;

            static SceneAsset* loadModel(const uint8_t* data, size_t size, bool flipYZ = false);
        };
    }
}