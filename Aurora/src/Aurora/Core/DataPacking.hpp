/*
 * Original code written by therealcain (https://github.com/therealcain)
 * Modified by SamCZ (https://github.com/SamCZ)
 */
#pragma once

#include <iostream>
#include <type_traits>
#include <memory>
#include <cstring>
#include <cstddef>
#include <utility>
#include <vector>

namespace Aurora
{
	template<size_t... Args>
	constexpr size_t StaticSum() {
		return (0 + ... + Args);
	}

	template<typename... Args>
	class MemoryPacker
	{
		static_assert(sizeof...(Args) > 0);

		// std::is_pod is deprecated.
		static_assert(std::conjunction_v< std::is_standard_layout<Args> ...>);
		static_assert(std::conjunction_v< std::is_trivially_copyable<Args> ...>);
		static_assert(std::conjunction_v< std::is_trivial<Args> ...>);

		static constexpr size_t DataSize = StaticSum<sizeof(Args)...>();

		std::vector<uint8_t> m_Data;
	public:
		MemoryPacker()
		{
			m_Data.resize(DataSize);
		}

		explicit MemoryPacker(std::vector<uint8_t> data) : m_Data(std::move(data)) {}
		MemoryPacker(uint8_t* data, size_t size)
		{
			m_Data.resize(size);
			std::memcpy(m_Data.data(), data, size);
		}

		void Pack(const Args& ... args)
		{
			size_t offset = 0;

			([&](const auto& val)
			{
				constexpr size_t val_size = sizeof(val);

				std::memcpy(m_Data.data() + offset, &val, val_size);
				offset += val_size;
			} (args), ...);
		}

		void Unpack(Args& ... args)
		{
			size_t offset = 0;

			([&](auto& val)
			{
				constexpr size_t val_size = sizeof(val);

				std::memcpy(&val, m_Data.data() + offset, val_size);
				offset += val_size;
			} (args), ...);
		}

		[[nodiscard]] inline const std::vector<uint8_t>& GetBuffer() const { return m_Data; }
		inline uint8_t* Data() { return m_Data.data(); }
		[[nodiscard]] inline size_t Size() const { return m_Data.size(); }
	};
}