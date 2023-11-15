#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>
#include <utility>

#include <GL/Geometry.h>

using namespace Firework::GL;

Assimp::Importer ModelLoader::importer;

StaticMeshHandle SceneMesh::createRenderableMesh() const
{
    return StaticMeshHandle::create
    (
        this->vertices().data(), this->vertices().size() * sizeof(MeshVertex),
        VertexLayout::create
        ({
            VertexDescriptor { .attribute = bgfx::Attrib::Position, .type = bgfx::AttribType::Float, .count = 3 },
            VertexDescriptor { .attribute = bgfx::Attrib::Normal, .type = bgfx::AttribType::Float, .count = 3 },
            VertexDescriptor { .attribute = bgfx::Attrib::Color0, .type = bgfx::AttribType::Float, .count = 4 },
            VertexDescriptor { .attribute = bgfx::Attrib::TexCoord0, .type = bgfx::AttribType::Float, .count = 3 }
        }),
        this->indices().data(), this->indices().size() * sizeof(uint16_t)
    );
}
DynamicMeshHandle SceneMesh::createRenderableMesh(std::nullptr_t) const
{
    return DynamicMeshHandle::create
    (
        this->vertices().data(), this->vertices().size() * sizeof(MeshVertex),
        VertexLayout::create
        ({
            VertexDescriptor { .attribute = bgfx::Attrib::Position, .type = bgfx::AttribType::Float, .count = 3 },
            VertexDescriptor { .attribute = bgfx::Attrib::Normal, .type = bgfx::AttribType::Float, .count = 3 },
            VertexDescriptor { .attribute = bgfx::Attrib::Color0, .type = bgfx::AttribType::Float, .count = 4 },
            VertexDescriptor { .attribute = bgfx::Attrib::TexCoord0, .type = bgfx::AttribType::Float, .count = 3 }
        }),
        this->indices().data(), this->indices().size() * sizeof(uint16_t)
    );
}

std::string SceneAsset::generateSceneStructureDescription()
{
    std::string ret;
    auto appendRecursive = [&](auto& node, size_t tabs, auto&& appendRecursive) -> void
    {
        for (auto it = node._meshes.begin(); it != node._meshes.end(); ++it)
        {
            for (size_t i = 0; i < tabs; i++)
                ret.push_back('\t');
            ret.append(it->_name);
            ret.push_back('\n');
        }
        for (auto it = node._children.begin(); it != node._children.end(); ++it)
            appendRecursive(*it, ++tabs, appendRecursive);
    };
    appendRecursive(this->_root, 0, appendRecursive);
    ret.pop_back();
    return ret;
}

SceneAsset* ModelLoader::loadModel(const uint8_t* data, size_t size, bool flipYZ)
{
    const aiScene* scene = ModelLoader::importer.ReadFileFromMemory(data, size * sizeof(uint8_t), aiProcess_Triangulate | aiProcess_MakeLeftHanded);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << ModelLoader::importer.GetErrorString() << '\n';
        return nullptr;
    }

    SceneAsset* ret = new SceneAsset;
    auto importRecursive = [&](aiNode* node, auto& scNode, auto&& importRecursive) -> void
    {
        scNode._meshes.reserve(node->mNumMeshes);
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            SceneMesh mesh(scene->mMeshes[node->mMeshes[i]]->mName.data);
            mesh._vertices.reserve(scene->mMeshes[node->mMeshes[i]]->mNumVertices);
            for (unsigned int j = 0; j < scene->mMeshes[node->mMeshes[i]]->mNumVertices; j++)
            {
                static_assert(AI_MAX_NUMBER_OF_TEXTURECOORDS == 8, "Assimp texture coordinate count must be exactly 8.");
                MeshVertex v;
                if (scene->mMeshes[node->mMeshes[i]]->HasPositions())
                {
                    v.x = scene->mMeshes[node->mMeshes[i]]->mVertices[j].x;
                    if (flipYZ)
                    {
                        v.z = -scene->mMeshes[node->mMeshes[i]]->mVertices[j].y;
                        v.y = -scene->mMeshes[node->mMeshes[i]]->mVertices[j].z;
                    }
                    else
                    {
                        v.y = scene->mMeshes[node->mMeshes[i]]->mVertices[j].y;
                        v.z = scene->mMeshes[node->mMeshes[i]]->mVertices[j].z;
                    }
                }
                if (scene->mMeshes[node->mMeshes[i]]->HasNormals())
                {
                    v.xNorm = scene->mMeshes[node->mMeshes[i]]->mNormals[j].x;
                    v.yNorm = scene->mMeshes[node->mMeshes[i]]->mNormals[j].y;
                    v.zNorm = scene->mMeshes[node->mMeshes[i]]->mNormals[j].z;
                }
                if (scene->mMeshes[node->mMeshes[i]]->HasVertexColors(0))
                {
                    v.r = scene->mMeshes[node->mMeshes[i]]->mColors[0][j].r;
                    v.g = scene->mMeshes[node->mMeshes[i]]->mColors[0][j].g;
                    v.b = scene->mMeshes[node->mMeshes[i]]->mColors[0][j].b;
                    v.a = scene->mMeshes[node->mMeshes[i]]->mColors[0][j].a;
                }
                if (scene->mMeshes[node->mMeshes[i]]->HasTextureCoords(0))
                {
                    v.tc0x = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[0][j].x;
                    v.tc0y = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[0][j].y;
                    v.tc0z = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[0][j].z;
                }
                // if (scene->mMeshes[node->mMeshes[i]]->HasTextureCoords(1))
                // {
                //     v.tc1x = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[1][j].x;
                //     v.tc1y = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[1][j].y;
                //     v.tc1z = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[1][j].z;
                // }
                // if (scene->mMeshes[node->mMeshes[i]]->HasTextureCoords(2))
                // {
                //     v.tc2x = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[2][j].x;
                //     v.tc2y = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[2][j].y;
                //     v.tc2z = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[2][j].z;
                // }
                // if (scene->mMeshes[node->mMeshes[i]]->HasTextureCoords(3))
                // {
                //     v.tc3x = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[3][j].x;
                //     v.tc3y = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[3][j].y;
                //     v.tc3z = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[3][j].z;
                // }
                // if (scene->mMeshes[node->mMeshes[i]]->HasTextureCoords(4))
                // {
                //     v.tc4x = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[4][j].x;
                //     v.tc4y = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[4][j].y;
                //     v.tc4z = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[4][j].z;
                // }
                // if (scene->mMeshes[node->mMeshes[i]]->HasTextureCoords(5))
                // {
                //     v.tc5x = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[5][j].x;
                //     v.tc5y = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[5][j].y;
                //     v.tc5z = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[5][j].z;
                // }
                // if (scene->mMeshes[node->mMeshes[i]]->HasTextureCoords(6))
                // {
                //     v.tc6x = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[6][j].x;
                //     v.tc6y = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[6][j].y;
                //     v.tc6z = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[6][j].z;
                // }
                // if (scene->mMeshes[node->mMeshes[i]]->HasTextureCoords(7))
                // {
                //     v.tc7x = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[7][j].x;
                //     v.tc7y = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[7][j].y;
                //     v.tc7z = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[7][j].z;
                // }
                mesh._vertices.push_back(v);
            }
            if (scene->mMeshes[node->mMeshes[i]]->HasFaces())
            {
                mesh._indices.reserve(scene->mMeshes[node->mMeshes[i]]->mNumFaces * 3);
                for (unsigned int j = 0; j < scene->mMeshes[node->mMeshes[i]]->mNumFaces; j++)
                {
                    if (scene->mMeshes[node->mMeshes[i]]->mFaces[j].mNumIndices == 3 &&
                        scene->mMeshes[node->mMeshes[i]]->mFaces[j].mIndices[0] < UINT16_MAX &&
                        scene->mMeshes[node->mMeshes[i]]->mFaces[j].mIndices[1] < UINT16_MAX &&
                        scene->mMeshes[node->mMeshes[i]]->mFaces[j].mIndices[2] < UINT16_MAX)
                    {
                        mesh._indices.push_back(scene->mMeshes[node->mMeshes[i]]->mFaces[j].mIndices[0]);
                        mesh._indices.push_back(scene->mMeshes[node->mMeshes[i]]->mFaces[j].mIndices[1]);
                        mesh._indices.push_back(scene->mMeshes[node->mMeshes[i]]->mFaces[j].mIndices[2]);
                    }
                }
            }
            scNode._meshes.push_back(std::move(mesh));
        }
        
        scNode._children.resize(node->mNumChildren);
        for (unsigned int i = 0; i < node->mNumChildren; i++)
            importRecursive(node->mChildren[i], scNode._children[i], importRecursive);
    };
    importRecursive(scene->mRootNode, ret->_root, importRecursive);

    ModelLoader::importer.FreeScene();

    return ret;
}