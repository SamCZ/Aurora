#pragma once

#include <vector>
#include "../Base/IRenderDevice.hpp"
#include "GL.hpp"
#include "GLContextState.hpp"
#include "Aurora/Tools/robin_hood.h"

namespace Aurora
{
	AU_API GLBuffer* GetBuffer(const Buffer_ptr& buffer);
	AU_API GLTexture* GetTexture(const Texture_ptr& texture);
	AU_API GLTexture* GetTexture(ITexture* texture);
	AU_API GLSampler* GetSampler(const Sampler_ptr& sampler);
	AU_API GLShaderProgram* GetShader(const Shader_ptr& shader);

	struct FrameBufferKey
	{
		static constexpr int MaxRenderTargets = 8;
		std::array<TargetBinding, MaxRenderTargets> RenderTargets;
		ITexture* DepthTarget;
		uint32_t DepthIndex;
		uint32_t DepthMipSlice;

		[[nodiscard]] int compare(const FrameBufferKey &other) const
		{
			/*if (Width < other.Width) return -1;
			if (Width > other.Width) return 1;*/

			if (DepthTarget < other.DepthTarget) return -1;
			if (DepthTarget > other.DepthTarget) return 1;

			if (DepthIndex < other.DepthIndex) return -1;
			if (DepthIndex > other.DepthIndex) return 1;

			if (DepthMipSlice < other.DepthMipSlice) return -1;
			if (DepthMipSlice > other.DepthMipSlice) return 1;

			for (int i = 0; i < MaxRenderTargets; ++i)
			{
				const TargetBinding& binding = RenderTargets[i];
				const TargetBinding& bindingOther = other.RenderTargets[i];

				if (binding.Texture < bindingOther.Texture) return -1;
				if (binding.Texture > bindingOther.Texture) return 1;

				if (binding.Index < bindingOther.Index) return -1;
				if (binding.Index > bindingOther.Index) return 1;

				if (binding.MipSlice < bindingOther.MipSlice) return -1;
				if (binding.MipSlice > bindingOther.MipSlice) return 1;
			}

			return 0;
		}

		bool operator<(const FrameBufferKey &other) const
		{ return compare(other) < 0; }

		bool operator>(const FrameBufferKey &other) const
		{ return compare(other) > 0; }

		bool operator==(const FrameBufferKey &other) const
		{ return compare(other) == 0; }
	};

	struct VaoKey
	{
		Shader_ptr Shader;
		robin_hood::unordered_map<uint32_t, Buffer_ptr> Buffers;

		[[nodiscard]] int compare(const VaoKey &other) const
		{
			if (Shader < other.Shader) return -1;
			if (Shader > other.Shader) return 1;

			size_t bufferSize = Buffers.size();

			if(bufferSize < other.Buffers.size()) return -1;
			if(bufferSize > other.Buffers.size()) return 1;

			for (const auto& [location, buffer]: Buffers)
			{
				auto it = other.Buffers.find(location);

				if (it == other.Buffers.end())
				{
					return -1;
				}

				const Buffer_ptr bufferOther = it->second;

				if (buffer < bufferOther) return -1;
				if (buffer > bufferOther) return 1;
			}

			return 0;
		}

		bool operator<(const VaoKey &other) const
		{ return compare(other) < 0; }

		bool operator>(const VaoKey &other) const
		{ return compare(other) > 0; }

		bool operator==(const VaoKey &other) const
		{ return compare(other) == 0; }
	};

	class FrameBuffer
	{
	public:
		std::string Name;
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

	class AU_API GLRenderDevice : public IRenderDevice
	{
	public:
		friend class GLTexture;
		friend class ShellRenderInterfaceOpenGL;
	private:
		GLuint m_nVAO;
		GLuint m_nVAOEmpty;
		GLuint m_LastVao;
		FrameBuffer_ptr m_CurrentFrameBuffer = nullptr;
		std::map<FrameBufferKey, FrameBuffer_ptr> m_CachedFrameBuffers;
		std::map<VaoKey, GLuint> m_CachedVaos;

		GLContextState m_ContextState;

		FViewPort m_LastViewPort;
		FRasterState m_LastRasterState;
		FDepthStencilState m_LastDepthState;
		InputLayout_ptr m_LastInputLayout;
		std::vector<uint8_t> m_LastVertexAttribs;

		// Embedded shaders
		Shader_ptr m_BlitShader;

		EGpuVendor m_GpuVendor;
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
		void ClearTextureFloat(const Texture_ptr& texture, float val) override;
		void ClearTextureUInt(const Texture_ptr& texture, uint32_t clearColor) override;
		void GenerateMipmaps(const Texture_ptr& texture) override;
		bool ReadTexture(const Texture_ptr& texture, std::vector<uint8>& imageBuffer) override;
		void* GetTextureHandleForBindless(const Texture_ptr& texture, bool srgb) override;
		bool MakeTextureHandleResident(const Texture_ptr& texture, bool enabled) override;
		// Buffers
		Buffer_ptr CreateBuffer(const BufferDesc& desc, const void* data) override;
		void WriteBuffer(const Buffer_ptr& buffer, const void* data, size_t dataSize, size_t offset) override;
		void ClearBufferUInt(const Buffer_ptr& buffer, uint32_t clearValue) override;
		void CopyToBuffer(const Buffer_ptr& dest, uint32_t destOffsetBytes, const Buffer_ptr& src, uint32_t srcOffsetBytes, size_t dataSizeBytes) override;
		uint8_t* MapBuffer(const Buffer_ptr& buffer, EBufferAccess bufferAccess) override;
		void UnmapBuffer(const Buffer_ptr& buffer) override;
		// Samplers
		Sampler_ptr CreateSampler(const SamplerDesc& desc) override;
		// InputLayout
		InputLayout_ptr CreateInputLayout(const std::vector<VertexAttributeDesc>& desc) override;
		// Drawing
		void Draw(const DrawCallState& state, const std::vector<DrawArguments>& args, bool bindState) override;
		void DrawIndexed(const DrawCallState& state, const std::vector<DrawArguments>& args, bool bindState) override;
		void DrawIndirect(const DrawCallState& state, const Buffer_ptr& indirectParams, uint32_t offsetBytes) override;

		void Dispatch(const DispatchState& state, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) override;
		void DispatchIndirect(const DispatchState& state, const Buffer_ptr& indirectParams, uint32_t offsetBytes) override;

		void InvalidateState() override;

		void Blit(const Texture_ptr &src, const Texture_ptr &dest) override;

		void SetViewPort(const FViewPort& wp) override;
		[[nodiscard]] const FViewPort& GetCurrentViewPort() const override;

		size_t GetUsedGPUMemory() override;
	public:
		void BindShaderResources(const BaseState& state) override;
		void ApplyShaderUniformResources(const Shader_ptr& shader, const UniformResources& resources) override;
		void ApplyDispatchState(const DispatchState& state) override;
		void ApplyDrawCallState(const DrawCallState& state) override;
		void BindShaderInputsCached(const DrawCallState &state);
		void BindShaderInputs(const DrawCallState &state, bool force) override;
		void BindRenderTargets(const DrawCallState &state) override;
		void SetBlendState(const FBlendState& state) override;
		void SetRasterState(const FRasterState& rasterState) override;
		void ClearRenderTargets(const DrawCallState &state) override;
		void SetDepthStencilState(FDepthStencilState state) override;

		void NotifyTextureDestroy(class GLTexture* texture);
		void NotifyBufferDestroy(class GLBuffer* buffer);
		FrameBuffer_ptr GetCachedFrameBuffer(const DrawCallState &state);
	};
}