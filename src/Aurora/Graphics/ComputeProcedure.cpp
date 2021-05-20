#include "ComputeProcedure.hpp"

#include <utility>
#include "Aurora/Assets/AssetManager.hpp"
#include "Aurora/AuroraEngine.hpp"
#include "Aurora/Core/Crc.hpp"

#include <GraphicsAccessories.hpp>
#include "GraphicUtilities.hpp"

namespace Aurora
{
	ComputeProcedure::ComputeProcedure(const String &name, const Path &shaderPath, const std::map<String, String>& macros)
			: ComputeProcedure(name, AuroraEngine::AssetManager->LoadShader(shaderPath, SHADER_TYPE_COMPUTE, macros)) { }

	ComputeProcedure::ComputeProcedure(String name, const RefCntAutoPtr<IShader> &computeShader, const std::map<String, String>& macros) : m_Name(std::move(name)), m_Shader(computeShader)
	{
		if(m_Shader == nullptr) {
			throw std::runtime_error("Shader is null");
		}

		m_CPSCInfo.PSODesc.PipelineType = PIPELINE_TYPE_COMPUTE;
		m_CPSCInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
		m_CPSCInfo.pCS = m_Shader;

		std::vector<StateTransitionDesc> barriers;

		for (int i = 0; i < m_Shader->GetResourceCount(); ++i) {
			ShaderResourceDesc desc;
			m_Shader->GetResourceDesc(i, desc);

			const SHADER_TYPE& shaderType = m_Shader->GetDesc().ShaderType;

			switch (desc.Type) {
				case SHADER_RESOURCE_TYPE_CONSTANT_BUFFER: {
					auto iter = m_ShaderConstantBuffers.find(desc.Name);
					if(iter != m_ShaderConstantBuffers.end()) {
						auto& constBufferData = iter->second;
						AU_LOG_ERROR("Found two different constant buffers in material: ", m_Name);
					} else {
						ShaderConstantBuffer shaderConstantBufferInfo = {};
						shaderConstantBufferInfo.Size = desc.Size;
						shaderConstantBufferInfo.Buffer = nullptr;
						shaderConstantBufferInfo.BufferData.resize(desc.Size);
						shaderConstantBufferInfo.NeedsUpdate = true;

						BufferDesc CBDesc;
						CBDesc.Name           = ("Constant buffer " + String(desc.Name) + "for shader " + m_Name).c_str();
						CBDesc.uiSizeInBytes  = desc.Size;
						CBDesc.Usage          = USAGE_DYNAMIC;
						CBDesc.BindFlags      = BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE;
						CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
						CBDesc.Mode = BUFFER_MODE_RAW;

						shaderConstantBufferInfo.Desc = CBDesc;

						if(desc.Variables != nullptr) {
							const std::vector<ShaderVariable>& variables = *desc.Variables;

							shaderConstantBufferInfo.Variables = variables;
						} else {
							// TODO: throw exception and exit program
						}

						AuroraEngine::RenderDevice->CreateBuffer(shaderConstantBufferInfo.Desc, nullptr, &shaderConstantBufferInfo.Buffer);
						barriers.emplace_back(shaderConstantBufferInfo.Buffer, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true);

						m_ShaderConstantBuffers[desc.Name] = shaderConstantBufferInfo;
					}
					break;
				}
				case SHADER_RESOURCE_TYPE_SAMPLER: {
					break;
				}
				case SHADER_RESOURCE_TYPE_TEXTURE_UAV:
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

					break;
				}
				case SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT: {
					break;
				}
				default:
					AU_LOG_WARNING("Unimplemented resource type: ", desc.Name)
					//TODO: Complete other buffers and UAV
					continue;
			}
		}

		if(!barriers.empty()) {
			AuroraEngine::ImmediateContext->TransitionResourceStates(barriers.size(), barriers.data());
		}
	}

	void ComputeProcedure::Dispatch(uint32_t ThreadGroupCountX, uint32_t ThreadGroupCountY, uint32_t ThreadGroupCountZ)
	{
		Validate();

		AuroraEngine::ImmediateContext->SetPipelineState(m_CurrentPipelineState);

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

		DispatchComputeAttribs DispatcAttribs;
		DispatcAttribs.ThreadGroupCountX = ThreadGroupCountX;
		DispatcAttribs.ThreadGroupCountY = ThreadGroupCountY;
		DispatcAttribs.ThreadGroupCountZ = ThreadGroupCountZ;
		AuroraEngine::ImmediateContext->DispatchCompute(DispatcAttribs);
	}

	void ComputeProcedure::Validate()
	{
		for(auto& it : m_ShaderConstantBuffers) {
			it.second.NeedsUpdate = true;
		}

		// Setup PSO
		m_CPSCInfo.PSODesc.ResourceLayout.Variables = m_ShaderResourceVariables.data();
		m_CPSCInfo.PSODesc.ResourceLayout.NumVariables = m_ShaderResourceVariables.size();

		m_CPSCInfo.PSODesc.ResourceLayout.ImmutableSamplers = m_ShaderResourceSamplers.data();
		m_CPSCInfo.PSODesc.ResourceLayout.NumImmutableSamplers = m_ShaderResourceSamplers.size();

		// Update pipeline
		uint32_t hash = GetCreateInfoHash(m_CPSCInfo);

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

		ComputePipelineStateCreateInfo infoCopy(m_CPSCInfo);
		AuroraEngine::RenderDevice->CreateComputePipelineState(infoCopy, &m_CurrentPipelineState);

		// Bind constant buffers
		for(auto& it : m_ShaderConstantBuffers) {
			String constantBufferName = it.first;
			ShaderConstantBuffer& shaderConstantBuffer = it.second;
			m_CurrentPipelineState->GetStaticVariableByName(SHADER_TYPE_COMPUTE, constantBufferName.c_str())->Set(shaderConstantBuffer.Buffer);
		}

		PipelineStateData stateData = {};
		stateData.State = m_CurrentPipelineState;
		m_CurrentPipelineState->CreateShaderResourceBinding(&stateData.ResourceBinding, true);

		m_CurrentResourceBinding = stateData.ResourceBinding;
		m_CurrentPipelineStateHash = hash;
		m_PipelineStates[hash] = stateData;
	}

	uint32_t ComputeProcedure::GetCreateInfoHash(ComputePipelineStateCreateInfo gpsci)
	{
		CrcHash hasher;

		// Hash shaders
		hasher.Add(gpsci.pCS);

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