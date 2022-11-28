#pragma once

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/Library.hpp"

namespace Aurora
{
	typedef size_t GPUMemoryLocation;

	class AU_API GPUBlockAllocator
	{
	private:

	public:
		GPUBlockAllocator();
		~GPUBlockAllocator();

		uint8_t* Alloc(size_t size);
		bool Delete(uint8_t* memory);

		template<typename T>
		T* AllocType()
		{
			return reinterpret_cast<T*>(Alloc(sizeof(T)));
		}

		template<typename T>
		T* AllocTypeArray(size_t count)
		{
			return reinterpret_cast<T*>(Alloc(count * sizeof(T)));
		}
	};
}
