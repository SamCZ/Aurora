#pragma once

#include <string>
#include <unordered_set>
#include <map>
#include <vector>

#include "../Base/ShaderBase.hpp"
#include "GLUtils.hpp"

namespace Aurora
{
	class GLSHader;

	struct GLResourceAttribs
	{
		std::string Name = "Unknown";
		ShaderResourceType ResourceType = ShaderResourceType::Unknown;
		uint32_t Binding = 0;
		uint32_t ArraySize = 0;
	};

	struct UniformBufferInfo final : GLResourceAttribs
	{
		GLuint UBIndex = 0;
		EShaderType ShadersIn = EShaderType::Unknown;
		size_t Size = 0;
		std::vector<ShaderVariable> Variables = {};
	};

	struct SamplerInfo final : GLResourceAttribs
	{
		GLint  Location = 0;
		GLenum SamplerType = GL_NONE;
	};

	struct ImageInfo final : GLResourceAttribs
	{
		GLint  Location = 0;
		GLenum ImageType = GL_NONE;
	};

	struct StorageBlockInfo final : GLResourceAttribs
	{
		GLuint SBIndex = 0;
	};

	class AU_API GLShaderResources
	{
	private:
		uint32_t m_UniformBufferBinding;
		uint32_t m_SamplerBinding;
		uint32_t m_ImageBinding;
		uint32_t m_StorageBufferBinding;

		std::vector<UniformBufferInfo> m_UniformBlocks;
		std::vector<SamplerInfo>       m_Samplers;
		std::vector<ImageInfo>         m_Images;
		std::vector<StorageBlockInfo>  m_StorageBlocks;
	public:
		GLShaderResources();

		void LoadUniforms(GLuint program);

		[[nodiscard]] inline const std::vector<UniformBufferInfo>& GetUniformBlocks() const noexcept { return m_UniformBlocks; }
		[[nodiscard]] inline const std::vector<SamplerInfo>& GetSamplers() const noexcept { return m_Samplers; }
		[[nodiscard]] inline const std::vector<ImageInfo>& GetImages() const noexcept { return m_Images; }
		[[nodiscard]] inline const std::vector<StorageBlockInfo>& GetStorageBlocks() const noexcept { return m_StorageBlocks; }
	};
}