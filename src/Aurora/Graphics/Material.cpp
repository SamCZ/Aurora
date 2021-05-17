#include "Material.hpp"
#include "Aurora/AuroraEngine.hpp"

#include <GraphicsAccessories.hpp>

#include <Aurora/Graphics/GraphicUtilities.hpp>
#include <Aurora/Core/Crc.hpp>

namespace Aurora
{
	std::vector<Material*> Material::m_CurrentMaterials;

	Material::Material(String name, const Path &shaderPath, ShaderMacros_t macros)
	: m_Name(std::move(name)),
	  m_Macros(std::move(macros)),
	  m_OnShaderResourceChangeEvent(nullptr),
	  m_CurrentPipelineState(nullptr),
	  m_CurrentResourceBinding(nullptr),
	  m_CurrentPipelineStateHash(0),
	  m_NeedsRebuildResourceLayout(true)
	{
		m_OnShaderResourceChangeEvent = std::make_shared<ResourceObject::ResourceChangedEvent>([this](ResourceObject* obj) { OnShaderResourceUpdate(obj); });

		for(const auto& shaderResource : AuroraEngine::AssetManager->LoadShaderResourceFolder(shaderPath)) {
			if(shaderResource->GetShaderType() == SHADER_TYPE_COMPUTE) continue;

			SetShader(shaderResource);
		}

		m_PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
		m_PSOCreateInfo.PSODesc.Name = name.c_str();
		m_PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

		SetCullMode(CULL_MODE_FRONT);
		SetDepthEnable(true);
		SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		m_CurrentMaterials.push_back(this);
	}

	Material::Material(String name, const std::vector<ShaderResourceObject_ptr> &shaders, ShaderMacros_t macros)
			: m_Name(std::move(name)),
			  m_Macros(std::move(macros)),
			  m_OnShaderResourceChangeEvent(nullptr),
			  m_CurrentPipelineState(nullptr),
			  m_CurrentResourceBinding(nullptr),
			  m_CurrentPipelineStateHash(0),
			  m_NeedsRebuildResourceLayout(true)
	{
		m_OnShaderResourceChangeEvent = std::make_shared<ResourceObject::ResourceChangedEvent>([this](ResourceObject* obj) { OnShaderResourceUpdate(obj); });

		for(const auto& shaderResource : shaders) {
			if(shaderResource->GetShaderType() == SHADER_TYPE_COMPUTE) continue;

			SetShader(shaderResource);
		}

		m_PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
		m_PSOCreateInfo.PSODesc.Name = name.c_str();
		m_PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

		SetCullMode(CULL_MODE_FRONT);
		SetDepthEnable(true);
		SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		m_CurrentMaterials.push_back(this);
	}

	Material::Material(String name, ShaderMacros_t macros)
	: m_Name(std::move(name)),
	  m_Macros(std::move(macros)),
	  m_CurrentPipelineState(nullptr),
	  m_CurrentResourceBinding(nullptr),
	  m_CurrentPipelineStateHash(0),
	  m_NeedsRebuildResourceLayout(true)
	{
		m_OnShaderResourceChangeEvent = std::make_shared<ResourceObject::ResourceChangedEvent>([this](ResourceObject* obj) { OnShaderResourceUpdate(obj); });

		m_PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
		m_PSOCreateInfo.PSODesc.Name = name.c_str();
		m_PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

		SetCullMode(CULL_MODE_FRONT);
		SetDepthEnable(true);
		SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		m_CurrentMaterials.push_back(this);
	}

	Material::~Material()
	{
		m_CurrentMaterials.erase(std::find(m_CurrentMaterials.begin(), m_CurrentMaterials.end(), this));
	}

	void Material::SetShader(const ShaderResourceObject_ptr &sharedPtr)
	{
		ShaderObject shaderObject = {};
		shaderObject.ResourceEventId = sharedPtr->ResourceUpdateEvents().Add(m_OnShaderResourceChangeEvent);
		shaderObject.Type = sharedPtr->GetShaderType();
		shaderObject.ResourceObject = sharedPtr;
		shaderObject.Shader = nullptr;
		m_Shaders[sharedPtr->GetShaderType()] = shaderObject;

		OnShaderResourceUpdate(sharedPtr.get());

		m_NeedsRebuildResourceLayout = true;
	}

	void Material::RemoveShader(const SHADER_TYPE &shaderType)
	{
		auto findShaderObjIter = m_Shaders.find(shaderType);

		if(findShaderObjIter == m_Shaders.end()) {
			return;
		}

		auto& shaderObj = findShaderObjIter->second;
		shaderObj.ResourceObject->ResourceUpdateEvents().Remove(shaderObj.ResourceEventId);
		m_Shaders.erase(findShaderObjIter);

		ApplyShaderToPSO(nullptr, shaderType);

		m_NeedsRebuildResourceLayout = true;
	}

	ShaderResourceObject_ptr Material::GetShader(const SHADER_TYPE &shaderType)
	{
		auto findShaderObjIter = m_Shaders.find(shaderType);

		if(findShaderObjIter == m_Shaders.end()) {
			return nullptr;
		}

		return findShaderObjIter->second.ResourceObject;
	}

	void Material::OnShaderResourceUpdate(ResourceObject* obj)
	{
		auto* shaderResourceObject = dynamic_cast<ShaderResourceObject*>(obj);

		//std::cout << "resource update!" << std::endl;

		auto findShaderObjIter = m_Shaders.find(shaderResourceObject->GetShaderType());

		if(findShaderObjIter == m_Shaders.end()) {
			throw std::runtime_error("Found unexpected shader update in material !");
		}

		auto& shaderObj = findShaderObjIter->second;

		auto compileResult = shaderResourceObject->GetOrCompile(m_Macros);

		if(compileResult.Compiled) {
			shaderObj.Shader = compileResult.Shader;
		} else {
			std::cerr << "Shader compilation error !" << std::endl;
			return;
		}

		// Load shader data

		auto shader = shaderObj.Shader;

		m_ShaderResourceVariables[shaderObj.Type].clear();

		std::vector<StateTransitionDesc> barriers;

		TextureDefList& textureDefList = m_ShaderTextures[shaderObj.Type];
		TextureDefList textureDefListCopy = textureDefList;
		textureDefList.clear();

		ConstantBufferList& constantBufferStorage = m_ShaderConstantBuffers[shaderObj.Type];
		ConstantBufferList constantBufferListCopy = constantBufferStorage;
		constantBufferStorage.clear();
		//TODO: Keep old variables.

		SamplerList& samplerList = m_ShaderSamplers[shaderObj.Type];
		SamplerList samplerListCopy = samplerList;
		samplerList.clear();

		for(auto sampler : samplerListCopy) {
			sampler.second.SamplerOrTextureName = sampler.first.c_str();
		}

		bool hasConstantBuffers = false;
		bool hasTextureSRVs = false;

		for (int i = 0; i < shader->GetResourceCount(); ++i) {
			ShaderResourceDesc desc;
			shader->GetResourceDesc(i, desc);

			// TODO: Load shader params
			switch (desc.Type) {
				case SHADER_RESOURCE_TYPE_CONSTANT_BUFFER:
					LoadConstantBuffers(shaderObj, desc, barriers, constantBufferStorage, constantBufferListCopy);
					hasConstantBuffers = true;
					break;
				case SHADER_RESOURCE_TYPE_TEXTURE_SRV:
					LoadTextureSRV(shaderObj, desc, barriers, textureDefList, textureDefListCopy, samplerList, samplerListCopy);
					hasTextureSRVs = true;
					break;
				case SHADER_RESOURCE_TYPE_BUFFER_SRV:
				case SHADER_RESOURCE_TYPE_TEXTURE_UAV:
				case SHADER_RESOURCE_TYPE_BUFFER_UAV:
				case SHADER_RESOURCE_TYPE_SAMPLER:
				case SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT:
				case SHADER_RESOURCE_TYPE_ACCEL_STRUCT:
				case SHADER_RESOURCE_TYPE_UNKNOWN:
					break;
			}
		}

		if(!barriers.empty()) {
			AuroraEngine::ImmediateContext->TransitionResourceStates(barriers.size(), barriers.data());
		}

		if(!hasConstantBuffers) {
			DeleteConstantBuffers(shaderObj.Type);
		}

		if(!hasTextureSRVs) {
			DeleteTextureSRVs(shaderObj.Type);
		}

		if(!hasConstantBuffers && !hasTextureSRVs) {
			m_ShaderResourceVariables.erase(shaderObj.Type);
		}

		ApplyShaderToPSO(shader, shaderObj.Type);

		m_NeedsRebuildResourceLayout = true;
	}

	void Material::LoadConstantBuffers(ShaderObject &object, ShaderResourceDesc desc, std::vector<StateTransitionDesc>& barriers, ConstantBufferList& constantBufferStorage, ConstantBufferList& constantBufferListCopy)
	{
		const SHADER_TYPE& shaderType = object.Type;

		m_ShaderResourceVariables[shaderType].push_back({{shaderType, desc.Name, SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}, true});

		if(desc.Variables != nullptr) {
			const std::vector<ShaderVariable>& variables = *desc.Variables;

			ShaderConstantBuffer shaderConstantBufferInfo = {};
			shaderConstantBufferInfo.Size = desc.Size;
			shaderConstantBufferInfo.Buffer = nullptr;
			shaderConstantBufferInfo.ShaderType = shaderType;
			shaderConstantBufferInfo.BufferData.resize(desc.Size);
			shaderConstantBufferInfo.NeedsUpdate = true;
			shaderConstantBufferInfo.Name = desc.Name;

			BufferDesc CBDesc;
			CBDesc.Name           = ("Constant buffer " + String(desc.Name) + "for shader " + m_Name).c_str();
			CBDesc.uiSizeInBytes  = desc.Size;
			CBDesc.Usage          = USAGE_DYNAMIC;
			CBDesc.BindFlags      = BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;

			shaderConstantBufferInfo.Desc = CBDesc;

			for (const auto & var : variables) {
				//std::cout << desc.Name << " - " << var.Name << ":" << var.Size << ":" << var.ArrayStride << ":" << var.MatrixStride << std::endl;
				shaderConstantBufferInfo.Variables = variables;

				{ // Search for old variables and set the value to the current buffer
					for(const auto& otherBufferStore : constantBufferListCopy) {
						for (const auto& otherVar : otherBufferStore.Variables) {
							if(otherVar.Name == var.Name && otherVar.Size == var.Size) {
								//std::cout << "Copying same variable: " << var.Name << std::endl;
								memcpy(shaderConstantBufferInfo.BufferData.data() + var.Offset, otherBufferStore.BufferData.data() + otherVar.Offset, var.Size);
								goto exit_loop;
							}
						}
					}
					exit_loop:;
				}
			}

			AuroraEngine::RenderDevice->CreateBuffer(shaderConstantBufferInfo.Desc, nullptr, &shaderConstantBufferInfo.Buffer);
			barriers.emplace_back(shaderConstantBufferInfo.Buffer, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true);

			constantBufferStorage.emplace_back(shaderConstantBufferInfo);
		} else {
			// TODO: throw exception and exit program
		}
	}

	void Material::LoadTextureSRV(ShaderObject &object, ShaderResourceDesc desc, std::vector<StateTransitionDesc>& barriers, TextureDefList& textureDefList, TextureDefList& textureDefListOld, SamplerList& samplerList, SamplerList& samplerListOld)
	{
		const SHADER_TYPE& shaderType = object.Type;

		m_ShaderResourceVariables[shaderType].push_back({{shaderType, desc.Name, SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}, false});

		for(auto& texDefine : textureDefListOld) {
			if(texDefine.Name == desc.Name) {
				texDefine.NeedsUpdate = true;
				textureDefList.push_back(texDefine);

				for(const auto& sampler : samplerListOld) {
					if(texDefine.Name == sampler.first) {
						samplerList.push_back(sampler);
						break;
					}
				}

				return;
			}
		}

		ShaderTextureDef textureDef = {};
		textureDef.Name = desc.Name;
		textureDef.ShaderType = shaderType;

		textureDef.TextureRef = GraphicUtilities::GetPlaceholderTexture();
		if(textureDef.TextureRef != nullptr) {
			textureDef.TextureView = GraphicUtilities::GetPlaceholderTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
		}

		//textureDef.TextureRef = nullptr;
		//textureDef.TextureView = nullptr;

		textureDef.NeedsUpdate = true;
		textureDefList.emplace_back(textureDef);

		SamplerDesc SamLinearClampDesc
		{
			FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
			TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP
		};

		samplerList.push_back({String(desc.Name), {shaderType, desc.Name, SamLinearClampDesc}});
	}

	void Material::DeleteConstantBuffers(const SHADER_TYPE& type)
	{
		m_ShaderConstantBuffers.erase(type);
	}

	void Material::DeleteTextureSRVs(const SHADER_TYPE &type)
	{
		m_ShaderTextures.erase(type);
		m_ShaderSamplers.erase(type);
	}

	void Material::ApplyShaderToPSO(IShader* shader, const SHADER_TYPE& shaderType)
	{
		switch (shaderType) {
			case SHADER_TYPE_VERTEX:
				m_PSOCreateInfo.pVS = shader;
				break;
			case SHADER_TYPE_PIXEL:
				m_PSOCreateInfo.pPS = shader;
				break;
			case SHADER_TYPE_GEOMETRY:
				m_PSOCreateInfo.pGS = shader;
				break;
			case SHADER_TYPE_HULL:
				m_PSOCreateInfo.pHS = shader;
				break;
			case SHADER_TYPE_DOMAIN:
				m_PSOCreateInfo.pDS = shader;
				break;
			default:
				// Unsupported shader type
				break;
		}
	}

	void Material::ValidateGraphicsPipelineState()
	{
		// Setup PSO vars
		if(m_NeedsRebuildResourceLayout) {
			m_NeedsRebuildResourceLayout = false;

			m_PSO_ResourceVariableDescList.clear();

			std::vector<String> usedVars;

			// Set resource vars
			for(const auto& it : m_ShaderResourceVariables) {
				for(const auto& varPair : it.second) {
					//if(!varPair.second) continue;

					auto var = varPair.first;
					if(std::find(usedVars.begin(), usedVars.end(), var.Name) == usedVars.end()) {
						m_PSO_ResourceVariableDescList.push_back(var);
						usedVars.emplace_back(var.Name);
					}
				}
			}

			m_PSOCreateInfo.PSODesc.ResourceLayout.Variables = m_PSO_ResourceVariableDescList.data();
			m_PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = m_PSO_ResourceVariableDescList.size();

			// Set samplers
			usedVars.clear();

			m_PSO_ShaderResourceSamplers.clear();

			for(const auto& it : m_ShaderSamplers) {
				for(const auto& sampler : it.second) {
					if(std::find(usedVars.begin(), usedVars.end(), sampler.first) == usedVars.end()) {
						ImmutableSamplerDesc newSampler;
						newSampler.ShaderStages = sampler.second.ShaderStages;
						newSampler.SamplerOrTextureName = sampler.first.c_str();
						newSampler.Desc = sampler.second.Desc;
						m_PSO_ShaderResourceSamplers.emplace_back(newSampler);
						usedVars.emplace_back(sampler.first);
					} else {
						// TODO: Throw warning! Found two different samplers with same name.
					}
				}
			}

			m_PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = m_PSO_ShaderResourceSamplers.data();
			m_PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = m_PSO_ShaderResourceSamplers.size();
		}

		// Update pipeline
		// TODO: fixme hashing not work
		uint32_t hash = HashPSO(m_PSOCreateInfo);

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

		PipelineStateData stateData = {};
		stateData.State = m_CurrentPipelineState;
		m_CurrentPipelineState->CreateShaderResourceBinding(&stateData.ResourceBinding, true);
		m_CurrentResourceBinding = stateData.ResourceBinding;

		// Bind constant buffers

		for(auto& it : m_ShaderConstantBuffers) {
			for(auto shaderConstantBuffer : it.second) {
				//auto var = m_CurrentPipelineState->GetStaticVariableByName(shaderConstantBuffer.ShaderType, shaderConstantBuffer.Name.c_str());
				auto var = GetCurrentResourceBinding()->GetVariableByName(shaderConstantBuffer.ShaderType, shaderConstantBuffer.Name.c_str());
				if(var != nullptr) {
					var->Set(shaderConstantBuffer.Buffer);
				} else {
					std::cerr << "var is null! " << shaderConstantBuffer.Name << std::endl;
				}

				//std::cout << shaderConstantBuffer.Name << " - " << m_Name << "(" << GetShaderTypeLiteralName(shaderConstantBuffer.ShaderType) << ") - " << shaderConstantBuffer.Buffer.RawPtr() << std::endl;
				// TODO: Warn about two different buffers with same name
			}
		}

		m_CurrentPipelineStateHash = hash;
		m_PipelineStates[hash] = stateData;
	}

	std::size_t hashVec(std::vector<uintptr_t> const& vec) {
		std::size_t seed = vec.size();
		for(auto& i : vec) {
			seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}

	uint32_t Material::HashPSO(const GraphicsPipelineStateCreateInfo &gpsci)
	{
		CrcHash hasher;

		const GraphicsPipelineDesc& gpd = gpsci.GraphicsPipeline;

		// Hash blend state
		hasher.Add(gpd.BlendDesc);

		// Hash raster state
		hasher.Add(gpd.RasterizerDesc);

		// Hash depth stencil
		hasher.Add(gpd.DepthStencilDesc);

		// Hash input layout
		const InputLayoutDesc& inputLayout = gpd.InputLayout;
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


		std::vector<uintptr_t> shadersPointers;

		if(gpsci.pVS != nullptr) {
			shadersPointers.push_back(reinterpret_cast<uintptr_t>(&gpsci.pVS));
		}

		if(gpsci.pPS != nullptr) {
			shadersPointers.push_back(reinterpret_cast<uintptr_t>(&gpsci.pPS));
		}

		hasher.Add(hashVec(shadersPointers));

		// Pointers not work in hashing
		/*hasher.Add(gpsci.pVS);
		hasher.Add(gpsci.pPS);
		hasher.Add(gpsci.pDS);
		hasher.Add(gpsci.pHS);
		hasher.Add(gpsci.pGS);
		hasher.Add(gpsci.pAS);
		hasher.Add(gpsci.pMS);*/

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

	void Material::ApplyPipeline()
	{
		AuroraEngine::ImmediateContext->SetPipelineState(GetCurrentPipelineState());
	}

	void Material::CommitShaderResources()
	{
		for (auto& varList : m_ShaderTextures) {
			for(auto& var : varList.second) {
				if(!var.NeedsUpdate) continue;
				if(var.TextureView == nullptr) {
					//std::cerr << "Cannot commit resources with empty texture: " << var.Name << std::endl;
					continue;
				}

				auto resVal = GetCurrentResourceBinding()->GetVariableByName(varList.first, var.Name.c_str());

				if(resVal != nullptr) {
					resVal->Set(var.TextureView);
				}

			}
		}

		for(auto& constantList : m_ShaderConstantBuffers) {
			for(auto& buffer : constantList.second) {
				if(!buffer.NeedsUpdate) {
					//continue;
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

				buffer.NeedsUpdate = true;

				//GetCurrentResourceBinding()->GetVariableByName(constantList.first, buffer.Name.c_str())->Set(buffer.Buffer);
			}
		}

		AuroraEngine::ImmediateContext->CommitShaderResources(GetCurrentResourceBinding(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	void Material::SetSampler(const String &textureName, const SamplerDesc &sampler)
	{
		for (auto& samplerList : m_ShaderSamplers) {
			for(auto& samplerFromList : samplerList.second) {
				if(textureName == samplerFromList.first) {
					samplerFromList.second.Desc = sampler;
					m_NeedsRebuildResourceLayout = true;
				}
			}
		}
	}

	void Material::SetTexture(const String &name, const RefCntAutoPtr<ITexture> &texture)
	{
		if(texture == nullptr) {
			for(auto& rvl : m_ShaderResourceVariables) {
				for(auto& rv : rvl.second) {
					if(rv.first.Name == name) {
						rv.second = false;
						m_NeedsRebuildResourceLayout = true;
						goto endfor;
					}
				}
			}
			endfor:;

			std::cout << "Null texture " << name << " in " << m_Name << std::endl;
			return;
		}

		// This is just internal! Don't ever do this!!!
		auto& tex = const_cast<RefCntAutoPtr<ITexture>&>(texture);

		for (auto& varList : m_ShaderTextures) {
			for(auto& textureDef : varList.second) {
				if(textureDef.Name == name) {
					textureDef.TextureRef = texture;
					textureDef.TextureView = tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
					textureDef.NeedsUpdate = true;

					for(auto& rvl : m_ShaderResourceVariables) {
						for(auto& rv : rvl.second) {
							if(rv.first.Name == name) {
								rv.second = true;
								m_NeedsRebuildResourceLayout = true;
								goto endfor2;
							}
						}
					}
					endfor2:;

					break;
				}
			}
		}
	}
}