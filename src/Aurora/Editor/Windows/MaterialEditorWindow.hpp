#pragma once
#ifdef IMGUI_ENABLED
#include <Aurora/App/ImGuiHelper.hpp>

#include <Aurora/Graphics/Material.hpp>

#include "../TextEditor.h"

namespace Aurora::Editor
{
	class MaterialEditorWindow
	{
	private:
		Material* m_Material;
		bool m_WindowOpened;
		ShaderResourceObject_ptr m_SelectedShader;
		TextEditor m_ShaderTextEditor;
	public:
		MaterialEditorWindow();
		void Draw();
		void SetMaterial(Material*& material, bool showWindow = true);
	};
}
#endif