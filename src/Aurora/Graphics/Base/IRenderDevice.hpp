#pragma once

#include <utility>
#include <map>
#include <array>

#include "ShaderBase.hpp"
#include "Texture.hpp"
#include "Sampler.hpp"
#include "Buffer.hpp"
#include "InputLayout.hpp"


#include "PrimitiveType.hpp"
#include "BlendState.hpp"
#include "RasterState.hpp"
#include "FDepthStencilState.hpp"

#include "../ViewPort.hpp"

namespace Aurora
{
	typedef const void* TextureData;

	struct TextureBinding
	{
		enum class EAccess : uint8_t
		{
			Read,
			Write,
			ReadAndWrite
		};

		Texture_ptr Texture;
		bool IsUAV;
		EAccess Access;
		int MipLevel;

		TextureBinding() : Texture(nullptr), IsUAV(false), Access(EAccess::Write), MipLevel(0) {}
		explicit TextureBinding(Texture_ptr texture) : Texture(std::move(texture)), IsUAV(false), Access(EAccess::Write), MipLevel(0) {}
		TextureBinding(Texture_ptr texture, bool isUAV) : Texture(std::move(texture)), IsUAV(isUAV), Access(EAccess::Write), MipLevel(0) {}
		TextureBinding(Texture_ptr texture, bool isUAV, EAccess access, int mipLevel) : Texture(std::move(texture)), IsUAV(isUAV), Access(access), MipLevel(mipLevel) {}
	};

	struct TargetBinding
	{
		Texture_ptr Texture;
		uint32_t Index; // This values is for texture arrays / cubemaps. TODO: Rename this var to something better.
		uint32_t MipSlice;

		TargetBinding() : Texture(nullptr), Index(0), MipSlice(0) {}
		explicit TargetBinding(Texture_ptr texture) : Texture(std::move(texture)), Index(0), MipSlice(0) {}
		TargetBinding(Texture_ptr texture, uint32_t index) : Texture(std::move(texture)), Index(index), MipSlice(0) {}
		TargetBinding(Texture_ptr texture, uint32_t index, uint32_t mipSlice) : Texture(std::move(texture)), Index(index), MipSlice(mipSlice) {}
	};

	struct IndexBufferBinding
	{
		Buffer_ptr Buffer;
		EIndexBufferFormat Format;

		IndexBufferBinding() : Buffer(nullptr), Format(EIndexBufferFormat::Uint32) {}
		IndexBufferBinding(Buffer_ptr buffer, EIndexBufferFormat format) : Buffer(std::move(buffer)), Format(format) {}
	};

	struct UniformBufferBinding
	{
		Buffer_ptr Buffer = nullptr;
		uint32_t Offset = 0;
		uint32_t Size = 0;
	};

	struct StateResources
	{
		static constexpr uint8_t MaxBoundTextures = 8;

		std::map<std::string, TextureBinding> BoundTextures{};
		std::map<std::string, Sampler_ptr> BoundSamplers{};
		std::map<std::string, UniformBufferBinding> BoundUniformBuffers{};
		std::map<std::string, Buffer_ptr> SSBOBuffers{};

		virtual void ResetResources()
		{
			BoundTextures.clear();
			BoundSamplers.clear();
			BoundUniformBuffers.clear();
			SSBOBuffers.clear();
		}

		inline void BindTexture(const std::string& name, const Texture_ptr& texture, bool isUAV = false, TextureBinding::EAccess access = TextureBinding::EAccess::Write, int mipLevel = 0)
		{
			BoundTextures[name] = TextureBinding(texture, isUAV, access, mipLevel);
		}

		inline void BindSampler(const std::string& name, const Sampler_ptr& sampler)
		{
			BoundSamplers[name] = sampler;
		}

		inline void BindUniformBuffer(const std::string& name, const Buffer_ptr& uniformBuffer, uint32_t offset = 0, uint32_t size = 0)
		{
			BoundUniformBuffers[name] = {uniformBuffer, offset, size};
		}

		inline void BindSSBOBuffer(const std::string& name, const Buffer_ptr& ssbo)
		{
			SSBOBuffers[name] = ssbo;
		}
	};

	struct BaseState : StateResources
	{
		Shader_ptr Shader;

		BaseState() : StateResources(), Shader(nullptr)
		{

		}
	};

	struct DispatchState : BaseState
	{
		DispatchState() : BaseState() {}
	};

	struct DrawCallState : BaseState
	{
		static constexpr int MaxRenderTargets = 8;
		std::array<TargetBinding, MaxRenderTargets> RenderTargets;
		Texture_ptr DepthTarget;
		uint32_t DepthIndex;
		uint32_t DepthMipSlice;

		InputLayout_ptr InputLayoutHandle;

		std::map<uint32_t, Buffer_ptr> VertexBuffers;
		bool HasAnyRenderTarget;
		FDepthStencilState DepthStencilState;

		bool ClearColorTarget;
		bool ClearDepthTarget;
		bool ClearStencilTarget;
		Color ClearColor;
		float ClearDepth;
		int ClearStencil;

		IndexBufferBinding IndexBuffer;
		uint32_t IndexBufferOffset;

		EPrimitiveType PrimitiveType;

		FRasterState RasterState;
		FViewPort ViewPort;

		DrawCallState()
		: BaseState(),
		  DepthTarget(nullptr),
		  DepthIndex(0),
		  DepthMipSlice(0),
		  IndexBuffer(),
		  PrimitiveType(EPrimitiveType::TriangleList),
		  IndexBufferOffset(0),
		  HasAnyRenderTarget(false),
		  RasterState(),
		  InputLayoutHandle(nullptr),
		  VertexBuffers(),
		  ClearColorTarget(true),
		  ClearDepthTarget(true),
		  ClearStencilTarget(false),
		  ClearDepth(1.0f),
		  ClearStencil(0),
		  ClearColor(0, 0, 0, 0),
		  DepthStencilState(),
		  ViewPort(0, 0) { }

		inline void ResetTargets()
		{
			for (int i = 0; i < MaxRenderTargets; ++i) {
				RenderTargets[i] = TargetBinding();
			}

			DepthTarget = nullptr;

			HasAnyRenderTarget = false;
		}

		inline void ResetVertexBuffers()
		{
			VertexBuffers.clear();
		}

		inline void ResetIndexBuffer()
		{
			IndexBuffer = IndexBufferBinding();
		}

		inline void SetIndexBuffer(Buffer_ptr buffer, EIndexBufferFormat format = EIndexBufferFormat::Uint32)
		{
			IndexBuffer = IndexBufferBinding(std::move(buffer), format);
		}

		inline void SetVertexBuffer(uint32_t slot, Buffer_ptr buffer)
		{
			VertexBuffers[slot] = std::move(buffer);
		}

		inline void BindTarget(uint16_t slot, Texture_ptr texture, uint32_t index = 0, uint32_t mipSlice = 0)
		{
			assert(slot < MaxRenderTargets);

			if(texture != nullptr) {
				HasAnyRenderTarget = true;
			}

			RenderTargets[slot] = TargetBinding(std::move(texture), index, mipSlice);
		}

		void BindDepthTarget(const Texture_ptr& depthTarget, uint32_t depthIndex, uint32_t depthMipSlice)
		{
			DepthTarget = depthTarget;
			DepthIndex = depthIndex;
			DepthMipSlice = depthMipSlice;

			if(depthTarget != nullptr)
			{
				HasAnyRenderTarget = true;
			}
		}

		inline void UnboundTarget(uint16_t slot)
		{
			assert(slot < MaxRenderTargets);
			RenderTargets[slot] = TargetBinding();
		}
	};

	struct DrawArguments
	{
		uint32_t VertexCount;
		uint32_t InstanceCount;
		size_t StartIndexLocation;
		uint32_t StartVertexLocation;
		uint32_t StartInstanceLocation;

		DrawArguments()
				: VertexCount(0)
				, InstanceCount(1)
				, StartIndexLocation(0)
				, StartVertexLocation(0)
				, StartInstanceLocation(0)
		{ }

		explicit DrawArguments(uint32_t vertexCount)
				: VertexCount(vertexCount)
				, InstanceCount(1)
				, StartIndexLocation(0)
				, StartVertexLocation(0)
				, StartInstanceLocation(0) {}
	};

	enum class EGpuVendor : uint8_t
	{
		Unknown = 0,
		Nvidia,
		AMD
	};

	struct FrameRenderStatistics
	{
		uint32_t VertexCount;
		uint32_t DrawCalls;
		uint32_t BufferWrites;
		uint32_t BufferMaps;
		uint32_t GPUMemoryUsage;
	};

	struct DrawArraysIndirectCommand
	{
		uint32_t count;
		uint32_t instanceCount;
		uint32_t first;
		uint32_t baseInstance;
	};

	typedef  struct {
		uint32_t  count;
		uint32_t  instanceCount;
		uint32_t  firstIndex;
		uint32_t  baseVertex;
		uint32_t  baseInstance;
	} DrawElementsIndirectCommand;

	class IRenderDevice
	{
	protected:
		FrameRenderStatistics m_FrameRenderStatistics = {};
	public:
		IRenderDevice() = default;
		IRenderDevice(const IRenderDevice& other) = delete;
		virtual ~IRenderDevice() = default;
	public:
		virtual void Init() = 0;
		// Shaders
		virtual Shader_ptr CreateShaderProgram(const ShaderProgramDesc& desc) = 0;
		virtual void SetShader(const Shader_ptr& shader) = 0;
		// Textures
		virtual Texture_ptr CreateTexture(const TextureDesc& desc, TextureData textureData) = 0;
		virtual void WriteTexture(const Texture_ptr &texture, uint32_t mipLevel, uint32_t subresource, const void *data) = 0;
		virtual void ClearTextureFloat(const Texture_ptr& texture, const Color& clearColor) = 0;
		virtual void ClearTextureUInt(const Texture_ptr& texture, uint32_t clearColor) = 0;
		virtual void GenerateMipmaps(const Texture_ptr& texture) = 0;
		inline Texture_ptr CreateTexture(const TextureDesc& desc) { return CreateTexture(desc, nullptr); }

		virtual void* GetTextureHandleForBindless(const Texture_ptr& texture, bool srgb) = 0;
		virtual bool MakeTextureHandleResident(const Texture_ptr& texture, bool enabled) = 0;

		// Buffers
		virtual Buffer_ptr CreateBuffer(const BufferDesc& desc, const void* data) = 0;
		virtual void WriteBuffer(const Buffer_ptr& buffer, const void* data, size_t dataSize, size_t offset) = 0;
		virtual void ClearBufferUInt(const Buffer_ptr& buffer, uint32_t clearValue) = 0;
		virtual void CopyToBuffer(const Buffer_ptr& dest, uint32_t destOffsetBytes, const Buffer_ptr& src, uint32_t srcOffsetBytes, size_t dataSizeBytes) = 0;
		virtual void* MapBuffer(const Buffer_ptr& buffer, EBufferAccess bufferAccess) = 0;

		template<typename T>
		inline T* MapBuffer(const Buffer_ptr& buffer, EBufferAccess bufferAccess)
		{
			return reinterpret_cast<T*>((void*)MapBuffer(buffer, bufferAccess));
		}

		inline void WriteBuffer(const Buffer_ptr& buffer, const void* data)
		{
			if(buffer)
				WriteBuffer(buffer, data, buffer->GetDesc().ByteSize, 0);
		}

		virtual void UnmapBuffer(const Buffer_ptr& buffer) = 0;
		inline Buffer_ptr CreateBuffer(const BufferDesc& desc) { return CreateBuffer(desc, nullptr); }
		// Samplers
		virtual Sampler_ptr CreateSampler(const SamplerDesc& desc) = 0;
		// InputLayout
		virtual InputLayout_ptr CreateInputLayout(const std::vector<VertexAttributeDesc>& desc) = 0;
		// Drawing
		virtual void Draw(const DrawCallState& state, const std::vector<DrawArguments>& args, bool bindState = true) = 0;
		virtual void DrawIndexed(const DrawCallState& state, const std::vector<DrawArguments>& args, bool bindState = true) = 0;
		virtual void DrawIndirect(const DrawCallState& state, const Buffer_ptr& indirectParams, uint32_t offsetBytes) = 0;

		virtual void Dispatch(const DispatchState& state, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) = 0;
		virtual void DispatchIndirect(const DispatchState& state, const Buffer_ptr& indirectParams, uint32_t offsetBytes) = 0;

		virtual void InvalidateState() = 0;

		virtual void Blit(const Texture_ptr &src, const Texture_ptr &dest) = 0;

		virtual void SetViewPort(const FViewPort& wp) = 0;
		[[nodiscard]] virtual const FViewPort& GetCurrentViewPort() const = 0;

		virtual void BindShaderResources(const BaseState& state) = 0;
		virtual void ApplyDispatchState(const DispatchState& state) = 0;
		virtual void ApplyDrawCallState(const DrawCallState& state) = 0;
		virtual void BindShaderInputs(const DrawCallState &state, bool force = false) = 0;
		virtual void BindRenderTargets(const DrawCallState &state) = 0;
		virtual void SetBlendState(const DrawCallState &state) = 0;
		virtual void SetRasterState(const FRasterState& rasterState) = 0;
		virtual void ClearRenderTargets(const DrawCallState &state) = 0;
		virtual void SetDepthStencilState(FDepthStencilState state) = 0;

		virtual size_t GetUsedGPUMemory() = 0;

		[[nodiscard]] inline const FrameRenderStatistics& GetFrameRenderStatistics() const { return m_FrameRenderStatistics; }
		inline void ResetFrameRenderStatistics() { memset(&m_FrameRenderStatistics, 0, sizeof(FrameRenderStatistics)); }
	};
}