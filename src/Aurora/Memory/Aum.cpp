#include "Aum.hpp"

#include <string.h>
#include "Aurora/Core/assert.hpp"
#include "Aurora/Core/String.hpp"
#include <iostream>

namespace Aurora
{
	Aum::Aum(MemSize blockSize) : m_BlockSize(blockSize)
	{
		AllocateMemoryBlock();
	}

	Aum::Aum(MemSize objectSize, MemSize objectCount) : m_BlockSize((objectSize + MemSizeOf) * objectCount)
	{
		AllocateMemoryBlock();
	}

	Aum::~Aum()
	{
		DestroyMemory();
	}

	void Aum::DestroyMemory()
	{
		for (const auto &item : m_Memory)
		{
			delete[] item.Memory;
		}

		m_Memory.clear();
	}

	Aum::MemoryBlock& Aum::AllocateMemoryBlock()
	{
		DestroyMemory();

		MemoryBlock memoryBlock;
		memoryBlock.Memory = new uint8_t[m_BlockSize];
		memoryBlock.Fragments.emplace_back(MemoryFragment{memoryBlock.Memory, memoryBlock.Memory + m_BlockSize, m_BlockSize});
		memoryBlock.FreeMemory = m_BlockSize;

		memset(memoryBlock.Memory, 0, m_BlockSize);

		return m_Memory.emplace_back(memoryBlock);
	}

	void* Aum::AllocFromFragment(MemoryBlock& memoryBlock, std::vector<MemoryFragment>::iterator fragmentIt, MemSize size)
	{
		MemoryFragment& fragment = *fragmentIt;

		MemPtr begin = fragment.Begin;

		// Store size of memory at the end of size uint32_t
		MemSize* newBlockSizePtr = reinterpret_cast<MemSize*>(begin);
		*newBlockSizePtr = size;

		// Get new memory start
		MemPtr newMemoryStart = begin + MemSizeOf;

		// Decrement block free size
		memoryBlock.FreeMemory -= size + MemSizeOf;

		// Change fragment size and if remaining is 0 then delete fragment
		fragment.Begin = newMemoryStart + size;
		fragment.Size -= size + MemSizeOf;
		au_assert(fragment.Size >= 0);
		if(fragment.Size == 0)
		{
			memoryBlock.Fragments.erase(fragmentIt);
		}

		// FIXME: when new block is allocated the memory is somehow invalid
		// TODO: Move the sizes to another struct to get rid of paddings

		// Return new memory
		return newMemoryStart;
	}

	void* Aum::Alloc(MemSize size)
	{
		au_assert(size);
		// Add sizeof uint32 for additional info
		size += MemSizeOf;
		au_assert(size <= m_BlockSize);

		if(!size)
		{
			return nullptr;
		}

		for (MemoryBlock& memoryBlock : m_Memory)
		{
			if(memoryBlock.FreeMemory < size)
			{
				continue;
			}

			// Check for memory fragments
			for(auto fragmentIt = memoryBlock.Fragments.begin(); fragmentIt != memoryBlock.Fragments.end(); ++fragmentIt)
			{
				if(fragmentIt->Size >= size)
				{
					return AllocFromFragment(memoryBlock, fragmentIt, size - MemSizeOf);
				}
			}
		}

		MemoryBlock& newBlock = AllocateMemoryBlock();
		return AllocFromFragment(newBlock, newBlock.Fragments.begin(), size - MemSizeOf);
	}

	void Aum::DeAlloc(void* mem)
	{
		if(!mem) return;

		MemPtr memPtrBegin = reinterpret_cast<MemPtr>(mem) - MemSizeOf;
		MemSize size = *reinterpret_cast<MemSize*>(memPtrBegin);
		MemPtr memPtrEnd = memPtrBegin + size;

		for (MemoryBlock& memoryBlock : m_Memory)
		{
			if(memPtrBegin >= memoryBlock.Memory && memPtrEnd <= (memoryBlock.Memory + m_BlockSize))
			{
				memoryBlock.Fragments.emplace_back(MemoryFragment{memPtrBegin, memPtrEnd, size});

				// TODO: merge blocks with same begin or end

				return;
			}
		}

		AU_LOG_FATAL("Memory ", PointerToString(mem), " is not part of this allocator !");
	}
}