#include "MaterialWindow.hpp"

#include "Utils.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Aurora.hpp"
#include "Aurora/Core/Profiler.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Tools/IconsFontAwesome5.hpp"

namespace Aurora
{

	MaterialWindow::MaterialWindow() : EditorWindowBase("MaterialWindow", false, false)
	{

	}

	void MaterialWindow::OnGui()
	{
		CPU_DEBUG_SCOPE("MaterialWindow");

		MaterialDefinition* matdef = m_CurrentMaterial->GetMaterialDef();

		ImGui::Text(matdef->GetName());
		ImGui::SameLine();
		if (ImGui::Button("Reload shader"))
		{
			matdef->ReloadShader();
		}

		static int ID = 0;
		ID = 0;

		const auto& textures = matdef->GetTextureVars();

		if (!textures.empty() && ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (const auto& it : textures)
			{
				ImGui::PushID(it.first);
				MTextureVar* var = m_CurrentMaterial->GetTextureVar(it.first);

				bool clearClicked = false;

				if (!var->Texture)
				{
					EUI::Slot(var->Name, (ITexture*)nullptr);
				}
				else
				{
					if(var->Texture->GetDesc().DimensionType != EDimensionType::TYPE_2D)
					{
						String name = "Unknown type";

						if(var->Texture->GetDesc().DimensionType == EDimensionType::TYPE_2DArray)
							name = Format("%s: [TextureArray(%d)]", var->Name.c_str(), var->Texture->GetDesc().DepthOrArraySize);

						if(var->Texture->GetDesc().DimensionType == EDimensionType::TYPE_3D)
							name = Format("%s: [Texture3D(%d)]", var->Name.c_str(), var->Texture->GetDesc().DepthOrArraySize);

						if(var->Texture->GetDesc().DimensionType == EDimensionType::TYPE_CubeMap)
							name = Format("%s: [CubeMap(%d)]", var->Name.c_str(), var->Texture->GetDesc().DepthOrArraySize);

						EUI::Slot(name, ICON_FA_IMAGE, &clearClicked);
					}
					else
					{
						EUI::Slot(var->Name, var->Texture.get(), &clearClicked);
					}
				}

				if (Texture_ptr textureDropped = EUI::AcceptTextureFileDrop())
				{
					m_CurrentMaterial->SetTexture(it.first, textureDropped);
				}

				if (Texture_ptr textureDropped = EUI::AcceptCubeMapTextureFileDrop())
				{
					m_CurrentMaterial->SetTexture(it.first, textureDropped);
				}

				if (clearClicked)
				{
					m_CurrentMaterial->SetTexture(it.first, nullptr);
				}

				ImGui::PopID();
			}
		}

		for (const auto& block : matdef->GetUniformBlocks())
		{
			ImGui::PushID(ID++);
			if (ImGui::CollapsingHeader(block.Name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				//ImGui::Indent();
				for (const auto& [typeID, var] : block.Vars)
				{
					const char* name = var.Connected ? var.ConnectedName.c_str() : var.Name.c_str();

					int componentCount = var.Size / sizeof(float);
					uint8* varMemory = m_CurrentMaterial->GetVariableMemory(typeID, var.Size);

					//TODO: Do this some other way than this hardcoded if
					if (var.Connected && var.Widget == "Color")
					{
						if (componentCount == 4)
							ImGui::ColorEdit4(name, reinterpret_cast<float*>(varMemory), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
						else if (componentCount == 3)
							ImGui::ColorEdit3(name, reinterpret_cast<float*>(varMemory), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
					}
					else
					{
						ImGui::DragScalarN(name, ImGuiDataType_Float, varMemory, componentCount, 0.01f);
					}
				}
			}

			ImGui::PopID();
		}
	}

	void MaterialWindow::Open(const Path &path)
	{
		if (ResourceManager::IsFileType(path, FT_MATERIAL_DEF))
		{
			Open(GEngine->GetResourceManager()->GetOrLoadMaterialDefinition(path));
		}
		else if(ResourceManager::IsFileType(path, FT_MATERIAL_INS))
		{
			Open(GEngine->GetResourceManager()->LoadMaterial(path));
		}
		else
		{
			AU_LOG_WARNING("Path ", path.string(), " is not an any material file!");
		}
	}

	void MaterialWindow::Open(const Material_ptr& materialInstance)
	{
		if (materialInstance)
		{
			OpenWindow(true);
		}

		m_CurrentMaterial = materialInstance;
	}
}