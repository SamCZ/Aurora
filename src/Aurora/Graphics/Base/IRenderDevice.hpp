#pragma once

#include "ShaderBase.hpp"
#include "Texture.hpp"
#include "Sampler.hpp"
#include "Buffer.hpp"
#include "InputLayout.hpp"


#include "PrimitiveType.hpp"
#include "BlendState.hpp"
#include "RasterState.hpp"
#include "DepthStencilState.hpp"

namespace Aurora
{
	typedef const void* TextureData;



	class IRenderDevice
	{
	public:
		IRenderDevice() = default;
		IRenderDevice(const IRenderDevice& other) = delete;
		virtual ~IRenderDevice() = default;
	public:
		virtual void Init() = 0;
		// Shaders
		virtual Shader_ptr CreateShaderProgram(const ShaderProgramDesc& desc) = 0;
		virtual void ApplyShader(const Shader_ptr& shader) = 0;
		// Textures
		virtual Texture_ptr CreateTexture(const TextureDesc& desc, TextureData textureData) = 0;
		virtual void WriteTexture(const Texture_ptr& texture, uint32_t subresource, const void* data) = 0;
		virtual void ClearTextureFloat(const Texture_ptr& texture, const Color& clearColor) = 0;
		virtual void ClearTextureUInt(const Texture_ptr& texture, uint32_t clearColor) = 0;
		inline Texture_ptr CreateTexture(const TextureDesc& desc) { return CreateTexture(desc, nullptr); }
		// Buffers
		virtual Buffer_ptr CreateBuffer(const BufferDesc& desc, const void* data) = 0;
		/*virtual void WriteBuffer(const Buffer_ptr& buffer, const void* data, size_t dataSize) = 0;
		virtual void ClearBufferUInt(const Buffer_ptr& buffer, uint32_t clearValue) = 0;
		virtual void CopyToBuffer(const Buffer_ptr& dest, uint32_t destOffsetBytes, const Buffer_ptr& src, uint32_t srcOffsetBytes, size_t dataSizeBytes) = 0;
		virtual void ReadBuffer(const Buffer_ptr& buffer, void* data, size_t* dataSize) = 0; // for debugging purposes only
		inline Buffer_ptr CreateBuffer(const BufferDesc& desc) { return CreateBuffer(desc, nullptr); }*/
	};
}