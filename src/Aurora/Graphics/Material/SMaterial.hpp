#pragma once

#include "Aurora/Core/Types.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/PassType.hpp"

namespace Aurora
{
	class MaterialShaderPass
	{
	private:
		Shader_ptr m_Shader;
	public:
		template<typename T>
		void SetVariable()
		{

		}
	};

	class SMaterial
	{
	private:

	public:
		SMaterial();
		~SMaterial();

		bool AddPass(uint8_t passType, const Path& shaderPath);
		bool RemovePass(uint8_t passType);

		void Init();
		void ReloadShaders();



		void BindResources(BaseState state, uint8_t passType);
	};
}
