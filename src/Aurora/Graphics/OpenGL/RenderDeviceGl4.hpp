/*
* Copyright (c) 2012-2016, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#pragma once

#include "../IRenderDevice.hpp"
#include "OpenGLShader.hpp"

#include <vector>
#include <map>
#include <glad/glad.h>

#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

namespace Aurora
{
	struct FormatMapping
	{
		Format::Enum abstractFormat;
		GLenum internalFormat;
		GLenum baseFormat;
		GLenum type;
		uint32_t components;
		uint32_t bytesPerPixel;
		bool isDepthStencil;
	};

	// Format mapping table. The rows must be in the exactly same order as Format enum members are defined.
	// https://www.khronos.org/opengles/sdk/docs/man31/html/glTexImage2D.xhtml
	static const FormatMapping FormatMappings[] = {
			{ Format::UNKNOWN,              0,                      0,                  0,                              0, 0, false },
			{ Format::R8_UINT,              GL_R8UI,                GL_RED_INTEGER,     GL_UNSIGNED_BYTE,               1, 1, false },
			{ Format::R8_UNORM,             GL_R8,                  GL_RED,             GL_UNSIGNED_BYTE,               1, 1, false },
			{ Format::RG8_UINT,             GL_RG8UI,               GL_RG_INTEGER,      GL_UNSIGNED_BYTE,               2, 2, false },
			{ Format::RG8_UNORM,            GL_RG8,                 GL_RG,              GL_UNSIGNED_BYTE,               2, 2, false },
			{ Format::R16_UINT,             GL_R16UI,               GL_RED_INTEGER,     GL_SHORT,                       1, 2, false },
			{ Format::R16_UNORM,            GL_R16,                 GL_RED,             GL_HALF_FLOAT,                  1, 2, false },
			{ Format::R16_FLOAT,            GL_R16F,                GL_RED,             GL_HALF_FLOAT,                  1, 2, false },
			{ Format::RGBA8_UNORM,          GL_RGBA8,               GL_RGBA,            GL_UNSIGNED_BYTE,               4, 4, false },
			{ Format::BGRA8_UNORM,          GL_RGBA8,               GL_BGRA,            GL_UNSIGNED_BYTE,               4, 4, false },
			{ Format::SRGBA8_UNORM,         GL_RGBA8,               GL_RGBA,            GL_UNSIGNED_BYTE,               4, 4, false },
			{ Format::R10G10B10A2_UNORM,    GL_RGB10_A2,            GL_RGBA,            GL_UNSIGNED_INT_2_10_10_10_REV, 4, 4, false },
			{ Format::R11G11B10_FLOAT,      GL_R11F_G11F_B10F,      GL_RGB,             GL_UNSIGNED_INT_10F_11F_11F_REV,4, 4, false },
			{ Format::RG16_UINT,            GL_RG16UI,              GL_RG_INTEGER,      GL_SHORT,                       2, 4, false },
			{ Format::RG16_FLOAT,           GL_RG16F,               GL_RG,              GL_HALF_FLOAT,                  2, 4, false },
			{ Format::R32_UINT,             GL_R32UI,               GL_RED_INTEGER,     GL_UNSIGNED_INT,                1, 4, false },
			{ Format::R32_FLOAT,            GL_R32F,                GL_RED,             GL_FLOAT,                       1, 4, false },
			{ Format::RGBA16_FLOAT,         GL_RGBA16F,             GL_RGBA,            GL_HALF_FLOAT,                  4, 8, false },
			{ Format::RGBA16_UNORM,         GL_RGBA16,              GL_RGBA,            GL_UNSIGNED_INT,                4, 8, false },
			{ Format::RGBA16_SNORM,         GL_RGBA16_SNORM,        GL_RGBA,            GL_INT,                         4, 8, false },
			{ Format::RG32_UINT,            GL_RG32UI,              GL_RG_INTEGER,      GL_UNSIGNED_INT,                2, 8, false },
			{ Format::RG32_FLOAT,           GL_RG32F,               GL_RG,              GL_FLOAT,                       2, 8, false },
			{ Format::RGB32_UINT,           GL_RGB32UI,             GL_RGB_INTEGER,     GL_UNSIGNED_INT,                3, 12, false },
			{ Format::RGB32_FLOAT,          GL_RGB32F,              GL_RGB,             GL_FLOAT,                       3, 12, false },
            { Format::RGBA16_UINT,          GL_RGBA16UI,            GL_RGBA_INTEGER,    GL_UNSIGNED_INT,                4, 16, false },
			{ Format::RGBA32_UINT,          GL_RGBA32UI,            GL_RGBA_INTEGER,    GL_UNSIGNED_INT,                4, 16, false },
			{ Format::RGBA32_FLOAT,         GL_RGBA32F,             GL_RGBA,            GL_FLOAT,                       4, 16, false },
			{ Format::D16,                  GL_DEPTH_COMPONENT16,   GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT,              1, 2, true },
			{ Format::D24S8,                GL_DEPTH24_STENCIL8,    GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8,           2, 4, true },
			{ Format::X24G8_UINT,           GL_DEPTH24_STENCIL8,    GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8,           2, 4, true },
			{ Format::D32,                  GL_DEPTH_COMPONENT32,   GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,                1, 4, true },
	};

    class FrameBuffer;

    class RendererInterfaceOGL : public IRendererInterface
    {
    public:
        explicit RendererInterfaceOGL(IErrorCallback* pErrorCallback);
        ~RendererInterfaceOGL() override;

        void                    init();

        void*                   getAPISpecificInterface(APISpecificInterface::Enum) override { return nullptr; }
        bool                    isOpenGLExtensionSupported(const char* name) override;
        void*                   getOpenGLProcAddress(const char* procname) override;

        TextureHandle           createTexture(const TextureDesc& d, const void* data) override;
        TextureDesc             describeTexture(TextureHandle t) override;
        void                    clearTextureFloat(TextureHandle t, const Color& clearColor) override;
        void                    clearTextureUInt(TextureHandle t, uint32_t clearColor) override;
        void                    writeTexture(TextureHandle t, uint32_t subresource, const void* data, uint32_t rowPitch, uint32_t depthPitch) override;
        void                    destroyTexture(TextureHandle t) override;

        BufferHandle            createBuffer(const BufferDesc& d, const void* data) override;
		BufferDesc describeBuffer(BufferHandle b) override;
        void                    writeBuffer(BufferHandle b, const void* data, size_t dataSize) override;
        void                    clearBufferUInt(BufferHandle b, uint32_t clearValue) override;
        void                    copyToBuffer(BufferHandle dest, uint32_t destOffsetBytes, BufferHandle src, uint32_t srcOffsetBytes, size_t dataSizeBytes) override;
        void                    readBuffer(BufferHandle b, void* data, size_t* dataSize) override; // for debugging purposes only
        void                    destroyBuffer(BufferHandle b) override;

        ConstantBufferHandle    createConstantBuffer(const ConstantBufferDesc& d, const void* data) override;
        void                    writeConstantBuffer(ConstantBufferHandle b, const void* data, size_t dataSize) override;
        void                    destroyConstantBuffer(ConstantBufferHandle b) override;

		Shader_ptr            createShader(const ShaderDesc& d, const void* binary, size_t binarySize) override;
		Shader_ptr            createShaderFromAPIInterface(ShaderType, const void*) override { return nullptr; }
        void                    destroyShader(Shader_ptr& s) override;

        SamplerHandle           createSampler(const SamplerDesc& d) override;
        void                    destroySampler(SamplerHandle s) override;

        InputLayoutHandle       createInputLayout(const VertexAttributeDesc* d, uint32_t attributeCount, const void* vertexShaderBinary, size_t binarySize) override;
        void                    destroyInputLayout(InputLayoutHandle i) override;

        PerformanceQueryHandle  createPerformanceQuery(const char*) override { return nullptr; }
        void                    destroyPerformanceQuery(PerformanceQueryHandle) override { }
        void                    beginPerformanceQuery(PerformanceQueryHandle, bool) override { }
        void                    endPerformanceQuery(PerformanceQueryHandle) override { }
        float                   getPerformanceQueryTimeMS(PerformanceQueryHandle) override { return 0.f; }

        GraphicsAPI::Enum       getGraphicsAPI() override { return GraphicsAPI::OPENGL4; };

        void                    draw(const DrawCallState& state, const DrawArguments* args, uint32_t numDrawCalls) override;
        void                    drawIndexed(const DrawCallState& state, const DrawArguments* args, uint32_t numDrawCalls) override;
        void                    drawIndirect(const DrawCallState& state, BufferHandle indirectParams, uint32_t offsetBytes) override;

        void                    dispatch(const DispatchState& state, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) override;
        void                    dispatchIndirect(const DispatchState& state, BufferHandle indirectParams, uint32_t offsetBytes) override;

        void                    executeRenderThreadCommand(IRenderThreadCommand* onCommand) override;

        uint32_t                getNumberOfAFRGroups() override { return 1; }
        uint32_t                getAFRGroupOfCurrentFrame(uint32_t) override { return 0; }
        void                    setEnableUavBarriersForTexture(TextureHandle, bool) override { }
        void                    setEnableUavBarriersForBuffer(BufferHandle, bool) override { }

        void                    ApplyState(const DrawCallState& state);
        void                    RestoreDefaultState() override;
        void                    UnbindFrameBuffer();

        TextureHandle           getHandleForDefaultBackBuffer() { return m_DefaultBackBuffer; }
        TextureHandle           getHandleForTexture(uint32_t target, uint32_t texture);
        uint32_t                getTextureOpenGLName(TextureHandle t);
        void                    releaseNonManagedTextures();

    protected:

        IErrorCallback*         m_pErrorCallback;

        uint32_t                m_nGraphicsPipeline;
        uint32_t                m_nComputePipeline;
        uint32_t                m_nVAO;

        // state cache
        std::vector<uint32_t>   m_vecBoundSamplers;
        std::vector<uint32_t>   m_vecBoundConstantBuffers;
        std::vector<uint32_t>   m_vecBoundBuffers;
        std::vector<uint32_t>   m_vecBoundImages;
        std::vector<std::pair<uint32_t, uint32_t> > m_vecBoundTextures;
        bool                    m_bConservativeRasterEnabled;
        bool                    m_bForcedSampleCountEnabled;

        std::map<uint32_t, FrameBuffer*> m_CachedFrameBuffers;
        std::vector<TextureHandle> m_NonManagedTextures;
        TextureHandle           m_DefaultBackBuffer;
        FrameBuffer*            m_pCurrentFrameBuffer;
        Viewport         m_vCurrentViewports[16];
        Rect             m_vCurrentScissorRects[16];
        bool                    m_bCurrentViewportsValid{};

        FrameBuffer*            GetCachedFrameBuffer(const RenderState& state);

        void                    BindVAO();
        void                    BindRenderTargets(const RenderState& renderState);
        void                    ClearRenderTargets(const RenderState& renderState);
        void                    SetRasterState(const RasterState& rasterState);
        void                    SetBlendState(const BlendState& blendState, uint32_t targetCount);
        void                    SetDepthStencilState(const DepthStencilState& depthState);
        void                    SetShaders(const DrawCallState& state);
        void                    BindShaderResources(const PipelineStageBindings& state);
        void                    BindShaderResources(const DrawCallState& state);

        void                    ApplyState(const DispatchState& state);

        void                    checkGLError(const char* file, int line);

        uint32_t                convertStencilOp(DepthStencilState::StencilOp value);
        uint32_t                convertComparisonFunc(DepthStencilState::ComparisonFunc value);
        uint32_t                convertPrimType(PrimitiveType primType);
        uint32_t                convertWrapMode(SamplerDesc::WrapMode in_eMode);
        uint32_t                convertBlendValue(BlendState::BlendValue value);
        uint32_t                convertBlendOp(BlendState::BlendOp value);

    public:
		GLuint getTextureHandle(Texture* t);
    };
}