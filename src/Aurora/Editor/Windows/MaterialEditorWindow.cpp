#include "MaterialEditorWindow.hpp"

#include <GraphicsAccessories.hpp>

namespace Aurora::Editor
{
	MaterialEditorWindow::MaterialEditorWindow() : m_ShaderTextEditor()
	{
		m_ShaderTextEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
		m_ShaderTextEditor.SetShowWhitespaces(false);
	}

	void MaterialEditorWindow::Draw()
	{
		auto material = m_Material.lock();

		if(material == nullptr) return;

		static std::map<String, bool> menus = {
				{"Shaders", true},
				{"Macros", false},
				{"Variables", false},
				{"Blending", false},
		};

		ImGui::BeginWindow("Material editor", 150, 250, 550, 400, false, 0, &m_WindowOpened);
		{
			ImGui::BeginChild("material preview", 128, 128, true, -1); {
				//ImGui::Image(crosshair->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), { 100, 100 });
			} ImGui::EndChild();

			ImGui::SameLine(0, -1);

			ImGui::BeginChild("Recompile", 0, 128, true, -1);
			{
				ImGui::Text("Info");
			}
			ImGui::EndChild();

			ImGui::BeginChild("menu", 128, 0, true, 10); {
				for(auto& it : menus) {
					if(ImGui::Selectable(it.first, it.second)) {
						it.second = true;
						for(auto& it2 : menus) if(it.first != it2.first) it2.second = false;
					}
				}
			} ImGui::EndChild();

			ImGui::SameLine(0, -1);

			ImGui::BeginChild("shaders", 0, 0, true, 10); {
				if(menus["Shaders"]) {

					for(const auto& it : material->GetShaders()) {
						if(ImGui::Button(String(GetShaderTypeLiteralName(it.first)) + ":" + PointerToString(it.second.Shader.RawPtr()))) {
							m_SelectedShader = it.second.ResourceObject;
							m_ShaderTextEditor.SetText(m_SelectedShader->GetShaderSource());
						}
						ImGui::SameLine();
					}

					ImGui::Text(" ");
					ImGui::Separator();

					if(m_SelectedShader != nullptr) {
						if(ImGui::Button("Compile and save")) {
							TextEditor::ErrorMarkers markers;

							auto compileStatus = m_SelectedShader->Compile(m_ShaderTextEditor.GetText(), material->GetMacros());

							if(compileStatus.Compiled) {
								m_SelectedShader->SetShaderSource(m_ShaderTextEditor.GetText());
							} else {
								for(const auto& err : compileStatus.LineErrors) {
									markers.insert(err);
								}
							}

							m_ShaderTextEditor.SetErrorMarkers(markers);
						}

						ImGui::Separator();

						m_ShaderTextEditor.Render("shaderTextEditor");
					}
				}
			} ImGui::EndChild();
		}
		ImGui::EndWindow();
	}

	void MaterialEditorWindow::SetMaterial(const Material2_wptr &material, bool showWindow)
	{
		m_Material = material;
		m_WindowOpened = showWindow;
		m_SelectedShader = nullptr;
	}
}