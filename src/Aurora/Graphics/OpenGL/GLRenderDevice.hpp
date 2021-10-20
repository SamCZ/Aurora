#pragma once

#include "../Base/IRenderDevice.hpp"
#include "GL.hpp"
#include "GLContextState.hpp"
#include "Aurora/Tools/robin_hood.h"

namespace Aurora
{
	class FrameBuffer
	{
	public:
		GLuint Handle;
		GLenum DrawBuffers[8]{};
		uint32_t NumBuffers;
		ITexture* DepthTarget;
		ITexture* RenderTargets[8]{};

		FrameBuffer()
				: Handle(0)
				, NumBuffers(0)
				, DepthTarget(nullptr) { }

		~FrameBuffer()
		{
			if (Handle)
				glDeleteFramebuffers(1, &Handle);
		}
	};

	typedef std::shared_ptr<FrameBuffer> FrameBuffer_ptr;

	class GLRenderDevice : public IRenderDevice
	{
	public:
		friend class GLTexture;
		friend class ShellRenderInterfaceOpenGL;
	private:
		GLuint m_nVAO;
		GLuint m_nVAOEmpty;
		GLuint m_LastVao;
		FrameBuffer_ptr m_CurrentFrameBuffer = nullptr;
		robin_hood::unordered_map<uint32_t, FrameBuffer_ptr> m_CachedFrameBuffers;

		GLContextState m_ContextState;

		Vector2i m_LastViewPort;
		FRasterState m_LastRasterState;
		FDepthStencilState m_LastDepthState;
		InputLayout_ptr m_LastInputLayout;

		// Embedded shaders
		Shader_ptr m_BlitShader;
	public:
		GLRenderDevice();
		~GLRenderDevice() override;

		inline GLContextState& GetContextState() { return m_ContextState; }
	public:
		void Init() override;
		// Shaders
		Shader_ptr CreateShaderProgram(const ShaderProgramDesc& desc) override;
		static GLuint CompileShaderRaw(const std::string& sourceString, const EShaderType& shaderType, std::string* errorOutput);
		void SetShader(const Shader_ptr& shader) override;
		// Textures
		Texture_ptr CreateTexture(const TextureDesc& desc, TextureData textureData) override;
		void WriteTexture(const Texture_ptr &texture, uint32_t mipLevel, uint32_t subresource, const void *data) override;
		void ClearTextureFloat(const Texture_ptr& texture, const Color& clearColor) override;
		void ClearTextureUInt(const Texture_ptr& texture, uint32_t clearColor) override;
		void GenerateMipmaps(const Texture_ptr& texture) override;
		// Buffers
		Buffer_ptr CreateBuffer(const BufferDesc& desc, const void* data) override;
		void WriteBuffer(const Buffer_ptr& buffer, const void* data, size_t dataSize, size_t offset) override;
		void ClearBufferUInt(const Buffer_ptr& buffer, uint32_t clearValue) override;
		void CopyToBuffer(const Buffer_ptr& dest, uint32_t destOffsetBytes, const Buffer_ptr& src, uint32_t srcOffsetBytes, size_t dataSizeBytes) override;
		void* MapBuffer(const Buffer_ptr& buffer, EBufferAccess bufferAccess) override;
		void UnmapBuffer(const Buffer_ptr& buffer) override;
		// Samplers
		Sampler_ptr CreateSampler(const SamplerDesc& desc) override;
		// InputLayout
		InputLayout_ptr CreateInputLayout(const std::vector<VertexAttributeDesc>& desc) override;
		// Drawing
		void Draw(const DrawCallState& state, const std::vector<DrawArguments>& args) override;
		void DrawIndexed(const DrawCallState& state, const std::vector<DrawArguments>& args, bool bindState) override;
		void DrawIndirect(const DrawCallState& state, const Buffer_ptr& indirectParams, uint32_t offsetBytes) override;

		void Dispatch(const DispatchState& state, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) override;
		void DispatchIndirect(const DispatchState& state, const Buffer_ptr& indirectParams, uint32_t offsetBytes) override;

		void InvalidateState() override;

		void Blit(const Texture_ptr &src, const Texture_ptr &dest) override;
	public:
		void BindShaderResources(const BaseState& state) override;
		void ApplyDispatchState(const DispatchState& state) override;
		void ApplyDrawCallState(const DrawCallState& state) override;
		void BindShaderInputs(const DrawCallState &state) override;
		void BindRenderTargets(const DrawCallState &state) override;
		void SetBlendState(const DrawCallState &state) override;
		void SetRasterState(const FRasterState& rasterState) override;
		void ClearRenderTargets(const DrawCallState &state) override;
		void SetDepthStencilState(FDepthStencilState state) override;

		void NotifyTextureDestroy(class GLTexture* texture);
		FrameBuffer_ptr GetCachedFrameBuffer(const DrawCallState &state);
	};
}