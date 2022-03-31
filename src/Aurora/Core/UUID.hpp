#pragma once

// Based on stduuid from github

#include <iostream>
#include <cstdint>
#include <atomic>
#include <string>
#include <functional>
#include <optional>
#include "Library.hpp"

namespace Aurora
{
	namespace uuid::detail
	{
		template<typename CharT>
		inline constexpr CharT guid_encoder[17] = "0123456789abcdef";

		template<>
		inline constexpr wchar_t guid_encoder<wchar_t>[17] = L"0123456789abcdef";

		template<typename CharT>
		inline constexpr CharT empty_guid[37] = "00000000-0000-0000-0000-000000000000";

		template<>
		inline constexpr wchar_t empty_guid<wchar_t>[37] = L"00000000-0000-0000-0000-000000000000";

		template<typename StringType>
		constexpr std::basic_string_view<typename StringType::value_type, typename StringType::traits_type>
		ToStringView(const StringType& str)
		{
				return str;
		}

		template<typename CharT>
		constexpr inline uint8_t HexToChar(CharT const ch)
		{
			if (ch >= static_cast<CharT>('0') && ch <= static_cast<CharT>('9'))
				return static_cast<uint8_t>(ch - static_cast<CharT>('0'));

			if (ch >= static_cast<CharT>('a') && ch <= static_cast<CharT>('f'))
				return static_cast<uint8_t>(10 + ch - static_cast<CharT>('a'));

			if (ch >= static_cast<CharT>('A') && ch <= static_cast<CharT>('F'))
				return static_cast<uint8_t>(10 + ch - static_cast<CharT>('A'));

			return 0;
		}

		template<typename CharT>
		constexpr inline uint8_t IsHex(CharT const ch)
		{
			return
				(ch >= static_cast<CharT>('0') && ch <= static_cast<CharT>('9')) ||
				(ch >= static_cast<CharT>('a') && ch <= static_cast<CharT>('f')) ||
				(ch >= static_cast<CharT>('A') && ch <= static_cast<CharT>('F'));
		}
	}

	class AU_API UUID
	{
	private:
		uint64_t m_Low;
		uint64_t m_High;
	public:
		UUID() : m_Low(0), m_High(0) {}
		constexpr UUID(uint64_t low, uint64_t high) : m_Low(low), m_High(high) {}
		UUID(const UUID& other)  = default;

		static UUID Generate();

		[[nodiscard]] uint64_t Low() const { return m_Low; }
		[[nodiscard]] uint64_t High() const { return m_High; }

		[[nodiscard]] inline constexpr bool Zero() const
		{
			return m_Low == 0 && m_High == 0;
		}

		[[nodiscard]] constexpr int Compare(const UUID& other) const
		{
			if (m_Low < other.m_Low)
				return -1;
			if (m_Low > other.m_Low)
				return 1;

			if (m_High < other.m_High)
				return -1;
			if (m_High > other.m_High)
				return 1;

			return 0;
		}

		constexpr bool operator>(const UUID& other) const
		{
			return Compare(other) > 0;
		}

		constexpr bool operator<(const UUID& other) const
		{
			return Compare(other) < 0;
		}

		constexpr bool operator==(const UUID& other) const
		{
			return m_Low == other.m_Low && m_High == other.m_High;
		}

		constexpr bool operator!=(const UUID& other) const
		{
			return !operator==(other);
		}

		constexpr operator bool() const
		{
			return !Zero();
		}

		template<typename StringType>
		constexpr static bool IsValid(const StringType& in_str) noexcept
		{
			auto str = uuid::detail::ToStringView(in_str);

			bool firstDigit = true;
			size_t hasBraces = 0;
			size_t index = 0;

			if (str.empty())
				return false;

			if (str.front() == '{')
				hasBraces = 1;
			if (hasBraces && str.back() != '}')
				return false;

			for (size_t i = hasBraces; i < str.size() - hasBraces; ++i)
			{
				if (str[i] == '-')
					continue;

				if(index >= 16 || !uuid::detail::IsHex(str[i]))
					return false;

				if (firstDigit)
				{
					firstDigit = false;
				}
				else
				{
					index++;
					firstDigit = true;
				}
			}

			if(index < 16)
				return false;

			return true;
		}

		template<typename StringType>
		constexpr static std::optional<UUID> FromString(const StringType& in_str) noexcept
		{
			auto str = uuid::detail::ToStringView(in_str);

			bool firstDigit = true;
			size_t hasBraces = 0;
			size_t index = 0;

			std::array<uint8_t, 16> data = { {0} };

			if (str.empty())
				return {};

			if (str.front() == '{')
				hasBraces = 1;
			if (hasBraces && str.back() != '}')
				return {};

			for (size_t i = hasBraces; i < str.size() - hasBraces; ++i)
			{
				if (str[i] == '-')
					continue;

				if (index >= 16 || !uuid::detail::IsHex(str[i]))
					return {};

				if (firstDigit)
				{
					data[index] = static_cast<uint8_t>(uuid::detail::HexToChar(str[i]) << 4);
					firstDigit = false;
				}
				else
				{
					data[index] = static_cast<uint8_t>(data[index] | uuid::detail::HexToChar(str[i]));
					index++;
					firstDigit = true;
				}
			}

			if(index < 16)
				return {};

			uint64_t low = 0;
			uint64_t high = 0;
			(void)std::memcmp(&low, data.data(), 8);
			(void)std::memcmp(&high, data.data() + 8, 8);

			return UUID(low, high);
		}

		template<typename CharT>
		constexpr std::basic_string<CharT> ToString() const
		{
			std::basic_string<CharT> uustr = {uuid::detail::empty_guid<CharT>};

			const auto* data = reinterpret_cast<const uint8_t*>(&m_Low);

			for (size_t i = 0, index = 0; i < 36; ++i)
			{
				if (i == 8 || i == 13 || i == 18 || i == 23)
				{
					continue;
				}

				uustr[i] = uuid::detail::guid_encoder<CharT>[(data[index] >> 4) & 0x0f];
				uustr[++i] = uuid::detail::guid_encoder<CharT>[data[index] & 0x0f];
				index++;
			}

			return uustr;
		}

		template<typename CharT>
		explicit constexpr operator std::basic_string<CharT>() const
		{
			return ToString<CharT>();
		}
	};
}
