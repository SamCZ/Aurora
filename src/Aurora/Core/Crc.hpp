#pragma once

#ifdef _WIN32
#include <Windows.h>
#include <wrl.h>
#endif

#include <cstdint>

namespace Aurora
{
#ifdef _WIN32
	inline bool GetSSE42Support()
	{
		int cpui[4];
		__cpuidex(cpui, 1, 0);
		return !!(cpui[2] & 0x100000);
	}

	static const bool CpuSupportsSSE42 = GetSSE42Support();
#endif

	static uint64_t CrcTable[256];

	class CrcHash
	{
	private:
		uint64_t m_crc;
	public:
		inline CrcHash() : m_crc(0)
		{
			uint64_t poly = 0xC96C5795D7870F42;

			for(int i=0; i<256; ++i)
			{
				uint64_t crc = i;

				for(uint32_t j=0; j<8; ++j)
				{
					// is current coefficient set?
					if(crc & 1)
					{
						// yes, then assume it gets zero'd (by implied x^64 coefficient of dividend)
						crc >>= 1;

						// and add rest of the divisor
						crc ^= poly;
					}
					else
					{
						// no? then move to next coefficient
						crc >>= 1;
					}
				}

				CrcTable[i] = crc;
			}
		}

		inline uint64_t Get()
		{
			return m_crc;
		}
#ifdef _WIN32_DISABLED
		template<size_t size> __forceinline void AddBytesSSE42(void* p)
        {
            static_assert(size % 4 == 0, "Size of hashable types must be multiple of 4");

            auto* data = (uint32_t*)p;

            const size_t numIterations = size / sizeof(uint32_t);
            for (size_t i = 0; i < numIterations; i++)
            {
                crc = _mm_crc32_u32(crc, data[i]);
            }
        }
#endif
		inline void AddBytes(char* p, uint64_t size)
		{
			for (uint64_t idx = 0; idx < size; idx++)
			{
				uint8_t index = p[idx] ^ m_crc;
				uint64_t lookup = CrcTable[index];

				m_crc >>= 8;
				m_crc ^= lookup;
			}
		}

		template<typename T> void Add(const T& value)
		{
#ifdef _WIN32_DISABLED
			if (CpuSupportsSSE42)
                AddBytesSSE42<sizeof(value)>((void*)&value);
            else
                AddBytes((char*)&value, sizeof(value));
#else
			AddBytes((char*)&value, sizeof(value));
#endif
		}
	};
}