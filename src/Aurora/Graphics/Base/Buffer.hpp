#pragma once

#include <cstdint>
#include <string>
#include <memory>

namespace Aurora
{
	enum class IndexBufferFormat : uint8_t
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

	struct BufferDesc
	{
		std::string Name;
		uint32_t ByteSize;
		uint32_t StructStride; //if non-zero it's structured
		EBufferType Type;
		EBufferUsage Usage;

		BufferDesc()
				: ByteSize(0),
				  StructStride(0),
				  Name("Undefined"),
				  Type(EBufferType::Unknown),
				  Usage(EBufferUsage::Unknown) {}
	};

	class IBuffer
	{
	public:
		virtual ~IBuffer() = default;

		[[nodiscard]] virtual const BufferDesc& GetDesc() const noexcept = 0;
	};

	typedef std::shared_ptr<IBuffer> Buffer_ptr;
}