#pragma once

#include "Aurora/Core/Types.hpp"
#include "Aurora/Framework/Mesh/Mesh.hpp"

#include "Base/ShaderBase.hpp"

namespace Aurora
{
	class MaterialBase
	{
	private:
		Shader_ptr m_Shader;
	public:
		MaterialBase();
		virtual ~MaterialBase();

		/*template<class T>
		uint AddVariableParameter(const String& name, T def)
		{

		}*/

	private:
		void SetShader(const Shader_ptr& shader) { m_Shader = shader; }
	};
}