#pragma once

#include "../Base/IRenderDevice.hpp"
#include "GL.hpp"

namespace Aurora
{
	struct GLPipelineState
	{
		GLuint LastShaderHandle;
	};

	class GLRenderDevice : public IRenderDevice
	{
	private:
		GLPipelineState m_PipelineState;
	public:
		GLRenderDevice();
		~GLRenderDevice() override = default;
	public:
		void Init() override;
		// Shaders
		Shader_ptr CreateShaderProgram(const ShaderProgramDesc& desc) override;
		static GLuint CompileShaderRaw(const std::string& sourceString, const ShaderType& shaderType, std::string* errorOutput);
		void ApplyShader(const Shader_ptr& shader) override;
		// Textures
		Texture_ptr CreateTexture(const TextureDesc& desc, TextureData textureData) override;
		void WriteTexture(const Texture_ptr& texture, uint32_t subresource, const void* data) override;
		void ClearTextureFloat(const Texture_ptr& texture, const Color& clearColor) override;
		void ClearTextureUInt(const Texture_ptr& texture, uint32_t clearColor) override;
		// Buffers
		Buffer_ptr CreateBuffer(const BufferDesc& desc, const void* data) override;
		/*void WriteBuffer(const Buffer_ptr& buffer, const void* data, size_t dataSize) override;
		void ClearBufferUInt(const Buffer_ptr& buffer, uint32_t clearValue) override;
		void CopyToBuffer(const Buffer_ptr& dest, uint32_t destOffsetBytes, const Buffer_ptr& src, uint32_t srcOffsetBytes, size_t dataSizeBytes) override;;
		void ReadBuffer(const Buffer_ptr& buffer, void* data, size_t* dataSize) override; // for debugging purposes only*/

	};
}