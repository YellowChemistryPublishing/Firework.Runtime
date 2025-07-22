#pragma once

#include <glm/vec3.hpp>

namespace Firework
{
    class LinAlgConstants final
    {
    public:
        LinAlgConstants() = delete;

        static const glm::vec3 forward;
        static const glm::vec3 up;
        static const glm::vec3 right;
    };

    constexpr inline glm::vec3 LinAlgConstants::forward { 0.0f, 0.0f, 1.0f };
    constexpr inline glm::vec3 LinAlgConstants::up { 0.0f, 1.0f, 0.0f };
    constexpr inline glm::vec3 LinAlgConstants::right { 1.0f, 0.0f, 0.0f };
} // namespace Firework
