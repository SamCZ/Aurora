#include "Material2.hpp"

#include <Aurora/Assets/AssetManager.hpp>
#include <Aurora/AuroraEngine.hpp>

namespace Aurora
{
	Material2::Material2(String name, const Path &shaderPath, ShaderMacros_t macros) : m_Name(std::move(name)), m_Macros(std::move(macros)), m_OnShaderResourceChangeEvent(nullptr)
	{
		m_OnShaderResourceChangeEvent = std::make_shared<ResourceObject::ResourceChangedEvent>([this](ResourceObject* obj) { OnShaderResourceUpdate(obj); });

		for(const auto& shaderResource : AuroraEngine::AssetManager->LoadShaderResourceFolder(shaderPath)) {
			SetShader(shaderResource);
		}
	}

	Material2::Material2(String name, ShaderMacros_t macros) : m_Name(std::move(name)), m_Macros(std::move(macros))
	{
		m_OnShaderResourceChangeEvent = std::make_shared<ResourceObject::ResourceChangedEvent>([this](ResourceObject* obj) { OnShaderResourceUpdate(obj); });
	}

	void Material2::SetShader(const ShaderResourceObject_ptr &sharedPtr)
	{
		ShaderObject shaderObject = {};
		shaderObject.ResourceEventId = sharedPtr->ResourceUpdateEvents().Add(m_OnShaderResourceChangeEvent);
		shaderObject.Type = sharedPtr->GetShaderType();
		shaderObject.ResourceObject = sharedPtr;
		shaderObject.Shader = nullptr;
		m_Shaders[sharedPtr->GetShaderType()] = shaderObject;
		OnShaderResourceUpdate(sharedPtr.get());
	}

	void Material2::RemoveShader(const SHADER_TYPE &shaderType)
	{
		auto findShaderObjIter = m_Shaders.find(shaderType);

		if(findShaderObjIter == m_Shaders.end()) {
			return;
		}

		auto& shaderObj = findShaderObjIter->second;
		shaderObj.ResourceObject->ResourceUpdateEvents().Remove(shaderObj.ResourceEventId);
		m_Shaders.erase(findShaderObjIter);
	}

	ShaderResourceObject_ptr Material2::GetShader(const SHADER_TYPE &shaderType)
	{
		auto findShaderObjIter = m_Shaders.find(shaderType);

		if(findShaderObjIter == m_Shaders.end()) {
			return nullptr;
		}

		return findShaderObjIter->second.ResourceObject;
	}

	void Material2::OnShaderResourceUpdate(ResourceObject* obj)
	{
		auto* shaderResourceObject = dynamic_cast<ShaderResourceObject*>(obj);

		std::cout << "resource update!" << std::endl;

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

		for (int i = 0; i < shader->GetResourceCount(); ++i) {
			ShaderResourceDesc desc;
			shader->GetResourceDesc(i, desc);

			switch (desc.Type) {
				case SHADER_RESOURCE_TYPE_CONSTANT_BUFFER:
					LoadConstantBuffers(shaderObj, desc);
					break;
				case SHADER_RESOURCE_TYPE_TEXTURE_SRV:
					break;
				case SHADER_RESOURCE_TYPE_BUFFER_SRV:
					break;
				case SHADER_RESOURCE_TYPE_TEXTURE_UAV:
					break;
				case SHADER_RESOURCE_TYPE_BUFFER_UAV:
					break;
				case SHADER_RESOURCE_TYPE_SAMPLER:
					break;
				case SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT:
					break;
				case SHADER_RESOURCE_TYPE_ACCEL_STRUCT:
					break;
				case SHADER_RESOURCE_TYPE_UNKNOWN:
					break;
			}
		}

		// TODO: Load shader params
	}

	void Material2::LoadConstantBuffers(ShaderObject &object, ShaderResourceDesc desc)
	{
		if(desc.Variables != nullptr) {
			const std::vector<ShaderVariable>& variables = *desc.Variables;

			for (const auto & var : variables) {
				std::cout << desc.Name << " - " << var.Name << ":" << var.Size << ":" << var.ArrayStride << ":" << var.MatrixStride << std::endl;
			}
		} else {
			// TODO: throw exception and exit program
		}
	}
}