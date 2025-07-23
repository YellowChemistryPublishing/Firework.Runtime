#pragma once

#include <utility>

#define _fw_gl_common_handle_interface(T)           \
    T(std::nullptr_t) noexcept                      \
    { }                                             \
    T(const T&) = delete;                           \
    T(T&& other) noexcept                           \
    {                                               \
        swap(*this, other);                         \
    }                                               \
    ~T();                                           \
                                                    \
    T& operator=(const T&) = delete;                \
    T& operator=(T&& other) noexcept                \
    {                                               \
        swap(*this, other);                         \
        return *this;                               \
    }                                               \
                                                    \
    operator bool() const noexcept                  \
    {                                               \
        return bgfx::isValid(this->internalHandle); \
    }                                               \
                                                    \
    friend class Firework::GL::Renderer

#define _fw_gl_common_handle_swap(T)              \
    friend void swap(T& a, T& b) noexcept         \
    {                                             \
        using std::swap;                          \
                                                  \
        swap(a.internalHandle, b.internalHandle); \
    }

#define _fw_gl_common_handle_dtor(T)             \
    T::~T()                                      \
    {                                            \
        if (bgfx::isValid(this->internalHandle)) \
            bgfx::destroy(this->internalHandle); \
    }
