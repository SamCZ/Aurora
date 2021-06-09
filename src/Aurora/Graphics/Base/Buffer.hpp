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

	struct BufferDesc
	{
		enum class EUsage : uint8_t
		{
			Stream,
			Static,
			Dynamic
		};

		// Notice that there are no atomic/append/consume buffer-related things here.
		// We should use another UAV of uints instead since you can do that in both DX and GL
		uint32_t ByteSize;
		uint32_t StructStride; //if non-zero it's structured
		std::string DebugName;
		bool CanHaveUAVs;
		bool IsVertexBuffer;
		bool IsIndexBuffer;
		bool IsCPUWritable;
		bool IsDrawIndirectArgs;
		bool DisableGPUsSync;
		EUsage Usage;

		BufferDesc()
				: ByteSize(0),
				  StructStride(0),
				  DebugName(),
				  CanHaveUAVs(false),
				  IsVertexBuffer(false),
				  IsIndexBuffer(false),
				  IsCPUWritable(false),
				  IsDrawIndirectArgs(false),
				  DisableGPUsSync(false) {}
	};

	class IBuffer
	{
	public:
		virtual ~IBuffer() = default;

		[[nodiscard]] virtual const BufferDesc& GetDesc() const noexcept = 0;
	};

	typedef std::shared_ptr<IBuffer> Buffer_ptr;
}