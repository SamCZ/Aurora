#pragma once

#include <cstdint>
#include <atomic>

namespace Aurora
{
	using UniqueIdentifier = int32_t;

	template <typename T>
	class UniqueIdHelper
	{
	private:
		mutable UniqueIdentifier m_ID = 0;
	public:
		UniqueIdHelper() noexcept = default;

		UniqueIdHelper             (const UniqueIdHelper&) = delete;
		UniqueIdHelper& operator = (const UniqueIdHelper&) = delete;

		UniqueIdHelper(UniqueIdHelper&& other) noexcept : m_ID(other.m_ID)
		{
			other.m_ID = 0;
		}

		UniqueIdHelper& operator=(UniqueIdHelper&& other) noexcept
		{
			m_ID     = other.m_ID;
			other.m_ID = 0;
			return *this;
		}

		UniqueIdentifier GetID() const noexcept
		{
			if (m_ID == 0)
			{
				static std::atomic_int32_t GlobalCounter{0};
				m_ID = GlobalCounter.fetch_add(1) + 1;
			}
			return m_ID;
		}
	};
}