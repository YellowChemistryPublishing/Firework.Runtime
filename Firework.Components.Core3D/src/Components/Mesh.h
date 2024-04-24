#pragma once

#include "Firework.Components.Core3D.Exports.h"

#include <robin_hood.h>
#include <span>
#include <unordered_map>
#include <variant>
#include <vector>

#include <GL/Shader.h>
#include <GL/ModelLoader.h>
#include <Library/Property.h>
#include <Objects/Component.h>

namespace Firework
{
    namespace Internal
    {
        struct ComponentCoreStaticInit;
    }

    enum class MeshType : uint_fast8_t
    {
        Static,
        Dynamic,
        Transient
    };
    
    class __firework_componentcore3d_api Mesh final : public Internal::Component
    {
        struct MeshHash
        {
            size_t operator()(const std::pair<std::vector<GL::MeshVertex>, std::vector<uint16_t>>& mesh) const
            {
                size_t ret = 0;
                size_t shift = 0;
                for (auto& vert : mesh.first)
                {
                    for (uint8_t byte : std::span((uint8_t*)&vert, (uint8_t*)&vert + sizeof(GL::MeshVertex)))
                    {
                        ret ^= size_t(byte) << shift;
                        if (++shift == sizeof(size_t) - sizeof(uint8_t))
                            shift = 0;
                    }
                }
                for (auto& ind : mesh.second)
                {
                    for (uint8_t byte : std::span((uint8_t*)&ind, (uint8_t*)&ind + sizeof(uint16_t)))
                    {
                        ret ^= size_t(byte) << shift;
                        if (++shift == sizeof(size_t) - sizeof(uint8_t))
                            shift = 0;
                    }
                }
                return ret;
            }
        };

        struct RenderData
        {
            uint_fast32_t accessCount;
            const std::pair<std::vector<GL::MeshVertex>, std::vector<uint16_t>>* meshData;
            union
            {
                GL::StaticMeshHandle staticMesh;
                GL::DynamicMeshHandle dynMesh;
            };
            MeshType type;
        }* data;
        static std::unordered_map<std::pair<std::vector<GL::MeshVertex>, std::vector<uint16_t>>, RenderData*, MeshHash> meshes;
        static robin_hood::unordered_map<RenderData*, std::vector<Mesh*>> meshInstances;

        MeshType _meshType = MeshType::Static;

        void setMesh(const GL::SceneMesh* value);
        void setMeshType(MeshType type);
        
        static void renderInitialize();
        void renderOffload();
    public:
        const Property<void, const GL::SceneMesh*> mesh
        {
            [this](const GL::SceneMesh* value) -> void
            {
                this->setMesh(value);
            }
        };
        const Property<MeshType, MeshType> meshType
        {{
            [this]() -> MeshType
            {
                return this->_meshType;
            },
            [this](MeshType value) -> void
            {
                this->setMeshType(value);
            }
        }};

        friend struct Firework::Internal::ComponentCoreStaticInit;
    };
}