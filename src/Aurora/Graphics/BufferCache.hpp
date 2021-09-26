#pragma once

#include "Aurora/Core/Types.hpp"
#include "Aurora/Graphics/Base/Buffer.hpp"

namespace Aurora
{
	class IRenderDevice;

	struct VBufferCacheIndex
	{
		uint Offset;
		uint Size;
		uint BufferIndex;
		Buffer_ptr Buffer;

		VBufferCacheIndex() : Offset(0), Size(0), BufferIndex(0), Buffer(nullptr)
		{}
	};

	class BufferCache
	{
	private:
		IRenderDevice* m_RenderDevice;
		std::vector<Buffer_ptr> m_Buffers;
		uint m_NumBuffers;
		uint m_SingleBufferSize;
		uint m_CurrentOffset;
		uint m_CurrentBuffer;
		uint m_NumBytesPerFrame;
		bool m_UseGpuMapping;
		uint8_t* m_LocalMappedMemory;
	public:
		BufferCache(IRenderDevice* renderDevice, EBufferType bufferType, uint numberOfBuffers, uint singleBufferSize, bool useGpuMapping);
		~BufferCache();

		void* GetOrMap(uint size, VBufferCacheIndex& bufferCacheIndex);

		template<typename T>
		T* GetOrMap(uint size, VBufferCacheIndex& bufferCacheIndex)
		{
			return reinterpret_cast<T*>(GetOrMap(size, bufferCacheIndex));
		}

		void Unmap(VBufferCacheIndex& bufferCacheIndex);

		void Reset()
		{
			m_CurrentOffset = 0;
			m_CurrentBuffer = 0;
			m_NumBytesPerFrame = 0;
		}
	};
}