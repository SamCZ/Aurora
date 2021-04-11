#pragma once

#include <RefCntAutoPtr.hpp>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <PipelineState.h>
#include <MapHelper.hpp>
#include <CommonlyUsedStates.h>

#include "IGraphicsPipeline.hpp"

#include "ShaderCollection.hpp"
#include "Aurora/Core/Common.hpp"

#include "Aurora/AuroraEngine.hpp"

#include "Aurora/Assets/Resources/ShaderResourceObject.hpp"

using namespace Diligent;

namespace Aurora
{
	struct ShaderObject
	{
		SHADER_TYPE Type;
		ShaderResourceObject_ptr ResourceObject;
		RefCntAutoPtr<IShader> Shader;
		int ResourceEventId;
	};

	typedef std::map<SHADER_TYPE, ShaderObject> ShaderList;

	AU_CLASS(Material2) : public IGraphicsPipeline
	{
	private:
		String m_Name;
		ShaderMacros_t m_Macros;

		ShaderList m_Shaders;
		std::shared_ptr<ResourceObject::ResourceChangedEvent> m_OnShaderResourceChangeEvent;
	public:
		Material2(String name, const Path& shaderPath, ShaderMacros_t macros = {});
		Material2(String name, ShaderMacros_t macros = {});
		~Material2() override = default;

		void SetShader(const ShaderResourceObject_ptr &sharedPtr);
		void RemoveShader(const SHADER_TYPE& shaderType);
		ShaderResourceObject_ptr GetShader(const SHADER_TYPE& shaderType);

	private:
		void OnShaderResourceUpdate(ResourceObject* obj);
	public:
		[[nodiscard]] const ShaderList& GetShaders() const noexcept { return m_Shaders; }
		[[nodiscard]] const ShaderMacros_t& GetMacros() const noexcept { return m_Macros; }

		void LoadConstantBuffers(ShaderObject &object, ShaderResourceDesc desc);
	};
}