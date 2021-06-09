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

#if 0

#include "../IRenderDeviceNV.hpp"
#include "GLFormatMapping.hpp"
#include "GLShaderProgram.hpp"

#include <vector>
#include <map>
#include <glad/glad.h>

#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

namespace Aurora
{
    class FrameBuffer;

    class RendererInterfaceOGL : public IRendererInterface
    {
    public:
        explicit RendererInterfaceOGL(IErrorCallback* pErrorCallback);
        ~RendererInterfaceOGL() override;

        void                    init();

        bool                    IsOpenGLExtensionSupported(const char* name) override;
        void*                   GetOpenGLProcAddress(const char* procname) override;

        TextureHandle           CreateTexture(const TextureDesc& d, const void* data) override;
        TextureDesc             DescribeTexture(TextureHandle t) override;
        void                    ClearTextureFloat(TextureHandle t, const Color& clearColor) override;
        void                    ClearTextureUInt(TextureHandle t, uint32_t clearColor) override;
        void                    WriteTexture(TextureHandle t, uint32_t subresource, const void* data, uint32_t rowPitch, uint32_t depthPitch) override;
        void                    DestroyTexture(TextureHandle t) override;

        BufferHandle            CreateBuffer(const BufferDesc& d, const void* data) override;
		BufferDesc DescribeBuffer(BufferHandle b) override;
        void                    WriteBuffer(BufferHandle b, const void* data, size_t dataSize) override;
        void                    ClearBufferUInt(BufferHandle b, uint32_t clearValue) override;
        void                    CopyToBuffer(BufferHandle dest, uint32_t destOffsetBytes, BufferHandle src, uint32_t srcOffsetBytes, size_t dataSizeBytes) override;
        void                    ReadBuffer(BufferHandle b, void* data, size_t* dataSize) override; // for debugging purposes only
        void                    DestroyBuffer(BufferHandle b) override;

        UniformBufferHandle    CreateUniformBuffer(const UniformBufferDesc& d, const void* data) override;
        void                    WriteUniformBuffer(UniformBufferHandle b, const void* data, size_t dataSize, uint32_t offset) override;
        void                    DestroyUniformBuffer(UniformBufferHandle b) override;

		Shader_ptr            CreateShader(const ShaderDesc& d, const void* binary, size_t binarySize) override;
        void                    DestroyShader(Shader_ptr& s) override;

        SamplerHandle           CreateSampler(const SamplerDesc& d) override;
        void                    DestroySampler(SamplerHandle s) override;

        InputLayoutHandle       CreateInputLayout(const VertexAttributeDesc* d, uint32_t attributeCount, const void* vertexShaderBinary, size_t binarySize) override;
        void                    CestroyInputLayout(InputLayoutHandle i) override;

        GraphicsAPI::Enum       GetGraphicsAPI() override { return GraphicsAPI::OPENGL4; };

        void                    Draw(const DrawCallState& state, const DrawArguments* args, uint32_t numDrawCalls) override;
        void                    DrawIndexed(const DrawCallState& state, const DrawArguments* args, uint32_t numDrawCalls) override;
        void                    DrawIndirect(const DrawCallState& state, BufferHandle indirectParams, uint32_t offsetBytes) override;

        void                    Dispatch(const DispatchState& state, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) override;
        void                    DispatchIndirect(const DispatchState& state, BufferHandle indirectParams, uint32_t offsetBytes) override;

        void                    ExecuteRenderThreadCommand(IRenderThreadCommand* onCommand) override;

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

        void                    checkGLError(const char* function, const char* file, int line);

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
#endif