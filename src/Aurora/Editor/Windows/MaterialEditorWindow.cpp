#ifdef IMGUI_ENABLED
#include "MaterialEditorWindow.hpp"

#include <GraphicsAccessories.hpp>

namespace Aurora::Editor
{
	MaterialEditorWindow::MaterialEditorWindow() : m_ShaderTextEditor(), m_Material(nullptr), m_WindowOpened(true)
	{
		m_ShaderTextEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
		m_ShaderTextEditor.SetShowWhitespaces(false);
	}

	void MaterialEditorWindow::Draw()
	{
		auto material = m_Material;

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
				//ImGui::Text("Hash: " + std::to_string(material->GetHash()));
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

			ImGui::BeginChild("childdddd", 0, 0, true, 10); {
				if(menus["Variables"]) {
					static glm::vec3 test;
					ImGui::DrawVec3Control("test", test);
				} else if(menus["Shaders"]) {

					for(const auto& it : material->GetShaders()) {
						if(ImGui::Button(String(GetShaderTypeLiteralName(it.first))/* + ":" + PointerToString(it.second.Shader.RawPtr())*/)) {
							m_SelectedShader = it.second.ResourceObject;
							m_ShaderTextEditor.SetText(m_SelectedShader->GetShaderSource());
						}
						ImGui::SameLine();
					}

					ImGui::Text(" ");
					if(m_SelectedShader != nullptr) {
						if(ImGui::Button("Reload from file")) {
							m_SelectedShader->Load(true);
							m_ShaderTextEditor.SetText(m_SelectedShader->GetShaderSource());

							TextEditor::ErrorMarkers markers;

							auto compileStatus = m_SelectedShader->Compile(m_ShaderTextEditor.GetText(), material->GetMacros());

							if(compileStatus.Compiled) {
								//m_SelectedShader->SetShaderSource(m_ShaderTextEditor.GetText());
							} else {
								for(const auto& err : compileStatus.LineErrors) {
									markers.insert(err);
								}
							}

							m_ShaderTextEditor.SetErrorMarkers(markers);
						}

						ImGui::SameLine();

						if(ImGui::Button("Save to file")) {
							if(m_SelectedShader != nullptr)
								m_SelectedShader->Save();
						}

						ImGui::Separator();
						ImGui::Separator();

						if(ImGui::Button("Compile only")) {
							TextEditor::ErrorMarkers markers;

							auto compileStatus = m_SelectedShader->Compile(m_ShaderTextEditor.GetText(), material->GetMacros());

							if(compileStatus.Compiled) {

							} else {
								for(const auto& err : compileStatus.LineErrors) {
									markers.insert(err);
								}
							}

							m_ShaderTextEditor.SetErrorMarkers(markers);
						}

						ImGui::SameLine();

						if(ImGui::Button("Compile and apply")) {
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

						ImGui::SameLine();

						if(ImGui::Button("Clear errors")) {
							TextEditor::ErrorMarkers markers;
							m_ShaderTextEditor.SetErrorMarkers(markers);
						}

						ImGui::BeginChild("shaders-editor", 0, 0, false, 0, ImGuiWindowFlags_MenuBar);

						if (ImGui::BeginMenuBar())
						{
							if (ImGui::BeginMenu("File"))
							{
								if (ImGui::MenuItem("Save"))
								{
									auto textToSave = m_ShaderTextEditor.GetText();
									/// save text....
								}
								ImGui::EndMenu();
							}



							if (ImGui::BeginMenu("Edit"))
							{
								bool ro = m_ShaderTextEditor.IsReadOnly();
								if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
									m_ShaderTextEditor.SetReadOnly(ro);
								ImGui::Separator();

								if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && m_ShaderTextEditor.CanUndo()))
									m_ShaderTextEditor.Undo();
								if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && m_ShaderTextEditor.CanRedo()))
									m_ShaderTextEditor.Redo();

								ImGui::Separator();

								if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, m_ShaderTextEditor.HasSelection()))
									m_ShaderTextEditor.Copy();
								if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && m_ShaderTextEditor.HasSelection()))
									m_ShaderTextEditor.Cut();
								if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && m_ShaderTextEditor.HasSelection()))
									m_ShaderTextEditor.Delete();
								if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
									m_ShaderTextEditor.Paste();

								ImGui::Separator();

								if (ImGui::MenuItem("Select all", nullptr, nullptr))
									m_ShaderTextEditor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(m_ShaderTextEditor.GetTotalLines(), 0));

								ImGui::EndMenu();
							}

							if (ImGui::BeginMenu("View"))
							{
								if (ImGui::MenuItem("Dark palette"))
									m_ShaderTextEditor.SetPalette(TextEditor::GetDarkPalette());
								if (ImGui::MenuItem("Light palette"))
									m_ShaderTextEditor.SetPalette(TextEditor::GetLightPalette());
								if (ImGui::MenuItem("Retro blue palette"))
									m_ShaderTextEditor.SetPalette(TextEditor::GetRetroBluePalette());
								ImGui::EndMenu();
							}
							ImGui::EndMenuBar();
						}


						m_ShaderTextEditor.Render("shaderTextEditor");
					}

					ImGui::EndChild();
				}
			} ImGui::EndChild();
		}
		ImGui::EndWindow();
	}

	void MaterialEditorWindow::SetMaterial(Material* &material, bool showWindow)
	{
		m_Material = material;
		m_WindowOpened = showWindow;
		m_SelectedShader = nullptr;
	}
}
#endif