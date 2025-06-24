#pragma once

#include "Firework.Runtime.GL.Exports.h"
#include <GL.Config.h>

#include <module/sys.Mathematics>

#include <GL/Texture.h>

namespace Firework::GL
{
    template <uint16_t Vec4Count>
    struct TextureVectorElement
    {
        sysm::vector4 elementData[Vec4Count];
    };

    template <uint16_t Vec4Count>
    requires requires { Vec4Count > 0; }
    class TextureVector final
    {
        std::vector<TextureVectorElement<Vec4Count>> cpuData;

        Texture2DHandle gpuData;
        bgfx::UniformHandle textureInfo;
        uint16_t size, capacity;
    public:
        using ValueType = TextureVectorElement<Vec4Count>;

        struct ElementQuery
        {
            inline const ValueType& operator*()
            {
                return this->vec[this->index];
            }
            inline ElementQuery& operator=(const ValueType& value)
            {
                this->vec.cpuData[this->index] = value;
                this->vec.gpuData.updateDynamic(&this->vec.cpuData[this->index], sizeof(ValueType), 0, 0, 0, this->index, Vec4Count, 1);

                return *this;
            }

            ElementQuery(const ElementQuery&) = delete;
            ElementQuery(ElementQuery&&) = delete;
            ElementQuery& operator=(const ElementQuery&) = delete;
            ElementQuery& operator=(ElementQuery&&) = delete;

            friend class Firework::GL::TextureVector<Vec4Count>;
        private:
            TextureVector& vec;
            uint16_t index;

            inline ElementQuery(TextureVector& vec, uint16_t index) : vec(vec), index(index)
            { }
        };

        inline TextureVector() : size(0), capacity(GL_TEXTURE_VECTOR_MIN_CAPACITY)
        {
            this->gpuData = Texture2DHandle::createDynamic
            (
                Vec4Count, GL_TEXTURE_VECTOR_MIN_CAPACITY, false, 1, bgfx::TextureFormat::RGBA32F,
                BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT |
                BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
            );

            this->cpuData.reserve(GL_TEXTURE_VECTOR_MIN_CAPACITY);
        }
        inline TextureVector(const TextureVector& other)
        {
            // TODO: Implement
        }
        inline TextureVector(TextureVector&& other) : cpuData(std::move(other.cpuData)), gpuData(other.gpuData)
        {
            other.gpuData = nullptr;
        }
        inline ~TextureVector()
        {
            this->gpuData.destroy();
        }

        inline TextureVector& operator=(const TextureVector& other)
        {
            // TODO: Implement
        }
        inline TextureVector& operator=(TextureVector&& other)
        {
            this->cpuData = std::move(other.cpuData);
            this->gpuData = other.gpuData;
            other.gpuData = nullptr;
        }

        inline ElementQuery operator[](uint16_t index)
        {
            return ElementQuery(*this, index);
        }

        inline void shrinkToFit()
        {
            // TODO: Implement
        }
        inline void clear()
        {
            // TODO: Implement
        }

        inline void reserve()
        {
            // TODO: Implement
        }
        inline void resize()
        {
            // TODO: Implement
        }

        inline void insert()
        {
            // TODO: Implement
        }
        inline void erase()
        {
            // TODO: Implement
        }

        inline void pushBack(const ValueType& element)
        {
            if (this->size == this->capacity)
            {
                Texture2DHandle newData = Texture2DHandle::createDynamic
                (
                    Vec4Count, this->capacity * 2, false, 1, bgfx::TextureFormat::RGBA32F,
                    BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT |
                    BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
                );
                // This doesn't work when an ```Texture2DHandle::updateDynamic``` call comes before.
                // this->gpuData.copyTo(GL_TEXTURE_VECTOR_RESERVED_VIEW, newData, 0, 0, 0, 0, Vec4Count, this->capacity);
                newData.updateDynamic(this->cpuData.data(), this->cpuData.size() * sizeof(ValueType), 0, 0, 0, 0, Vec4Count, this->cpuData.size());
                this->gpuData.destroy();
                this->gpuData = newData;
                
                this->cpuData.reserve(this->capacity * 2);

                this->capacity *= 2;
            }

            this->gpuData.updateDynamic(&element, sizeof(ValueType), 0, 0, 0, this->size, Vec4Count, 1);
            this->cpuData.push_back(element);
            ++this->size;
        }
        inline void popBack()
        {
            if (this->size == this->capacity / 2)
            {
                uint16_t newCapacity = std::max<uint16_t>(this->capacity / 2, GL_TEXTURE_VECTOR_MIN_CAPACITY);
                Texture2DHandle newData = Texture2DHandle::createDynamic
                (
                    Vec4Count, newCapacity, false, 1, bgfx::TextureFormat::RGBA32F,
                    BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT |
                    BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
                );
                // This doesn't work when an ```Texture2DHandle::updateDynamic``` call comes before.
                // this->gpuData.copyTo(GL_TEXTURE_VECTOR_RESERVED_VIEW, newData, 0, 0, 0, 0, Vec4Count, newCapacity);
                newData.updateDynamic(this->cpuData.data(), this->cpuData.size() * sizeof(ValueType), 0, 0, 0, 0, Vec4Count, this->cpuData.size());
                this->gpuData.destroy();
                this->gpuData = newData;
                
                this->capacity = newCapacity;
            }

            this->cpuData.pop_back();
            --this->size;
        }

        friend class Firework::GL::Renderer;
    };
}