#include "MaterialOLD.hpp"
#include "Aurora/AuroraEngine.hpp"

#include <Aurora/Graphics/GraphicUtilities.hpp>
#include <Aurora/Core/Crc.hpp>

namespace Aurora
{
	std::vector<MaterialOLD*> MaterialOLD::m_CurrentMaterials;

	MaterialOLD::MaterialOLD(String name, const Path &shaderPath, ShaderMacros_t macros)
	: m_Name(std::move(name)),
	  m_Macros(std::move(macros)),
	  m_OnShaderResourceChangeEvent(nullptr)
	{
		m_OnShaderResourceChangeEvent = std::make_shared<ResourceObject::ResourceChangedEvent>([this](ResourceObject* obj) { OnShaderResourceUpdate(obj); });

		/*for(const auto& shaderResource : AuroraEngine::AssetManager->LoadShaderResourceFolder(shaderPath)) {
			if(shaderResource->GetShaderType() == ShaderType::Compute) continue;

			SetShader(shaderResource);
		}*/

		/*SetCullMode(RasterState::CullMode::Front);
		SetDepthEnable(true);
		SetPrimitiveTopology(PrimitiveType::TriangleList);*/

		m_CurrentMaterials.push_back(this);
	}

	MaterialOLD::MaterialOLD(String name, const std::vector<ShaderResourceObject_ptr> &shaders, ShaderMacros_t macros)
			: m_Name(std::move(name)),
			  m_Macros(std::move(macros)),
			  m_OnShaderResourceChangeEvent(nullptr)
	{
		m_OnShaderResourceChangeEvent = std::make_shared<ResourceObject::ResourceChangedEvent>([this](ResourceObject* obj) { OnShaderResourceUpdate(obj); });

		for(const auto& shaderResource : shaders) {
			if(shaderResource->GetShaderType() == EShaderType::Compute) continue;

			SetShader(shaderResource);
		}

		/*SetCullMode(RasterState::CullMode::Front);
		SetDepthEnable(true);
		SetPrimitiveTopology(PrimitiveType::TriangleList);*/

		m_CurrentMaterials.push_back(this);
	}

	MaterialOLD::MaterialOLD(String name, ShaderMacros_t macros)
	: m_Name(std::move(name)),
	  m_Macros(std::move(macros))
	{
		m_OnShaderResourceChangeEvent = std::make_shared<ResourceObject::ResourceChangedEvent>([this](ResourceObject* obj) { OnShaderResourceUpdate(obj); });


		/*SetCullMode(RasterState::CullMode::Front);
		SetDepthEnable(true);
		SetPrimitiveTopology(PrimitiveType::TriangleList);*/

		m_CurrentMaterials.push_back(this);
	}

	MaterialOLD::~MaterialOLD()
	{
		m_CurrentMaterials.erase(std::find(m_CurrentMaterials.begin(), m_CurrentMaterials.end(), this));
	}

	void MaterialOLD::SetShader(const ShaderResourceObject_ptr &sharedPtr)
	{
		ShaderObject shaderObject = {};
		shaderObject.ResourceEventId = sharedPtr->ResourceUpdateEvents().Add(m_OnShaderResourceChangeEvent);
		shaderObject.Type = sharedPtr->GetShaderType();
		shaderObject.ResourceObject = sharedPtr;
		shaderObject.Shader = nullptr;
		m_Shaders[sharedPtr->GetShaderType()] = shaderObject;

		OnShaderResourceUpdate(sharedPtr.get());
	}

	void MaterialOLD::RemoveShader(const EShaderType &shaderType)
	{
		auto findShaderObjIter = m_Shaders.find(shaderType);

		if(findShaderObjIter == m_Shaders.end()) {
			return;
		}

		auto& shaderObj = findShaderObjIter->second;
		shaderObj.ResourceObject->ResourceUpdateEvents().Remove(shaderObj.ResourceEventId);
		m_Shaders.erase(findShaderObjIter);

		ApplyShaderToPSO(nullptr, shaderType);
	}

	ShaderResourceObject_ptr MaterialOLD::GetShader(const EShaderType &shaderType)
	{
		auto findShaderObjIter = m_Shaders.find(shaderType);

		if(findShaderObjIter == m_Shaders.end()) {
			return nullptr;
		}

		return findShaderObjIter->second.ResourceObject;
	}

	void MaterialOLD::OnShaderResourceUpdate(ResourceObject* obj)
	{
		auto* shaderResourceObject = dynamic_cast<ShaderResourceObject*>(obj);

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

		/*auto shader = shaderObj.Shader;

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
		m_NeedsRebuildPipeline = true;*/
	}

	void MaterialOLD::LoadConstantBuffers(ShaderObject &object, ShaderResourceDesc desc, ConstantBufferList& constantBufferStorage, ConstantBufferList& constantBufferListCopy)
	{
		const EShaderType& shaderType = object.Type;

		/*if(!desc.Variables.empty()) {
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
		}*/
	}

	void MaterialOLD::LoadTextureSRV(ShaderObject &object, ShaderResourceDesc desc, TextureDefList& textureDefList, TextureDefList& textureDefListOld, SamplerList& samplerList, SamplerList& samplerListOld)
	{
		const EShaderType& shaderType = object.Type;

		//m_ShaderResourceVariables[shaderType].push_back({{shaderType, desc.Name, SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}, false});

		/*for(auto& texDefine : textureDefListOld) {
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

		samplerList.push_back({String(desc.Name), {shaderType, desc.Name, SamLinearClampDesc}});*/
	}

	void MaterialOLD::DeleteConstantBuffers(const EShaderType& type)
	{
		m_ShaderConstantBuffers.erase(type);
	}

	void MaterialOLD::DeleteTextureSRVs(const EShaderType &type)
	{
		m_ShaderTextures.erase(type);
		m_ShaderSamplers.erase(type);
	}

	void MaterialOLD::ApplyShaderToPSO(Shader_ptr shader, const EShaderType& shaderType)
	{
		/*switch (shaderType) {
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
		}*/
	}

	void MaterialOLD::ValidateGraphicsPipelineState()
	{
		/*// Setup PSO vars
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
		//uint32_t hash = HashPSO(m_PSOCreateInfo);

		if(!m_NeedsRebuildPipeline) {
			return;
		}

		m_NeedsRebuildPipeline = false;

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
		}*/
	}

	std::size_t hashVec(std::vector<uintptr_t> const& vec) {
		std::size_t seed = vec.size();
		for(auto& i : vec) {
			seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}

	void MaterialOLD::ApplyPipeline()
	{

	}

	void MaterialOLD::CommitShaderResources()
	{
		/*for (auto& varList : m_ShaderTextures) {
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

		AuroraEngine::ImmediateContext->CommitShaderResources(GetCurrentResourceBinding(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);*/
	}

	void MaterialOLD::SetSampler(const String &textureName, const SamplerDesc &sampler)
	{
		/*for (auto& samplerList : m_ShaderSamplers) {
			for(auto& samplerFromList : samplerList.second) {
				if(textureName == samplerFromList.first) {
					samplerFromList.second.Desc = sampler;
					m_NeedsRebuildResourceLayout = true;
				}
			}
		}*/
	}

	void MaterialOLD::SetTexture(const String &name, Texture_ptr texture)
	{
		/*if(texture == nullptr) {
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

			AU_LOG_WARNING("Null texture ", name, " in ", m_Name)
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
		}*/
	}
}