#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <map>

namespace Aurora
{
	enum class ShaderResourceType
	{
		/// Shader resource type is unknown
		Unknown = 0,

		/// Constant (uniform) buffer
		ConstantBuffer,

		/// Shader resource view of a texture (sampled image)
		TextureSRV,

		/// Shader resource view of a buffer (read-only storage image)
		BufferSRV,

		/// Unordered access view of a texture (sotrage image)
		TextureUAV,

		/// Unordered access view of a buffer (storage buffer)
		BufferUAV,

		/// Sampler (separate sampler)
		Sampler,

		/// Input attachment in a render pass
		InputAttachment,

		/// Acceleration structure
		AccelStruct,
	};

	enum class ShaderType
	{
		Vertex,
		Hull,
		Domain,
		Geometry,
		Pixel,
		Compute
	};

	struct ShaderVariable
	{
		std::string Name;
		size_t      Size;
		size_t      Offset;
		size_t      ArrayStride;
		size_t      MatrixStride;

		ShaderVariable() noexcept :
				Name(),
				Size(0),
				Offset(0),
				ArrayStride(0),
				MatrixStride(0) {}

		ShaderVariable(std::string name, size_t size, size_t offset, size_t arrayStride, size_t matrixStride) noexcept :
				Name(std::move(name)),
				Size(size),
				Offset(offset),
				ArrayStride(arrayStride),
				MatrixStride(matrixStride) {}
	};

	struct ShaderResourceDesc
	{
		std::string Name;

		/// Shader resource type, see Diligent::SHADER_RESOURCE_TYPE.
		ShaderResourceType Type = ShaderResourceType::Unknown;

		/// Array size. For non-array resource this value is 1.
		uint32_t ArraySize = 0;

		/// Size of resource
		uint32_t Size = 0;

		std::vector<ShaderVariable> Variables{};
	};

	typedef std::map<std::string, std::string> ShaderMacro;

	struct ShaderDesc
	{
		ShaderType Type;

		explicit ShaderDesc(ShaderType type)
				: Type(type)
		{ }
	};

	class ShaderBase
	{
	public:
		[[nodiscard]] virtual const ShaderDesc& GetDesc() const = 0;
		[[nodiscard]] virtual std::vector<ShaderResourceDesc> GetResources(const ShaderResourceType& resourceType) = 0;
	};

	typedef std::shared_ptr<ShaderBase> Shader_ptr;
}