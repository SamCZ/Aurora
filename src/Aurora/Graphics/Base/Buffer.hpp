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

	struct BufferDesc
	{
		std::string Name;
		uint32_t ByteSize;
		uint32_t Stride; //if non-zero it's structured
		EBufferType Type;
		EBufferUsage Usage;

		BufferDesc()
				: ByteSize(0),
				  Stride(0),
				  Name("Undefined"),
				  Type(EBufferType::Unknown),
				  Usage(EBufferUsage::Unknown) {}

		BufferDesc(std::string name, uint32_t size, uint32_t stride, EBufferType type, EBufferUsage usage = EBufferUsage::DynamicDraw)
				: ByteSize(size),
				  Stride(stride),
				  Name(std::move(name)),
				  Type(type),
				  Usage(usage) {}
	};

	class IBuffer : public TypeBase<IBuffer>
	{
	public:
		virtual ~IBuffer() = default;

		[[nodiscard]] virtual const BufferDesc& GetDesc() const noexcept = 0;
	};

	typedef std::shared_ptr<IBuffer> Buffer_ptr;
}