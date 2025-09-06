#include "CascadingStyleParser.h"

#include <algorithm>

using namespace Firework;

void CascadingStyleParser::ignoreWhitespace(const char*& it, const char* end)
{
    while (it != end && std::isspace(*it)) ++it;
}

bool CascadingStyleParser::parseBlock(const std::string_view block, std::map<std::string_view, std::string_view>& out)
{
    const char* it = std::to_address(block.begin());
    const char* const end = std::to_address(block.end());

    CascadingStyleParser::ignoreWhitespace(it, end); // Start ourselves at the first real character.

    while (it != end)
    {
        const char* delim = it;
        while (delim != end && *delim != ':')
        {
            _fence_value_return(false, *delim == '{' || *delim == '}' || *delim == ';');
            ++delim;
        }
        _fence_value_return(false, delim == end);

        const char* bound = delim;
        while (bound > it && std::isspace(*(bound - 1))) --bound;

        std::string_view property(it, bound);

        it = delim + 1;
        CascadingStyleParser::ignoreWhitespace(it, end);

        delim = it;
        i32 nestedLevel = 0;
        while (delim != end && nestedLevel == 0 && *delim != ';')
        {
            if (*delim == '{')
                ++nestedLevel;
            else if (*delim == '}')
                _fence_value_return(false, --nestedLevel < 0);
            else
                _fence_value_return(false, *delim == ':');
            ++delim;
        }

        bound = delim;
        while (bound > it && std::isspace(*(bound - 1))) --bound;

        out.emplace(property, std::string_view(it, bound));
        it = delim;
        if (it != end)
        {
            ++it;
            CascadingStyleParser::ignoreWhitespace(it, end);
        }
    }

    return true;
}
