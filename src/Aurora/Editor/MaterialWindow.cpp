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

		static int ID = 0;
		ID = 0;

		const auto& textures = matdef->GetTextureVars();

		if (!textures.empty() && ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (const auto& it : textures)
			{
				MTextureVar* var = m_CurrentMaterial->GetTextureVar(it.first);

				if (!var->Texture)
				{
					ImGui::Text("%s: No Image", var->Name.c_str());
				}
				else
				{
					if(var->Texture->GetDesc().DimensionType != EDimensionType::TYPE_2D)
					{
						if(var->Texture->GetDesc().DimensionType == EDimensionType::TYPE_2DArray)
							ImGui::Text("%s: [TextureArray(%d)]", var->Name.c_str(), var->Texture->GetDesc().DepthOrArraySize);

						if(var->Texture->GetDesc().DimensionType == EDimensionType::TYPE_3D)
							ImGui::Text("%s: [Texture3D(%d)]", var->Name.c_str(), var->Texture->GetDesc().DepthOrArraySize);

						if(var->Texture->GetDesc().DimensionType == EDimensionType::TYPE_CubeMap)
							ImGui::Text("%s: [CubeMap(%d)]", var->Name.c_str(), var->Texture->GetDesc().DepthOrArraySize);
					}
					else
					{
						ImGui::Text("%s", var->Name.c_str());
						EUI::Image(var->Texture, Vector2(55, 55), false);
					}
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_PATH"))
					{
						Path filePath = (char*)payload->Data;

						if (ResourceManager::IsFileType(filePath, FT_IMAGE))
						{
							m_CurrentMaterial->SetTexture(it.first, GEngine->GetResourceManager()->LoadTexture(filePath));
						}
					}

					ImGui::EndDragDropTarget();
				}
			}
		}

		for (const auto& block : matdef->GetUniformBlocks())
		{
			ImGui::PushID(ID++);
			if (ImGui::CollapsingHeader(block.Name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				//ImGui::Indent();
				for (const auto& it : block.Vars)
				{
					ImGui::Text("%s", it.second.Name.c_str());

					const MUniformVar& var = it.second;
					int componentCount = var.Size / sizeof(float);
					uint8* varMemory = m_CurrentMaterial->GetVariableMemory(it.first, var.Size);

					ImGui::DragScalarN(var.Name.c_str(), ImGuiDataType_Float, varMemory, componentCount, 0.01f);
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