#include "Material.hpp"
#include "Aurora/Core/Crc.hpp"

#include <GraphicsAccessories.hpp>
#include "GraphicUtilities.hpp"

namespace Aurora
{
	Material::Material(const String& name, ShaderCollection_ptr shaderCollection)
			: m_Name(name), m_ShaderCollection(std::move(shaderCollection)), m_CurrentPipelineState(nullptr), m_CurrentResourceBinding(nullptr), m_CurrentPipelineStateHash(0)
	{
		InitShaderResources(m_ShaderCollection->Vertex);
		InitShaderResources(m_ShaderCollection->Pixel);
		InitShaderResources(m_ShaderCollection->Domain);
		InitShaderResources(m_ShaderCollection->Hull);
		InitShaderResources(m_ShaderCollection->Geometry);
		InitShaderResources(m_ShaderCollection->Amplification);
		InitShaderResources(m_ShaderCollection->Mesh);

		AssignShaders();

		m_PSOCreateInfo.PSODesc.Name = name.c_str();
		m_PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS; //TODO: check by attached shaders if compute PIPELINE_TYPE_COMPUTE
	}

	void Material::InitShaderResources(const RefCntAutoPtr<IShader>& shader)
	{
		if(shader == nullptr) {
			return;
		}

		m_PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

		std::vector<StateTransitionDesc> barriers;

		for (int i = 0; i < shader->GetResourceCount(); ++i) {
			ShaderResourceDesc desc;
			shader->GetResourceDesc(i, desc);

			const SHADER_TYPE& shaderType = shader->GetDesc().ShaderType;

			switch (desc.Type) {
				case SHADER_RESOURCE_TYPE_CONSTANT_BUFFER: {
					auto iter = m_ShaderConstantBuffers.find(desc.Name);
					if(iter != m_ShaderConstantBuffers.end()) {
						auto& constBufferData = iter->second;

						if(constBufferData.Size == desc.Size) {
							constBufferData.ShaderTypes.push_back(shaderType);
							continue;
						}

						AU_THROW_ERROR("Found two different constant buffers in material: " << m_Name);
					} else {
						ShaderConstantBuffer shaderConstantBufferInfo = {};
						shaderConstantBufferInfo.Size = desc.Size;
						shaderConstantBufferInfo.Buffer = nullptr;
						shaderConstantBufferInfo.ShaderTypes.push_back(shaderType);
						shaderConstantBufferInfo.BufferData.resize(desc.Size);
						shaderConstantBufferInfo.NeedsUpdate = true;

						BufferDesc CBDesc;
						CBDesc.Name           = ("Constant buffer " + String(desc.Name) + "for shader " + m_Name).c_str();
						CBDesc.uiSizeInBytes  = desc.Size;
						CBDesc.Usage          = USAGE_DYNAMIC;
						CBDesc.BindFlags      = BIND_UNIFORM_BUFFER;
						CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;

						shaderConstantBufferInfo.Desc = CBDesc;

						if(desc.Variables != nullptr) {
							const std::vector<ShaderVariable>& variables = *desc.Variables;

							for (int j = 0; j < variables.size(); ++j) {
								const ShaderVariable& var = variables[j];

								std::cout << desc.Name << " - " << var.Name << ":" << var.Size << ":" << var.ArrayStride << ":" << var.MatrixStride << std::endl;
							}

							shaderConstantBufferInfo.Variables = variables;
						} else {
							// TODO: throw exception and exit program
						}

						AuroraEngine::RenderDevice->CreateBuffer(shaderConstantBufferInfo.Desc, nullptr, &shaderConstantBufferInfo.Buffer);
						barriers.emplace_back(shaderConstantBufferInfo.Buffer, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true);

						m_ShaderConstantBuffers[desc.Name] = shaderConstantBufferInfo;

						std::cout << "Initialized constant buffer " << desc.Name << "(" << GetShaderTypeLiteralName(shaderType) << ") for material " << m_Name << " with buffer: " << shaderConstantBufferInfo.Buffer.RawPtr() << std::endl;
					}
					break;
				}
				case SHADER_RESOURCE_TYPE_SAMPLER: {
					std::cout << "Sampler for: " << desc.Name << std::endl;
					break;
				}
				case SHADER_RESOURCE_TYPE_TEXTURE_SRV: {
					m_ShaderResourceVariables.emplace_back(shaderType, desc.Name, SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC);

					SamplerDesc SamLinearClampDesc
							{
									FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
									TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP
							};
					m_ShaderResourceSamplers.emplace_back(shaderType, desc.Name, SamLinearClampDesc);

					if(m_ShaderTextures.find(desc.Name) != m_ShaderTextures.end()) {
						m_ShaderTextures[desc.Name].ShaderTypes.push_back(shaderType);
					} else {
						m_ShaderTextures[desc.Name] = {{shaderType}, GraphicUtilities::GetPlaceholderTexture(), GraphicUtilities::GetPlaceholderTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), true};
					}

					std::cout << "Texture: " << desc.Name << std::endl;
					break;
				}
				case SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT: {
					std::cout << "Input: " << desc.Name << std::endl;
					break;
				}
				default:
					std::cout << "Unimplemented resource type" << std::endl;
					//TODO: Complete other buffers and UAV
					continue;
			}
		}

		if(!barriers.empty()) {
			AuroraEngine::ImmediateContext->TransitionResourceStates(barriers.size(), barriers.data());
		}
	}

	void Material::AssignShaders()
	{
		m_PSOCreateInfo.pVS = m_ShaderCollection->Vertex;
		m_PSOCreateInfo.pPS = m_ShaderCollection->Pixel;
		m_PSOCreateInfo.pDS = m_ShaderCollection->Domain;
		m_PSOCreateInfo.pHS = m_ShaderCollection->Hull;
		m_PSOCreateInfo.pGS = m_ShaderCollection->Geometry;
		m_PSOCreateInfo.pAS = m_ShaderCollection->Amplification;
		m_PSOCreateInfo.pMS = m_ShaderCollection->Mesh;
	}

	/**
	 * Do not call this method on your own !
	 * if you have vertex shader with some input and you did not provided input layout,
	 * the program will behave unexpectedly and some graphics will result in black screen
	 * You can check when it is happening through Vulkan validation layer and break points.
	 */
	void Material::ValidateGraphicsPipelineState()
	{
		for(auto& it : m_ShaderConstantBuffers) {
			it.second.NeedsUpdate = true;
		}

		// Setup PSO
		m_PSOCreateInfo.PSODesc.ResourceLayout.Variables = m_ShaderResourceVariables.data();
		m_PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = m_ShaderResourceVariables.size();

		m_PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = m_ShaderResourceSamplers.data();
		m_PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = m_ShaderResourceSamplers.size();

		// Update pipeline
		uint32_t hash = GetGraphicsPipelineStateCreateInfoHash(m_PSOCreateInfo);

		if(m_CurrentPipelineStateHash == hash) {
			return;
		}

		if(m_PipelineStates.find(hash) != m_PipelineStates.end()) {
			auto& data = m_PipelineStates[hash];
			m_CurrentPipelineState = data.State;
			m_CurrentResourceBinding = data.ResourceBinding;
			m_CurrentPipelineStateHash = hash;
			return;
		}

		m_CurrentPipelineState.Release();

		GraphicsPipelineStateCreateInfo infoCopy(m_PSOCreateInfo);
		AuroraEngine::RenderDevice->CreateGraphicsPipelineState(infoCopy, &m_CurrentPipelineState);

		// Bind constant buffers
		for(auto& it : m_ShaderConstantBuffers) {
			String constantBufferName = it.first;
			ShaderConstantBuffer& shaderConstantBuffer = it.second;

			for(auto& shaderType : shaderConstantBuffer.ShaderTypes) {
				std::cout << constantBufferName << " - " << m_Name << "(" << GetShaderTypeLiteralName(shaderType) << ") - " << shaderConstantBuffer.Buffer.RawPtr() << std::endl;
				m_CurrentPipelineState->GetStaticVariableByName(shaderType, constantBufferName.c_str())->Set(shaderConstantBuffer.Buffer);
			}
		}

		PipelineStateData stateData = {};
		stateData.State = m_CurrentPipelineState;
		m_CurrentPipelineState->CreateShaderResourceBinding(&stateData.ResourceBinding, true);

		m_CurrentResourceBinding = stateData.ResourceBinding;
		m_CurrentPipelineStateHash = hash;
		m_PipelineStates[hash] = stateData;
	}

	void Material::ApplyPipeline()
	{
		AuroraEngine::ImmediateContext->SetPipelineState(GetCurrentPipelineState());
	}

	void Material::CommitShaderResources()
	{
		for (auto& var : m_ShaderTextures) {
			if(!var.second.NeedsUpdate) continue;
			if(var.second.TextureView == nullptr) {
				std::cerr << "Cannot commit resources with empty texture: " << var.first << std::endl;
				continue;
			}

			for (auto& shaderType : var.second.ShaderTypes) {
				GetCurrentResourceBinding()->GetVariableByName(shaderType, var.first.c_str())->Set(var.second.TextureView);
			}
		}

		for(auto& it : m_ShaderConstantBuffers) {
			const String& name = it.first;
			ShaderConstantBuffer& buffer = it.second;

			if(!buffer.NeedsUpdate) {
				continue;
			}

			// TODO: Find out whats best for performance
			if(false) {
				// Option one: update buffer
				// This is not working !
				StateTransitionDesc barrier = {buffer.Buffer, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true};
				AuroraEngine::ImmediateContext->UpdateBuffer(buffer.Buffer, 0, buffer.Size, buffer.BufferData.data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
				AuroraEngine::ImmediateContext->TransitionResourceStates(1, &barrier);
			} else {
				// Option two map raw buffer to variable and copy data
				uint8_t* mappedData = nullptr;
				AuroraEngine::ImmediateContext->MapBuffer(buffer.Buffer, MAP_WRITE, MAP_FLAG_DISCARD, (PVoid&)mappedData);
				memcpy(mappedData, buffer.BufferData.data(), buffer.Size);
				AuroraEngine::ImmediateContext->UnmapBuffer(buffer.Buffer, MAP_WRITE);
			}

			buffer.NeedsUpdate = false;

			/*for(auto& shaderType : buffer.ShaderTypes) {
				GetCurrentResourceBinding()->GetVariableByName(shaderType, name.c_str())->Set(buffer.Buffer);
			}*/
		}

		AuroraEngine::ImmediateContext->CommitShaderResources(GetCurrentResourceBinding(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	GraphicsPipelineDesc& Material::GetPipelineDesc()
	{
		return m_PSOCreateInfo.GraphicsPipeline;
	}

	uint32_t Material::GetGraphicsPipelineStateCreateInfoHash(Diligent::GraphicsPipelineStateCreateInfo& gpsci)
	{
		CrcHash hasher;

		Diligent::GraphicsPipelineDesc& gpd = gpsci.GraphicsPipeline;

		// Hash blend state
		hasher.Add(gpd.BlendDesc);

		// Hash raster state
		hasher.Add(gpd.RasterizerDesc);

		// Hash depth stencil
		hasher.Add(gpd.DepthStencilDesc);

		// Hash input layout
		Diligent::InputLayoutDesc& inputLayout = gpd.InputLayout;
		hasher.Add(inputLayout.NumElements);
		for (int i = 0; i < inputLayout.NumElements; ++i) {
			hasher.Add(inputLayout.LayoutElements[i]);
		}

		// Hash other things
		hasher.Add(gpd.PrimitiveTopology);
		hasher.Add(gpd.NumViewports);
		hasher.Add(gpd.NumRenderTargets);
		hasher.Add(gpd.SubpassIndex);
		hasher.Add(gpd.SubpassIndex);
		hasher.Add(gpd.SmplDesc);

		// Hash shaders

		hasher.Add(gpsci.pVS);
		hasher.Add(gpsci.pPS);
		hasher.Add(gpsci.pDS);
		hasher.Add(gpsci.pHS);
		hasher.Add(gpsci.pGS);
		hasher.Add(gpsci.pAS);
		hasher.Add(gpsci.pMS);

		// Pso desc
		hasher.Add(gpsci.PSODesc.CommandQueueMask);
		hasher.Add(gpsci.PSODesc.PipelineType);

		hasher.Add(gpsci.PSODesc.ResourceLayout.DefaultVariableType);

		for (int i = 0; i < gpsci.PSODesc.ResourceLayout.NumVariables; ++i) {
			hasher.Add(gpsci.PSODesc.ResourceLayout.Variables[i]);
		}

		for (int i = 0; i < gpsci.PSODesc.ResourceLayout.NumImmutableSamplers; ++i) {
			hasher.Add(gpsci.PSODesc.ResourceLayout.ImmutableSamplers[i]);
		}

		return hasher.Get();
	}
}