#pragma once

#include <string>

namespace Firework
{
    struct [[fw::component]] EntityAttributes final
    {
        std::string name;
    };
} // namespace Firework