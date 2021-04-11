#pragma once

#include <Aurora/App/ImGuiHelper.hpp>

#include <Aurora/Graphics/Material.hpp>

#include "../TextEditor.h"

namespace Aurora::Editor
{
	class MaterialEditorWindow
	{
	private:
		Material_wptr m_Material;
		bool m_WindowOpened;
		ShaderResourceObject_ptr m_SelectedShader;
		TextEditor m_ShaderTextEditor;
	public:
		MaterialEditorWindow();
		void Draw();
		void SetMaterial(const Material_wptr& material, bool showWindow = true);
	};
}
