#include "BufferCache.hpp"
#include "Base/IRenderDevice.hpp"
#include "Aurora/Core/String.hpp"

namespace Aurora
{

	BufferCache::BufferCache(IRenderDevice* renderDevice, EBufferType bufferType, uint numberOfBuffers, uint singleBufferSize, bool useGpuMapping)
			: m_RenderDevice(renderDevice),
			m_NumBuffers(numberOfBuffers),
			m_Buffers(numberOfBuffers),
			m_SingleBufferSize(singleBufferSize),
			m_CurrentOffset(0),
			m_CurrentBuffer(0),
			m_NumBytesPerFrame(0),
			m_UseGpuMapping(useGpuMapping)
	{
		for (int i = 0; i < numberOfBuffers; ++i)
		{
			m_Buffers[i] = renderDevice->CreateBuffer(BufferDesc("BufferCache #" + std::to_string(i), singleBufferSize, bufferType));
		}

		if(!m_UseGpuMapping)
		{
			m_LocalMappedMemory = new uint8_t[numberOfBuffers * singleBufferSize];
		}
	}

	BufferCache::~BufferCache()
	{
		delete[] m_LocalMappedMemory;
	}

	uint8* BufferCache::GetOrMap(uint size, VBufferCacheIndex &bufferCacheIndex)
	{
		if(size > m_SingleBufferSize)
		{
			AU_LOG_WARNING("Cannot write ", FormatBytes(size), " to cache with max size ", FormatBytes(m_SingleBufferSize), " !");
			return nullptr;
		}

		uint freeSpace = m_SingleBufferSize - m_CurrentOffset;

		if(size <= freeSpace && m_CurrentBuffer < m_NumBuffers)
		{ // Free space, get memory and advance
			bufferCacheIndex.Offset = m_CurrentOffset;
			bufferCacheIndex.BufferIndex = m_CurrentBuffer;
			bufferCacheIndex.Size = size;
			bufferCacheIndex.Buffer = m_Buffers[bufferCacheIndex.BufferIndex];

			m_CurrentOffset += std::max(size, 256u);

			if(freeSpace == size)
			{ // If we fill whole buffer we advance to another
				m_CurrentOffset = 0;
				m_CurrentBuffer += 1;
			}

			m_NumBytesPerFrame += size;

			size_t offset = bufferCacheIndex.BufferIndex * m_SingleBufferSize + bufferCacheIndex.Offset;
			if(m_UseGpuMapping)
			{
				return reinterpret_cast<uint8_t*>(m_RenderDevice->MapBuffer(bufferCacheIndex.Buffer, EBufferAccess::WriteOnly)) + offset;
			}
			else
			{
				return &m_LocalMappedMemory[offset];
			}
		}
		else if(m_CurrentBuffer < m_NumBuffers - 1)
		{ // No free space, need to cache remaining and advance buffer
			AU_LOG_WARNING(FormatBytes(freeSpace), " bytes was skipped at the end of BufferCache !");
			m_CurrentOffset = std::max(size, 256u);
			m_CurrentBuffer += 1;

			bufferCacheIndex.Offset = m_CurrentOffset;
			bufferCacheIndex.BufferIndex = m_CurrentBuffer;
			bufferCacheIndex.Size = size;
			bufferCacheIndex.Buffer = m_Buffers[bufferCacheIndex.BufferIndex];

			m_NumBytesPerFrame += size;

			return m_RenderDevice->MapBuffer(bufferCacheIndex.Buffer, EBufferAccess::WriteOnly);
		}
		else
		{ // We are at the end of max memory, throw an error
			AU_LOG_WARNING("BufferCache memory at maximum ", FormatBytes(m_NumBuffers * m_SingleBufferSize), " !");
		}

		return nullptr;
	}

	void BufferCache::Unmap(VBufferCacheIndex &bufferCacheIndex)
	{
		if(bufferCacheIndex.Buffer == nullptr)
		{
			return;
		}

		if(m_UseGpuMapping)
		{
			m_RenderDevice->UnmapBuffer(bufferCacheIndex.Buffer);
		}
		else
		{
			size_t offset = bufferCacheIndex.BufferIndex * m_SingleBufferSize + bufferCacheIndex.Offset;
			m_RenderDevice->WriteBuffer(bufferCacheIndex.Buffer, &m_LocalMappedMemory[offset], bufferCacheIndex.Size, offset);
		}
	}
}