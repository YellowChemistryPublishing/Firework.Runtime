#include "Mesh.h"

#include <Core/CoreEngine.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <GL/Renderer.h>
#include <Components/Transform.h>

#include <Mesh.vfAll.h>
#include <cstdint>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::GL;

GeometryProgramHandle Mesh::program;
std::unordered_map<std::pair<std::vector<MeshVertex>, std::vector<uint16_t>>, Mesh::RenderData*, Mesh::MeshHash> Mesh::meshes;
robin_hood::unordered_map<Mesh::RenderData*, std::vector<Mesh*>> Mesh::meshInstances;

void Mesh::setMesh(const GL::SceneMesh* value)
{
    auto meshData = std::make_pair(value->vertices(), value->indices());
    if (this->data)
    {
        --this->data->accessCount;
        if (this->data->accessCount == 0)
        {
            Mesh::meshes.erase(meshData);
            CoreEngine::queueRenderJobForFrame([data = this->data, type = this->_meshType]
            {
                if (type != MeshType::Static)
                    delete data->meshData;
                
                switch (data->type)
                {
                case MeshType::Static:
                    data->staticMesh.destroy();
                    break;
                case MeshType::Dynamic:
                    data->dynMesh.destroy();
                    break;
                case MeshType::Transient:
                    throw "unimplemented";
                    break;
                }

                delete data;
            });
        }
    }
    if (value)
    {
        switch (this->_meshType)
        {
        case MeshType::Static:
            if (auto it = Mesh::meshes.find(meshData);
                it != Mesh::meshes.end())
            {
                ++it->second->accessCount;
                this->data = it->second;
            }
            else
            {
                RenderData* data = new RenderData { 1, nullptr, { } };
                data->meshData = &Mesh::meshes.insert(std::make_pair(std::move(meshData), data)).first->first;
                data->staticMesh = StaticMeshHandle::create
                (
                    value->vertices().data(), value->vertices().size() * sizeof(MeshVertex),
                    VertexLayout::create
                    ({
                        VertexDescriptor { .attribute = bgfx::Attrib::Position, .type = bgfx::AttribType::Float, .count = 3 },
                        VertexDescriptor { .attribute = bgfx::Attrib::Normal, .type = bgfx::AttribType::Float, .count = 3 },
                        VertexDescriptor { .attribute = bgfx::Attrib::Color0, .type = bgfx::AttribType::Float, .count = 4 },
                        VertexDescriptor { .attribute = bgfx::Attrib::TexCoord0, .type = bgfx::AttribType::Float, .count = 3 }
                    }),
                    value->indices().data(), value->indices().size() * sizeof(uint16_t)
                );
                this->data = data;
            }
            break;
        case MeshType::Dynamic:
            break;
        case MeshType::Transient:
            break;
        }
    }
    else this->data = nullptr;
}
void Mesh::setMeshType(MeshType value)
{
}

void Mesh::renderInitialize()
{
    CoreEngine::queueRenderJobForFrame([]
    {
        switch (Renderer::rendererBackend())
        {
        #if _WIN32
        case RendererBackend::Direct3D9:
            Mesh::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Mesh, d3d9));
            break;
        case RendererBackend::Direct3D11:
            Mesh::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Mesh, d3d11));
            break;
        case RendererBackend::Direct3D12:
            Mesh::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Mesh, d3d12));
            break;
        #endif
        case RendererBackend::OpenGL:
            Mesh::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Mesh, opengl));
            break;
        case RendererBackend::Vulkan:
            Mesh::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Mesh, vulkan));
            break;
        default:
            throw "unimplemented";
        }
    });
    InternalEngineEvent::OnRenderShutdown += []
    {
        for (auto it = Mesh::meshes.begin(); it != Mesh::meshes.end(); ++it)
        {
            switch (it->second->type)
            {
            case MeshType::Static:
                it->second->staticMesh.destroy();
                break;
            case MeshType::Dynamic:
                it->second->dynMesh.destroy();
                break;
            case MeshType::Transient:
                throw "unimplemented";
                break;
            }
            delete it->second;
        }
        Mesh::meshInstances.clear();
        Mesh::program.destroy();
    };
}
void Mesh::renderOffload()
{
    if (this->data) [[likely]]
    {
        CoreEngine::queueRenderJobForFrame([data = this->data, t = renderTransformFromTransform(this->transform())]
        {
            Renderer::setDrawTransform(t);
            switch (data->type)
            {
            case MeshType::Static:
                Renderer::submitDraw(1, data->staticMesh, Mesh::program);
                break;
            case MeshType::Dynamic:
                Renderer::submitDraw(1, data->dynMesh, Mesh::program);
                break;
            case MeshType::Transient:
                throw "unimplemented";
                break;
            }
        }, false);
    }
}