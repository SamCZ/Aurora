#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include "Aurora/Core/Library.hpp"
#include "Aurora/Tools/robin_hood.h"

namespace Aurora
{
	typedef uint8_t* MemPtr;
	typedef uint32_t MemSize;
	static constexpr MemSize MemSizeOf = sizeof(MemSize);

	template<typename T>
	constexpr T Align(T val, uint64_t alignment)
	{
		uint64_t mask = ~(alignment - 1);
		uint64_t final = ((uint64_t)val + alignment - 1) & mask;
		return (T)final;
	}

	template<typename T>
	constexpr T IsAligned(T val, uint64_t alignment)
	{
		return !((uint64_t)val & (alignment - 1));
	}

	class AU_API Aum
	{
		struct MemoryFragment
		{
			MemPtr Begin;
			MemPtr End;
			MemSize Size;
		};

		struct MemoryBlock
		{
			MemPtr Memory;
			std::vector<MemoryFragment> Fragments; // TODO: Change to binary tree for memory size optimizations
			MemSize FreeMemory;
		};

	private:
		MemSize m_BlockSize;
		std::vector<MemoryBlock> m_Memory;
		robin_hood::unordered_map<uintptr_t, MemSize> m_MemorySizes;
	public:
		explicit Aum(MemSize blockSize = 8388608); // 8MB Default block
		Aum(MemSize objectSize, MemSize objectCount); // 8MB Default block
		~Aum();

		Aum(const Aum& left) = delete;
		Aum & operator=(const Aum&) = delete;

		MemPtr Alloc(MemSize size);

		template<typename T>
		T* Alloc(MemSize count = 1)
		{
			return reinterpret_cast<T*>(Alloc(sizeof(T) * count));
		}

		template<typename T, typename... Args>
		T* AllocAndInit(MemSize count = 1, Args&&... args)
		{
			T* type = reinterpret_cast<T*>(Alloc(sizeof(T) * count));

			for (int i = 0; i < count; ++i)
			{
				new (type + i) T(std::forward<Args>(args)...);
			}

			return type;
		}

		void DeAlloc(void* mem);

		template<typename T>
		void DeAllocAndUnload(void* mem)
		{
			T* type = reinterpret_cast<T*>(mem);
			type->~T();
			DeAlloc(mem);
		}

		[[nodiscard]] MemSize GetMemoryBlockCount() const
		{
			return m_Memory.size();
		}
		MemPtr GetMemoryBlockPtr(MemSize index)
		{
			return m_Memory[index].Memory;
		}
		[[nodiscard]] MemSize GetMemoryBlockSize() const
		{
			return m_BlockSize;
		}

	private:
		MemoryBlock& AllocateMemoryBlock();
		void DestroyMemory();
		MemPtr AllocFromFragment(MemoryBlock& memoryBlock, std::vector<MemoryFragment>::iterator framentIt, MemSize size);
	};
}