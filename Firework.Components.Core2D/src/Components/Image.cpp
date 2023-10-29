#include "Image.h"

#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <Mathematics.h>
#include <Core/Application.h>
#include <Core/CoreEngine.h>
#include <Core/Debug.h>
#include <Core/PackageManager.h>
#include <Components/RectTransform.h>
#include <EntityComponentSystem/EngineEvent.h>
#include <GL/Renderer.h>

#include <Image.vfAll.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::Mathematics;
using namespace Firework::GL;
using namespace Firework::PackageSystem;
namespace fs = std::filesystem;

GeometryProgramHandle Image::program;
robin_hood::unordered_map<PackageSystem::PortableGraphicPackageFile*, Image::TextureData*> Image::imageTextures;

Image::~Image()
{
    RenderData* data = this->data;
    if (this->data) [[likely]]
    {
        CoreEngine::queueRenderJobForFrame
        (
            [=]
            {
                data->internalMesh.destroy();
                delete data;
            }
        );
    }

    TextureData* textureData = this->textureData;
    if (textureData) [[likely]]
    {
        --textureData->accessCount;
        if (textureData->accessCount == 0)
        {
            Image::imageTextures.erase(this->file);
            CoreEngine::queueRenderJobForFrame
            (
                [=]
                {
                    textureData->internalTexture.destroy();
                    textureData->internalSampler.destroy();
                    delete textureData;
                }
            );
        }
    }
}

void Image::updateImageSplit()
{
    RectTransform* thisTransform = this->rectTransform();
    const RectFloat& thisRect = thisTransform->rect();
    float x0 = 0.0f, x1 = 2.0f * (this->split.left - thisRect.left) / (thisRect.right - thisRect.left), x2 = 2.0f - 2.0f * (thisRect.right - this->split.right) / (thisRect.right - thisRect.left), x3 = 2.0f;
    float y0 = 0.0f, y1 = 2.0f * (this->split.bottom - thisRect.bottom) / (thisRect.top - thisRect.bottom), y2 = 2.0f - 2.0f * (thisRect.top - this->split.top) / (thisRect.top - thisRect.bottom), y3 = 2.0f;
    
    struct ImageVertex
    {
        float x, y, z;
        float tc0x, tc0y;
    };
    ImageVertex squareVerts[]
    {
        { x0 - 1.0f, y3 - 1.0f, 1.0f, 0.0f,                     0.0f                            },
        { x1 - 1.0f, y3 - 1.0f, 1.0f, x1 / (x1 - x0 + x3 - x2), 0.0f                            },
        { x2 - 1.0f, y3 - 1.0f, 1.0f, x1 / (x1 - x0 + x3 - x2), 0.0f                            },
        { x3 - 1.0f, y3 - 1.0f, 1.0f, 1.0f,                     0.0f                            },
        { x0 - 1.0f, y2 - 1.0f, 1.0f, 0.0f,                     (y3 - y2) / (y1 - y0 + y3 - y2) },
        { x1 - 1.0f, y2 - 1.0f, 1.0f, x1 / (x1 - x0 + x3 - x2), (y3 - y2) / (y1 - y0 + y3 - y2) },
        { x2 - 1.0f, y2 - 1.0f, 1.0f, x1 / (x1 - x0 + x3 - x2), (y3 - y2) / (y1 - y0 + y3 - y2) },
        { x3 - 1.0f, y2 - 1.0f, 1.0f, 1.0f,                     (y3 - y2) / (y1 - y0 + y3 - y2) },
        { x0 - 1.0f, y1 - 1.0f, 1.0f, 0.0f,                     (y3 - y2) / (y1 - y0 + y3 - y2) },
        { x1 - 1.0f, y1 - 1.0f, 1.0f, x1 / (x1 - x0 + x3 - x2), (y3 - y2) / (y1 - y0 + y3 - y2) },
        { x2 - 1.0f, y1 - 1.0f, 1.0f, x1 / (x1 - x0 + x3 - x2), (y3 - y2) / (y1 - y0 + y3 - y2) },
        { x3 - 1.0f, y1 - 1.0f, 1.0f, 1.0f,                     (y3 - y2) / (y1 - y0 + y3 - y2) },
        { x0 - 1.0f, y0 - 1.0f, 1.0f, 0.0f,                     1.0f                            },
        { x1 - 1.0f, y0 - 1.0f, 1.0f, x1 / (x1 - x0 + x3 - x2), 1.0f                            },
        { x2 - 1.0f, y0 - 1.0f, 1.0f, x1 / (x1 - x0 + x3 - x2), 1.0f                            },
        { x3 - 1.0f, y0 - 1.0f, 1.0f, 1.0f,                     1.0f                            }
    };
    std::vector<uint16_t> squareInds; squareInds.reserve(24);

    uint16_t inds[]
    {
        1, 0, 4,
        1, 4, 5
    };
    if (y3 - y2 > 0.0f)
    {
        if (x1 - x0 > 0.0f)
        {
            for (size_t i = 0; i < 6; i++)
                squareInds.push_back(inds[i]);
        }
        if (x2 - x1 > 0.0f)
        {
            for (size_t i = 0; i < 6; i++)
                squareInds.push_back(inds[i] + 1);
        }
        if (x3 - x2 > 0.0f)
        {
            for (size_t i = 0; i < 6; i++)
                squareInds.push_back(inds[i] + 2);
        }
    }
    if (y2 - y1 > 0.0f)
    {
        if (x1 - x0 > 0.0f)
        {
            for (size_t i = 0; i < 6; i++)
                squareInds.push_back(inds[i] + 4);
        }
        if (x2 - x1 > 0.0f)
        {
            for (size_t i = 0; i < 6; i++)
                squareInds.push_back(inds[i] + 5);
        }
        if (x3 - x2 > 0.0f)
        {
            for (size_t i = 0; i < 6; i++)
                squareInds.push_back(inds[i] + 6);
        }
    }
    if (y1 - y0 > 0.0f)
    {
        if (x1 - x0 > 0.0f)
        {
            for (size_t i = 0; i < 6; i++)
                squareInds.push_back(inds[i] + 8);
        }
        if (x2 - x1 > 0.0f)
        {
            for (size_t i = 0; i < 6; i++)
                squareInds.push_back(inds[i] + 9);
        }
        if (x3 - x2 > 0.0f)
        {
            for (size_t i = 0; i < 6; i++)
                squareInds.push_back(inds[i] + 10);
        }
    }

    if (!this->data)
    {
        this->data = new RenderData;
        CoreEngine::queueRenderJobForFrame([data = this->data, squareVerts, squareInds = std::move(squareInds)]
        {
            data->internalMesh = DynamicMeshHandle::create
            (
                squareVerts, sizeof(squareVerts),
                VertexLayout::create
                ({
                    VertexDescriptor { .attribute = bgfx::Attrib::Position, .type = bgfx::AttribType::Float, .count = 3 },
                    VertexDescriptor { .attribute = bgfx::Attrib::TexCoord0, .type = bgfx::AttribType::Float, .count = 2 }
                }),
                squareInds.data(), squareInds.size() * sizeof(uint16_t)
            );
        });
    }
    else
    {
        CoreEngine::queueRenderJobForFrame([data = this->data, squareVerts, squareInds = std::move(squareInds)]
        {
            data->internalMesh.update
            (
                squareVerts, sizeof(squareVerts),
                squareInds.data(), squareInds.size() * sizeof(uint16_t)
            );
        });
    }
}
void Image::setImageSplit(const RectFloat& value)
{
    if (this->split == value) [[unlikely]]
        return;
    this->split = value;
    if (this->file) [[likely]]
        this->updateImageSplit();
}
void Image::setImageFile(PortableGraphicPackageFile* value)
{
    if (this->textureData)
    {
        --this->textureData->accessCount;
        if (this->textureData->accessCount == 0)
        {
            Image::imageTextures.erase(this->file);
            CoreEngine::queueRenderJobForFrame
            (
                [textureData = this->textureData]
                {
                    textureData->internalTexture.destroy();
                    textureData->internalSampler.destroy();
                    delete textureData;
                }
            );
        }
    }
    if (value)
    {
        if (this->file != value)
        {
            this->file = value;
            auto it = Image::imageTextures.find(value);
            if (it != Image::imageTextures.end())
            {
                this->textureData = it->second;
                ++this->textureData->accessCount;
            }
            else
            {
                this->textureData = Image::imageTextures.emplace(value, new TextureData { 1, { }, { } }).first->second;

                uint32_t width = value->imageWidth(), height = value->imageHeight();
                uint32_t imageDataSize = width * height * 4;
                std::vector<uint8_t> imageData(value->imageRGBAData(), value->imageRGBAData() + imageDataSize);
                
                CoreEngine::queueRenderJobForFrame
                (
                    [width, height, textureData = this->textureData, imageData = std::move(imageData)]
                    {
                        textureData->internalTexture = Texture2DHandle::create(imageData.data(), width * height * 4, width, height);
                        textureData->internalSampler = TextureSamplerHandle::create("s_imageTexture");
                    }
                );

                this->updateImageSplit();
            }
        }
    }
    else
    {
        this->textureData = nullptr;
        this->file = nullptr;
        
        if (this->data) [[likely]]
        {
            CoreEngine::queueRenderJobForFrame
            (
                [data = this->data]
                {
                    data->internalMesh.destroy();
                    delete data;
                }
            );
            this->data = nullptr;
        }
    }
}

void Image::renderInitialize()
{
    CoreEngine::queueRenderJobForFrame([]
    {
        switch (Renderer::rendererBackend())
        {
        #if _WIN32
        case RendererBackend::Direct3D9:
            Image::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Image, d3d9), { ShaderUniform { .name = "u_tint", .type = UniformType::Vec4 } });
            break;
        case RendererBackend::Direct3D11:
            Image::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Image, d3d11), { ShaderUniform { .name = "u_tint", .type = UniformType::Vec4 } });
            break;
        case RendererBackend::Direct3D12:
            Image::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Image, d3d12), { ShaderUniform { .name = "u_tint", .type = UniformType::Vec4 } });
            break;
        #endif
        case RendererBackend::OpenGL:
            Image::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Image, opengl), { ShaderUniform { .name = "u_tint", .type = UniformType::Vec4 } });
            break;
        case RendererBackend::Vulkan:
            Image::program = GeometryProgramHandle::create(getGeometryProgramArgsFromPrecompiledShaderName(Image, vulkan), { ShaderUniform { .name = "u_tint", .type = UniformType::Vec4 } });
            break;
        default:
            throw "unimplemented";
        }
    });
    InternalEngineEvent::OnRenderShutdown += []
    {
        for (auto it = Image::imageTextures.begin(); it != Image::imageTextures.end(); ++it)
        {
            it->second->internalTexture.destroy();
            delete it->second;
        }

        Image::program.destroy();
    };
}
void Image::renderOffload()
{
    if (this->textureData)
    {
        CoreEngine::queueRenderJobForFrame([t = renderTransformFromRectTransform(this->rectTransform()), data = this->data, textureData = this->textureData, tint = this->tint]
        {
            Renderer::setDrawTexture(0, textureData->internalTexture, textureData->internalSampler);
            float col[4] { (float)tint.r / 255.0f, (float)tint.g / 255.0f, (float)tint.b / 255.0f, (float)tint.a / 255.0f };
            Image::program.setUniform("u_tint", col);
            Renderer::setDrawTransform(t);
            Renderer::submitDraw
            (
                1, data->internalMesh, Image::program,
                BGFX_STATE_CULL_CW | BGFX_STATE_DEPTH_TEST_ALWAYS | BGFX_STATE_WRITE_Z |
                BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA
            );
        }, false);
    }
}
