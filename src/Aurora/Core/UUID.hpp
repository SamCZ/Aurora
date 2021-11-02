#pragma once

#include <cstdint>
#include <atomic>
#include <functional>

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

	class UUID64
	{
	private:
		uint64_t m_UUID;
	public:
		UUID64();
		UUID64(uint64_t uuid);
		UUID64(const UUID64& other);

		static UUID64 Generate();

		operator uint64_t () { return m_UUID; }
		operator const uint64_t () const { return m_UUID; }
	};

	class UUID32
	{
	private:
		uint32_t m_UUID;
	public:
		UUID32();
		UUID32(uint32_t uuid);
		UUID32(const UUID32& other);

		UUID32 Generate();

		operator uint32_t () { return m_UUID; }
		operator const uint32_t() const { return m_UUID; }
	};
}

namespace std
{
	template <>
	struct hash<Aurora::UUID64>
	{
		std::size_t operator()(const Aurora::UUID64& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};

	template <>
	struct hash<Aurora::UUID32>
	{
		std::size_t operator()(const Aurora::UUID32& uuid) const
		{
			return hash<uint32_t>()((uint32_t)uuid);
		}
	};
}