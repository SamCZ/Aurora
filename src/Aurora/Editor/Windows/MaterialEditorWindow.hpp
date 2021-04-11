#pragma once

#include <Aurora/App/ImGuiHelper.hpp>

#include <Aurora/Graphics/Material2.hpp>

#include "../TextEditor.h"

namespace Aurora::Editor
{
	class MaterialEditorWindow
	{
	private:
		Material2_wptr m_Material;
		bool m_WindowOpened;
		ShaderResourceObject_ptr m_SelectedShader;
		TextEditor m_ShaderTextEditor;
	public:
		MaterialEditorWindow();
		void Draw();
		void SetMaterial(const Material2_wptr& material, bool showWindow = true);
	};
}
