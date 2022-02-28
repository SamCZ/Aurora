#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <utility>

#include "TypeBase.hpp"

namespace Aurora
{
	enum class EIndexBufferFormat : uint8_t
	{
		Uint8,
		Uint16,
		Uint32
	};

	enum class EBufferType : uint8_t
	{
		VertexBuffer,
		IndexBuffer,
		UniformBuffer,
		ShaderStorageBuffer,
		TextureBuffer,
		IndirectBuffer,
		Unknown
	};

	enum class EBufferUsage : uint8_t
	{
		StreamDraw,
		StreamRead,
		StreamCopy,

		StaticDraw,
		StaticRead,
		StaticCopy,

		DynamicDraw,
		DynamicRead,
		DynamicCopy,

		Unknown
	};

	enum class EBufferOp : uint8_t
	{
		Draw,
		Read,
		Copy
	};

	enum class EBufferAccess : uint8_t
	{
		ReadOnly = 0,
		WriteOnly,
		ReadWrite
	};

	enum EBufferFlags : uint8_t
	{
		BF_DYNAMIC_STORAGE	= 1 << 0,
		BF_MAP_READ			= 1 << 1,
		BF_MAP_WRITE		= 1 << 2,
		BF_MAP_PERSISTENT	= 1 << 3,
		BF_MAP_COHERENT		= 1 << 4,
		BF_CLIENT_STORAGE	= 1 << 5
	};

	struct BufferDesc
	{
		std::string Name;
		uint32_t ByteSize;
		EBufferType Type;
		EBufferUsage Usage;
		uint8_t Flags; // TODO: Fix flags, buffer is write only now !
		bool IsDMA : 1; // IMPORTANT! If you set this you need to make barrier after writing to the buffer !

		BufferDesc()
				: ByteSize(0),
				  Name("Undefined"),
				  Type(EBufferType::Unknown),
				  Usage(EBufferUsage::Unknown),
				  Flags(BF_MAP_WRITE), IsDMA(false) {}

		BufferDesc(std::string name, uint32_t size, EBufferType type, EBufferUsage usage = EBufferUsage::DynamicDraw, bool dma = false)
				: ByteSize(size),
				  Name(std::move(name)),
				  Type(type),
				  Flags(0),
				  Usage(usage), IsDMA(dma) {}
	};

	class IBuffer : public TypeBase<IBuffer>
	{
	public:
		virtual ~IBuffer() = default;

		[[nodiscard]] virtual const BufferDesc& GetDesc() const noexcept = 0;
	};

	typedef std::shared_ptr<IBuffer> Buffer_ptr;
}