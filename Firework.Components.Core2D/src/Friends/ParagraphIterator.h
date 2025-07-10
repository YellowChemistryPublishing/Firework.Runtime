#pragma once

#include <concepts>
#include <string>
#include <utility>

namespace Firework
{
    template <std::integral CharType>
    struct ParagraphIterator
    {
        using StringIterator = std::basic_string<CharType>::const_iterator;

        static ParagraphIterator begin(const std::basic_string<CharType>& str)
        {
            IteratorTriplet ret = ParagraphIterator::createFromBegin(str, str.begin());
            return ParagraphIterator(str, ret.tBeg, ret.sBeg, ret.end);
        }
        static ParagraphIterator end(const std::basic_string<CharType>& str)
        {
            return ParagraphIterator(str, str.end(), str.end(), str.end());
        }

        ParagraphIterator() = default;

        friend bool operator==(const ParagraphIterator&, const ParagraphIterator&) = default;

        StringIterator textBegin()
        {
            return this->tBeg;
        }
        StringIterator textEnd()
        {
            return this->sBeg;
        }
        StringIterator spaceBegin()
        {
            return this->sBeg;
        }
        StringIterator spaceEnd()
        {
            return this->sEnd;
        }

        ParagraphIterator& operator++()
        {
            IteratorTriplet ret = ParagraphIterator::createFromBegin(*this->str, this->sEnd);
            this->tBeg = ret.tBeg;
            this->sBeg = ret.sBeg;
            this->sEnd = ret.end;
            return *this;
        }
        ParagraphIterator operator++(int)
        {
            ParagraphIterator ret = *this;
            ++*this;
            return ret;
        }
    private:
        struct IteratorTriplet
        {
            StringIterator tBeg, sBeg, end;
        };
        static IteratorTriplet createFromBegin(const std::basic_string<CharType>& str, StringIterator beg)
        {
            auto tBeg = beg;

            auto sBeg = tBeg;
            while (sBeg != str.end() && !std::isspace(*sBeg)) ++sBeg;

            auto end = sBeg;
            while (end != str.end() && std::isspace(*end)) ++end;

            return IteratorTriplet { tBeg, sBeg, end };
        }

        ParagraphIterator(const std::basic_string<CharType>& str, StringIterator tBeg, StringIterator sBeg, StringIterator end) : str(&str), tBeg(tBeg), sBeg(sBeg), sEnd(end)
        { }

        const std::basic_string<CharType>* str;
        StringIterator tBeg;
        StringIterator sBeg;
        StringIterator sEnd;
    };
} // namespace Firework
