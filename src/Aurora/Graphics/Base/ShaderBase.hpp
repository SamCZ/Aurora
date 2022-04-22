#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <map>
#include <functional>

#include "InputLayout.hpp"
#include "TypeBase.hpp"
#include "Aurora/Core/Types.hpp"
#include "Aurora/Logger/Logger.hpp"

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
		Unknown = 0,
		Vertex,
		Hull,
		Domain,
		Geometry,
		Pixel,
		Compute,

		Amplification,
		Mesh,
		RayGen,
		RayMiss,
		RayClosestHit,
		RayAnyHit,
		RayIntersection,
		Callable
	};

	static std::string ShaderType_ToString(const EShaderType& shaderType)
	{
		switch (shaderType) {

			case EShaderType::Unknown: return "Unknown";
			case EShaderType::Vertex: return "Vertex";
			case EShaderType::Hull: return "Hull";
			case EShaderType::Domain: return "Domain";
			case EShaderType::Geometry: return "Geometry";
			case EShaderType::Pixel: return "Pixel";
			case EShaderType::Compute: return "Compute";
			default: return "Unknown";
		}
	}

	static EShaderType ShaderType_FromString(const std::string& name)
	{
		if(name == "vertex")
			return EShaderType::Vertex;
		else if(name == "fragment" || name == "pixel")
			return EShaderType::Pixel;
		else if(name == "geometry")
			return EShaderType::Geometry;
		else if(name == "compute")
			return EShaderType::Compute;
		else
			return EShaderType::Unknown;
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

		/// Shader resource type
		ShaderResourceType Type = ShaderResourceType::Unknown;

		/// Array size. For non-array resource this value is 1.
		uint32_t ArraySize = 0;

		/// Size of resource
		uint32_t Size = 0;

		/// Shaders in
		EShaderType ShadersIn = EShaderType::Unknown;

		std::vector<ShaderVariable> Variables{};
	};

	typedef std::map<std::string, std::string> ShaderMacros;

	struct ShaderDesc
	{
		EShaderType Type;
		std::string Source;
		Path FilePath;
		ShaderMacros Macros;
		bool EnableBindless;

		ShaderDesc() : Type(EShaderType::Unknown), Source(), FilePath(), Macros(), EnableBindless(false)
		{

		}

		ShaderDesc(EShaderType type, std::string source, Path filepath, ShaderMacros macros, bool enableBindless)
				: Type(type), Source(std::move(source)), FilePath(std::move(filepath)), Macros(std::move(macros)), EnableBindless(enableBindless) { }
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
		ShaderProgramDesc() = default;
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
				AU_LOG_WARNING("Cannot add Unknown shader type to ", Name, " ! Skipping...");
				return;
			}

			if(ShaderDescriptions.contains(shaderDesc.Type)) {
				AU_LOG_WARNING("Shader ", ShaderType_ToString(shaderDesc.Type), " already exists in program ", Name, " ! Skipping...");
				return;
			}

			ShaderDescriptions[shaderDesc.Type] = shaderDesc;
		}

		inline void AddShader(const EShaderType& shaderType, const std::string& source, const Path& filepath = "", const ShaderMacros& macros = {}, bool enableBindless = false)
		{
			AddShader(ShaderDesc(shaderType, source, filepath, macros, enableBindless));
		}

		inline bool SetShaderMacros(const EShaderType& shaderType, const ShaderMacros& shaderMacros)
		{
			const auto& it = ShaderDescriptions.find(shaderType);

			if(it == ShaderDescriptions.end())
				return false;

			ShaderDesc& shaderDesc = it->second;
			shaderDesc.Macros = shaderMacros;

			return true;
		}

		inline void SetShaderMacros(const ShaderMacros& shaderMacros)
		{
			for(auto& it : ShaderDescriptions)
			{
				it.second.Macros = shaderMacros;
			}
		}

		inline void AddShaderMacros(const ShaderMacros& shaderMacros)
		{
			for(auto& it : ShaderDescriptions)
			{
				it.second.Macros.insert(shaderMacros.begin(), shaderMacros.end());
			}
		}

		[[nodiscard]] inline bool HasShader(const EShaderType& shaderType) const noexcept { return ShaderDescriptions.contains(shaderType); }
		[[nodiscard]] inline const std::string& GetName() const noexcept { return Name; }
		[[nodiscard]] inline std::map<EShaderType, ShaderDesc>& GetShaderDescriptions() noexcept { return ShaderDescriptions; }
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