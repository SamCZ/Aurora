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

	template<typename T>
	class TypeBase
	{
	private:
		UniqueIdHelper<T> m_UniqueId;
	public:
		[[nodiscard]] UniqueIdentifier GetUniqueID() const
		{
			// This unique ID is used to unambiguously identify the object for
			// tracking purposes.
			// Niether GL handle nor pointer could be safely used for this purpose
			// as both GL reuses released handles and the OS reuses released pointers
			return m_UniqueId.GetID();
		}

		static T* Null()
		{
			return nullptr;
		}
	};
}