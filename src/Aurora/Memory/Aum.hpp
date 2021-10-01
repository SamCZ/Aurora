#pragma once

#include <cstdint>
#include <utility>
#include <vector>

namespace Aurora
{
	typedef uint8_t* MemPtr;
	typedef uint32_t MemSize;
	static constexpr MemSize MemSizeOf = sizeof(MemSize);

	class Aum
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
	public:
		Aum(MemSize blockSize = 8388608); // 8MB Default block
		~Aum();

		void* Alloc(MemSize size);

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

	private:
		MemoryBlock& AllocateMemoryBlock();
		void* AllocFromFragment(MemoryBlock& memoryBlock, std::vector<MemoryFragment>::iterator framentIt, MemSize size);
	};
}