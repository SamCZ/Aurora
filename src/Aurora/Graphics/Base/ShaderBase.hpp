#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <map>

#include "Aurora/Logger/Logger.hpp"

#include "InputLayout.hpp"
#include "TypeBase.hpp"

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

	enum class EShaderType : uint8_t
	{
		Vertex = 0,
		Hull,
		Domain,
		Geometry,
		Pixel,
		Compute,
		Unknown
	};
	static std::string ShaderType_ToString(const EShaderType& shaderType)
	{
		static std::string types[] = {
				"Vertex", "Hull", "Domain", "Geometry", "Pixel", "Compute", "Unknown"
		};

		return types[static_cast<uint8_t>(shaderType)];
	}

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

	typedef std::map<std::string, std::string> ShaderMacros;

	struct ShaderDesc
	{
		EShaderType Type;
		std::string Source;
		ShaderMacros Macros;

		ShaderDesc() : Type(EShaderType::Unknown), Source(), Macros()
		{

		}

		explicit ShaderDesc(EShaderType type, std::string source, ShaderMacros macros)
				: Type(type), Source(std::move(source)), Macros(std::move(macros)) { }
	};

	class ShaderProgramDesc
	{
	public:
		typedef std::function<void(const std::string&)> ErrorFnc;
	private:
		std::string Name;
		std::map<EShaderType, ShaderDesc> ShaderDescriptions;
		ErrorFnc ErrorOutput;
	public:
		explicit ShaderProgramDesc(std::string name) : Name(std::move(name)), ShaderDescriptions(), ErrorOutput(nullptr) {}
	public:
		/**
		 * This will set the error output of the shader compilation.
		 * @param output pointer of std::string for the messages
		 */
		inline void SetErrorOutput(const ErrorFnc& output) { ErrorOutput = output; }
		[[nodiscard]] inline bool HasSetErrorOutput() const noexcept { return ErrorOutput != nullptr; }
		[[nodiscard]] inline const ErrorFnc& GetErrorOutput() const { return ErrorOutput; }
	public:
		inline void AddShader(const ShaderDesc& shaderDesc)
		{
			if(shaderDesc.Type == EShaderType::Unknown)
			{
				AU_LOG_WARNING("Cannot add Unknown shader type to ", Name, " ! Skipping...")
				return;
			}

			if(ShaderDescriptions.contains(shaderDesc.Type)) {
				AU_LOG_WARNING("Shader ", ShaderType_ToString(shaderDesc.Type), " already exists in program ", Name, " ! Skipping...")
				return;
			}

			ShaderDescriptions[shaderDesc.Type] = shaderDesc;
		}

		inline void AddShader(const EShaderType& shaderType, const std::string& source, const ShaderMacros& macros = {})
		{
			AddShader(ShaderDesc(shaderType, source, macros));
		}

		[[nodiscard]] inline bool HasShader(const EShaderType& shaderType) const noexcept { return ShaderDescriptions.contains(shaderType); }
		[[nodiscard]] inline const std::string& GetName() const noexcept { return Name; }
		[[nodiscard]] inline const std::map<EShaderType, ShaderDesc>& GetShaderDescriptions() const noexcept { return ShaderDescriptions; }
	};

	class IShaderProgram : public TypeBase<IShaderProgram>
	{
	public:
		[[nodiscard]] virtual const ShaderProgramDesc& GetDesc() const = 0;
		[[nodiscard]] virtual std::vector<ShaderResourceDesc> GetResources(const ShaderResourceType& resourceType) = 0;

		[[nodiscard]] virtual bool HasInputLayout() const noexcept = 0;
		[[nodiscard]] virtual uint8_t GetInputVariablesCount() const noexcept = 0;
		[[nodiscard]] virtual const ShaderInputVariables_t& GetInputVariables() const noexcept = 0;
	};

	typedef std::shared_ptr<IShaderProgram> Shader_ptr;
}