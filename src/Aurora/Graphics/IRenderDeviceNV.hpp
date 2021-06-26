/*
* Copyright (c) 2012-2016, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma once

#include <cstdint>
#include <memory.h>
#include "Aurora/Core/Common.hpp"
#include "Aurora/Core/Color.hpp"

#include "Base/ShaderBase.hpp"
#include "Base/Texture.hpp"
#include "Base/Sampler.hpp"
#include "Base/Buffer.hpp"
#include "Base/InputLayout.hpp"
#include "Base/PrimitiveType.hpp"
#include "Base/RasterState.hpp"
#include "Base/FDepthStencilState.hpp"
#include "Base/BlendState.hpp"

namespace Aurora
{
	class IRenderThreadCommand
	{
		IRenderThreadCommand& operator=(const IRenderThreadCommand& other); //undefined
	protected:
		//The user cannot delete this
		virtual ~IRenderThreadCommand() = default;;
	public:
		//execute the operation
		virtual void execute() = 0;
		//the caller is finished with this object and it can be destroyed
		virtual void dispose() = 0;
		//do both
		virtual void executeAndDispose() = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// Basic Types
	//////////////////////////////////////////////////////////////////////////

	struct Viewport
	{
		float minX, maxX;
		float minY, maxY;
		float minZ, maxZ;

		Viewport() : minX(0.f), maxX(0.f), minY(0.f), maxY(0.f), minZ(0.f), maxZ(1.f) { }
		Viewport(float width, float height) : minX(0.f), maxX(width), minY(0.f), maxY(height), minZ(0.f), maxZ(1.f) { }
		Viewport(float _minX, float _maxX, float _minY, float _maxY, float _minZ, float _maxZ) : minX(_minX), maxX(_maxX), minY(_minY), maxY(_maxY), minZ(_minZ), maxZ(_maxZ) { }
	};

	struct Rect
	{
		int minX, maxX;
		int minY, maxY;

		Rect() : minX(0), maxX(0), minY(0), maxY(0) { }
		Rect(int width, int height) : minX(0), maxX(width), minY(0), maxY(height) { }
		Rect(int _minX, int _maxX, int _minY, int _maxY) : minX(_minX), maxX(_maxX), minY(_minY), maxY(_maxY) { }
	};

	//////////////////////////////////////////////////////////////////////////
	// Texture
	//////////////////////////////////////////////////////////////////////////

	class Texture;
	typedef Texture* TextureHandle;

	//////////////////////////////////////////////////////////////////////////
	// Input Layout
	//////////////////////////////////////////////////////////////////////////

	class InputLayout;
	typedef InputLayout* InputLayoutHandle;

	//////////////////////////////////////////////////////////////////////////
	// Buffer
	//////////////////////////////////////////////////////////////////////////

	class Buffer;
	typedef Buffer* BufferHandle;

	//////////////////////////////////////////////////////////////////////////
	// Constant Buffer
	//////////////////////////////////////////////////////////////////////////

	class UniformBuffer;
	typedef UniformBuffer* UniformBufferHandle;

	struct UniformBufferDesc
	{
		uint32_t ByteSize;
		std::string DebugName;

		UniformBufferDesc(uint32_t size, const char* name) : ByteSize(size), DebugName(name) {}
		UniformBufferDesc() : ByteSize(0), DebugName() { }
	};

	//////////////////////////////////////////////////////////////////////////
	// Blend State
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Sampler
	//////////////////////////////////////////////////////////////////////////

	class Sampler;
	typedef Sampler* SamplerHandle;

	//////////////////////////////////////////////////////////////////////////
	// Render State (used by DrawCallState)
	//////////////////////////////////////////////////////////////////////////

	struct RenderState
	{
		enum { MAX_RENDER_TARGETS = 8, MAX_VIEWPORTS = 16 };

		uint32_t targetCount;
		TextureHandle targets[MAX_RENDER_TARGETS]{};
		uint32_t targetIndicies[MAX_RENDER_TARGETS]{}; // this is the array index, 3d-z, coord or cube face. For cube arrays, slice is /6, face is %6
		uint32_t targetMipSlices[MAX_RENDER_TARGETS]{};

		//These are in pixels
		uint32_t viewportCount;
		Viewport viewports[MAX_VIEWPORTS];
		Rect     scissorRects[MAX_VIEWPORTS];

		TextureHandle depthTarget;
		uint32_t depthIndex;
		uint32_t depthMipSlice;

		Color clearColor;
		float clearDepth;
		uint8_t clearStencil;

		bool clearColorTarget, clearDepthTarget, clearStencilTarget;

		// This flag is used by VXGI on OpenGL and it indicates that the rendering backend should call
		// IGlobalIllumination::setupExtraVoxelizationState() when applying this render state.
		bool setupExtraVoxelizationState;

		FBlendState blendState;
		FDepthStencilState depthStencilState;
		RasterState rasterState;

		RenderState() : targetCount(0), viewportCount(0), clearColor(0, 0, 0, 0), clearDepth(1.0f), clearStencil(0),
						clearColorTarget(false), clearDepthTarget(false), clearStencilTarget(false),
						depthTarget(nullptr), depthIndex(0), depthMipSlice(0),
						setupExtraVoxelizationState(false)
		{
			memset(targets, 0, sizeof(targets));
			memset(targetIndicies, 0, sizeof(targetIndicies));
			memset(targetMipSlices, 0, sizeof(targetMipSlices));
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// Bindings
	//////////////////////////////////////////////////////////////////////////

	struct ConstantBufferBinding
	{
		UniformBufferHandle buffer;
		uint32_t slot;
	};

	struct SamplerBinding
	{
		SamplerHandle sampler;
		uint32_t slot;
	};

	struct TextureBinding
	{
		TextureHandle texture;
		uint32_t slot : 8;
		GraphicsFormat format : 8;
		uint32_t mipLevel : 8;
		bool isWritable : 1; // true if this is a UAV (DX) or image (GL)
	};

	struct BufferBinding
	{
		BufferHandle buffer;
		uint32_t slot : 8;
		GraphicsFormat format : 8;
		bool isWritable : 1; // true if this is a UAV (DX) or SSBO (GL)
	};

	struct VertexBufferBinding
	{
		BufferHandle buffer;
		uint32_t slot;
		uint32_t offset;
		uint32_t stride;
	};

	//////////////////////////////////////////////////////////////////////////
	// Draw State
	//////////////////////////////////////////////////////////////////////////

	struct PipelineStageBindings
	{
		enum { MAX_TEXTURE_BINDINGS = 128, MAX_SAMPLER_BINDINGS = 16, MAX_BUFFER_BINDINGS = 128, MAX_CB_BINDINGS = 15 };

		// This field helps other code identify which stage the binding set is intended for
		EShaderType stage;

		Shader_ptr shader;

		//If this state came from an IUserDefinedShaderSet this is the index of the permutation we are using in case the application needs it to find reflection data
		uint32_t     userDefinedShaderPermutationIndex;

		//Each texture has a corresponding sampler to ease GL vs. DX porting
		TextureBinding textures[MAX_TEXTURE_BINDINGS]{};
		uint32_t textureBindingCount;

		std::map<std::string, TextureHandle> BoundTextures;
		std::map<std::string, SamplerHandle> BoundSamplers;
		std::map<std::string, UniformBufferHandle> BoundUniformBuffers;

		SamplerBinding textureSamplers[MAX_SAMPLER_BINDINGS]{};
		uint32_t textureSamplerBindingCount;
		BufferBinding buffers[MAX_BUFFER_BINDINGS]{};
		uint32_t bufferBindingCount;
		ConstantBufferBinding constantBuffers[MAX_CB_BINDINGS]{};
		uint32_t constantBufferBindingCount;

		PipelineStageBindings()
				: stage(EShaderType::Pixel)
				, textureBindingCount(0)
				, textureSamplerBindingCount(0)
				, bufferBindingCount(0)
				, constantBufferBindingCount(0)
				, shader(nullptr)
				, userDefinedShaderPermutationIndex(0)
		{

		}

		explicit PipelineStageBindings(EShaderType _stage)
				: stage(_stage)
				, textureBindingCount(0)
				, textureSamplerBindingCount(0)
				, bufferBindingCount(0)
				, constantBufferBindingCount(0)
				, shader(nullptr)
				, userDefinedShaderPermutationIndex(0)
		{

		}

	};

	struct DrawCallState
	{
		enum { MAX_VERTEX_ATTRIBUTE_COUNT = 16 };

		EPrimitiveType primType;
		InputLayoutHandle inputLayout;
		BufferHandle indexBuffer;
		GraphicsFormat indexBufferFormat;
		uint32_t indexBufferOffset;

		PipelineStageBindings VS;
		PipelineStageBindings HS;
		PipelineStageBindings DS;
		PipelineStageBindings GS;
		PipelineStageBindings PS;

		uint32_t vertexBufferCount;
		VertexBufferBinding vertexBuffers[MAX_VERTEX_ATTRIBUTE_COUNT]{};

		RenderState renderState;

		DrawCallState()
				: VS(EShaderType::Vertex)
				, HS(EShaderType::Hull)
				, DS(EShaderType::Domain)
				, GS(EShaderType::Geometry)
				, PS(EShaderType::Pixel)
				, primType(EPrimitiveType::TriangleList)
				, inputLayout(nullptr)
				, indexBuffer(nullptr)
				, indexBufferFormat(GraphicsFormat::R32_UINT)
				, indexBufferOffset(0)
				, vertexBufferCount(0)
		{

		}
	};

	struct DispatchState : PipelineStageBindings
	{
		DispatchState() : PipelineStageBindings(EShaderType::Compute) {}
	};

	//////////////////////////////////////////////////////////////////////////
	// Misc
	//////////////////////////////////////////////////////////////////////////

	struct GraphicsAPI
	{
		enum Enum
		{
			D3D11,
			D3D12,
			OPENGL4
		};
	};

	class PerformanceQuery;
	typedef PerformanceQuery* PerformanceQueryHandle;

	struct DrawArguments
	{
		uint32_t vertexCount;
		uint32_t instanceCount;
		uint32_t startIndexLocation;
		uint32_t startVertexLocation;
		uint32_t startInstanceLocation;

		DrawArguments()
				: vertexCount(0)
				, instanceCount(1)
				, startIndexLocation(0)
				, startVertexLocation(0)
				, startInstanceLocation(0)
		{ }
	};

	// Should be implemented by the application.
	// Clients will call signalError(...) on every error it encounters, in addition to returning one of the
	// failure status codes. The application can display a message box in case of errors.
	class IErrorCallback
	{
		IErrorCallback& operator=(const IErrorCallback& other) = delete; //undefined
	protected:
		virtual ~IErrorCallback() = default;
	public:
		virtual void SignalError(const char* function, const char* file, int line, const char* errorDesc) = 0;
	};

	/**
	 * IRendererInterface
	 */
	class IRendererInterface
	{
	public:
		virtual ~IRendererInterface() = default;
		IRendererInterface& operator=(const IRendererInterface& other) = delete;
	public:
		virtual TextureHandle CreateTexture(const TextureDesc& d, const void* data) = 0;
		virtual TextureDesc DescribeTexture(TextureHandle t) = 0;
		virtual void ClearTextureFloat(TextureHandle t, const Color& clearColor) = 0;
		virtual void ClearTextureUInt(TextureHandle t, uint32_t clearColor) = 0;
		virtual void WriteTexture(TextureHandle t, uint32_t subresource, const void* data, uint32_t rowPitch, uint32_t depthPitch) = 0;
		virtual void DestroyTexture(TextureHandle t) = 0;

		virtual BufferHandle CreateBuffer(const BufferDesc& d, const void* data) = 0;
		virtual BufferDesc DescribeBuffer(BufferHandle b) = 0;
		virtual void WriteBuffer(BufferHandle b, const void* data, size_t dataSize) = 0;
		virtual void ClearBufferUInt(BufferHandle b, uint32_t clearValue) = 0;
		virtual void CopyToBuffer(BufferHandle dest, uint32_t destOffsetBytes, BufferHandle src, uint32_t srcOffsetBytes, size_t dataSizeBytes) = 0;
		virtual void ReadBuffer(BufferHandle b, void* data, size_t* dataSize) = 0; // for debugging purposes only
		virtual void DestroyBuffer(BufferHandle b) = 0;

		virtual UniformBufferHandle CreateUniformBuffer(const UniformBufferDesc& d, const void* data) = 0;
		virtual void WriteUniformBuffer(UniformBufferHandle b, const void* data, size_t dataSize, uint32_t offset) = 0;
		virtual void DestroyUniformBuffer(UniformBufferHandle b) = 0;
		virtual Shader_ptr CreateShader(const ShaderDesc& d, const void* binary, size_t binarySize) = 0;
		virtual void DestroyShader(Shader_ptr& s) = 0;

		virtual SamplerHandle CreateSampler(const SamplerDesc& d) = 0;
		virtual void DestroySampler(SamplerHandle s) = 0;

		virtual InputLayoutHandle CreateInputLayout(const VertexAttributeDesc* d, uint32_t attributeCount, const void* vertexShaderBinary, size_t binarySize) = 0;
		virtual void CestroyInputLayout(InputLayoutHandle i) = 0;

		// Returns the API kind that the RHI backend is running on top of.
		virtual GraphicsAPI::Enum GetGraphicsAPI() = 0;

		virtual bool IsOpenGLExtensionSupported(const char* name) = 0;

		// Try to get the address of openGL procedure specified by procname; return 0 if unsupported or not GL.
		virtual void* GetOpenGLProcAddress(const char* procname) = 0;

		virtual void Draw(const DrawCallState& state, const DrawArguments* args, uint32_t numDrawCalls) = 0;
		virtual void DrawIndexed(const DrawCallState& state, const DrawArguments* args, uint32_t numDrawCalls) = 0;
		virtual void DrawIndirect(const DrawCallState& state, BufferHandle indirectParams, uint32_t offsetBytes) = 0;

		virtual void Dispatch(const DispatchState& state, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) = 0;
		virtual void DispatchIndirect(const DispatchState& state, BufferHandle indirectParams, uint32_t offsetBytes) = 0;

		virtual void ExecuteRenderThreadCommand(IRenderThreadCommand* onCommand) = 0;

		virtual void RestoreDefaultState() = 0;
	};

}
#pragma clang diagnostic pop