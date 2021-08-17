#pragma once

#include "Aurora/Core/Common.hpp"
#include "../Base/IRenderDevice.hpp"
#include "GL.hpp"
#include "GLContextState.hpp"

namespace Aurora
{
	AU_CLASS(FrameBuffer)
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
		std::map<uint32_t, FrameBuffer_ptr> m_CachedFrameBuffers;

		GLContextState m_ContextState;

		Vector2i m_LastViewPort;
		FRasterState m_LastRasterState;
		FDepthStencilState m_LastDepthState;
		InputLayout_ptr m_LastInputLayout;
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
		void WriteBuffer(const Buffer_ptr& buffer, const void* data, size_t dataSize) override;
		void ClearBufferUInt(const Buffer_ptr& buffer, uint32_t clearValue) override;
		void CopyToBuffer(const Buffer_ptr& dest, uint32_t destOffsetBytes, const Buffer_ptr& src, uint32_t srcOffsetBytes, size_t dataSizeBytes) override;
		void ReadBuffer(const Buffer_ptr& buffer, void* data, size_t* dataSize) override;
		// Samplers
		Sampler_ptr CreateSampler(const SamplerDesc& desc) override;
		// InputLayout
		InputLayout_ptr CreateInputLayout(const std::vector<VertexAttributeDesc>& desc) override;
		// Drawing
		void Draw(const DrawCallState& state, const std::vector<DrawArguments>& args) override;
		void DrawIndexed(const DrawCallState& state, const std::vector<DrawArguments>& args) override;
		void DrawIndirect(const DrawCallState& state, const Buffer_ptr& indirectParams, uint32_t offsetBytes) override;

		void Dispatch(const DispatchState& state, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) override;
		void DispatchIndirect(const DispatchState& state, const Buffer_ptr& indirectParams, uint32_t offsetBytes) override;

		void InvalidateState() override;
	public:
		void BindShaderResources(const BaseState& state);

		void ApplyDispatchState(const DispatchState& state);
		void ApplyDrawCallState(const DrawCallState& state);

		void BindShaderInputs(const DrawCallState &state);

		void BindRenderTargets(const DrawCallState &state);

		FrameBuffer_ptr GetCachedFrameBuffer(const DrawCallState &state);
		void NotifyTextureDestroy(class GLTexture* texture);

		void SetBlendState(const DrawCallState &state);

		void SetRasterState(const FRasterState& rasterState);

		void ClearRenderTargets(const DrawCallState &state);

		void SetDepthStencilState(FDepthStencilState state);
	};
}