#pragma once

#include <Aurora/Core/String.hpp>
#include "ShaderCollection.hpp"

#include <RefCntAutoPtr.hpp>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <PipelineState.h>
#include <MapHelper.hpp>

#include "../AuroraEngine.hpp"

using namespace Diligent;

namespace Aurora::Render
{
    class FRenderInterface;

    class FMaterial
    {
    private:
        struct PipelineStateData
        {
            RefCntAutoPtr<IPipelineState> State;
            RefCntAutoPtr<IShaderResourceBinding> ResourceBinding;
        };

        struct ShaderConstantBuffer
        {
            List<SHADER_TYPE> ShaderTypes;
            uint32_t Size;
            RefCntAutoPtr<IBuffer> Buffer;
            BufferDesc Desc;
        };

        struct ShaderTextureDef
        {
            List<SHADER_TYPE> ShaderTypes;
            ITextureView* TextureView;
            bool NeedsUpdate;
        };
    private:
        String m_Name;
        FShaderCollectionPtr m_ShaderCollection;

        RefCntAutoPtr<IPipelineState> m_CurrentPipelineState;
        RefCntAutoPtr<IShaderResourceBinding> m_CurrentResourceBinding;
        Map<uint32_t, PipelineStateData> m_PipelineStates;
        uint32_t m_CurrentPipelineStateHash;
        GraphicsPipelineStateCreateInfo m_PSOCreateInfo;

        Map<String, ShaderConstantBuffer> m_ShaderConstantBuffers;

        List<ShaderResourceVariableDesc> m_ShaderResourceVariables;
        List<ImmutableSamplerDesc> m_ShaderResourceSamplers;

        Map<String, ShaderTextureDef> m_ShaderTextures;
    public:
        explicit FMaterial(const String& name, FShaderCollectionPtr shaderCollection);
        ~FMaterial() = default;
    private:
        void InitShaderResources(const RefCntAutoPtr<IShader>& shader);
        void AssignShaders();
    public:
        template <typename DataType, bool KeepStrongReferences = false>
        bool GetConstantBuffer(const String& name, MapHelper<DataType, KeepStrongReferences>& map)
        {
            if(m_ShaderConstantBuffers.find(name) == m_ShaderConstantBuffers.end()) {
                return false;
            }

            map = MapHelper<DataType, KeepStrongReferences>(AuroraEngine::ImmediateContext, m_ShaderConstantBuffers[name].Buffer, MAP_WRITE, MAP_FLAG_DISCARD);

            return true;
        }

        void ValidateGraphicsPipelineState();

        void ApplyPipeline();
        void CommitShaderResources();

        GraphicsPipelineDesc& GetPipelineDesc();

        inline GraphicsPipelineStateCreateInfo& GetGraphicsPipelineStateCreateInfo()
        {
            return m_PSOCreateInfo;
        }

        [[nodiscard]] inline uint32_t GetHash() const noexcept
        {
            return m_CurrentPipelineStateHash;
        }

        inline RefCntAutoPtr<IPipelineState>& GetCurrentPipelineState()
        {
            return m_CurrentPipelineState;
        }

        inline RefCntAutoPtr<IShaderResourceBinding>& GetCurrentResourceBinding()
        {
            return m_CurrentResourceBinding;
        }

    public:
        inline void SetSampler(const String& textureName, const SamplerDesc& sampler)
        {
            for (auto& m_ShaderResourceSampler : m_ShaderResourceSamplers) {
                m_ShaderResourceSampler.Desc = sampler;
            }
        }

        inline void SetTexture(const String& name, RefCntAutoPtr<ITexture>& texture)
        {
            if(texture == nullptr) {
                std::cerr << "Null texture " << name << " in " << m_Name << std::endl;
                return;
            }

            for (auto& var : m_ShaderTextures) {
                var.second.TextureView = texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
                var.second.NeedsUpdate = true;
            }
        }
    public:
        inline void SetFillMode(const FILL_MODE& fillMode) noexcept
        {
            m_PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FillMode = fillMode;
        }

        inline void SetCullMode(const CULL_MODE& cullMode) noexcept
        {
            m_PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = cullMode;
        }

        inline void SetBlendStateForRenderTarget(int index, const RenderTargetBlendDesc& blendDesc)
        {
            m_PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[index] = blendDesc;
        }

        inline void SetBlendState(const RenderTargetBlendDesc& blendDesc)
        {
            SetBlendStateForRenderTarget(0, blendDesc);
        }

        inline void SetIndependentBlend(bool flag) noexcept
        {
            m_PSOCreateInfo.GraphicsPipeline.BlendDesc.IndependentBlendEnable = flag;
        }

        inline void SetPrimitiveTopology(const PRIMITIVE_TOPOLOGY& primitiveTopology) noexcept
        {
            m_PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = primitiveTopology;
        }

        inline void SetDepthEnable(bool enabled) noexcept
        {
            m_PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = enabled;
        }

        inline void SetDepthWriteEnable(bool enabled) noexcept
        {
            m_PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = enabled;
        }

        inline void SetDepthFunc(const COMPARISON_FUNCTION& comparisonFunction) noexcept
        {
            m_PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthFunc = comparisonFunction;
        }
    private:
        static uint32_t GetGraphicsPipelineStateCreateInfoHash(Diligent::GraphicsPipelineStateCreateInfo& gpsci);
    };
    DEFINE_PTR(FMaterial)
}