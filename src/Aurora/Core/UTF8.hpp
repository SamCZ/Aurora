#pragma once

namespace Aurora
{
    inline std::u8string CodepointToUtf8(uint32_t codepoint)
    {
        static_assert(sizeof(char8_t) == sizeof(uint8_t));
        if((codepoint & 0b0'00000'0000'0000'0111'1111u) == codepoint)
        {
            std::u8string utf8(1, '\0');
            utf8[0] = static_cast<char8_t>(codepoint); // 0xxx xxxx
            return utf8;
        }
        else if((codepoint & 0b0'00000'0000'0111'1111'1111u) == codepoint)
        {
            std::u8string utf8(2, '\0');
            utf8[0] = static_cast<char8_t>(static_cast<char8_t>((codepoint >>  6u) & 0b0001'1111) | 0b1100'0000u); // 110x xxxx
            utf8[1] = static_cast<char8_t>(static_cast<char8_t>((codepoint >>  0u) & 0b0011'1111) | 0b1000'0000u); // 10xx xxxx
            return utf8;
        }
        else if((codepoint & 0b0'00000'1111'1111'1111'1111u) == codepoint)
        {
            std::u8string utf8(3, '\0');
            utf8[0] = static_cast<char8_t>(static_cast<char8_t>((codepoint >> 12u) & 0b0000'1111) | 0b1110'0000u); // 1110 xxxx
            utf8[1] = static_cast<char8_t>(static_cast<char8_t>((codepoint >>  6u) & 0b0011'1111) | 0b1000'0000u); // 10xx xxxx
            utf8[2] = static_cast<char8_t>(static_cast<char8_t>((codepoint >>  0u) & 0b0011'1111) | 0b1000'0000u); // 10xx xxxx
            return utf8;
        }
        else if((codepoint & 0b1'1111'1111'1111'1111'1111u) == codepoint)
        {
            std::u8string utf8(4, '\0');
            utf8[0] = static_cast<char8_t>(static_cast<char8_t>((codepoint >> 18u) & 0b0000'0111) | 0b1111'0000u); // 1111 0xxx
            utf8[1] = static_cast<char8_t>(static_cast<char8_t>((codepoint >> 12u) & 0b0011'1111) | 0b1000'0000u); // 10xx xxxx
            utf8[2] = static_cast<char8_t>(static_cast<char8_t>((codepoint >>  6u) & 0b0011'1111) | 0b1000'0000u); // 10xx xxxx
            utf8[3] = static_cast<char8_t>(static_cast<char8_t>((codepoint >>  0u) & 0b0011'1111) | 0b1000'0000u); // 10xx xxxx
            return utf8;
        }
        else
        {
            AU_DEBUG_CERR("Unsupported codepoint " << codepoint);
            return {};
        }
    }
}