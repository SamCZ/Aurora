#include "Utils.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

namespace Aurora::EUI
{
	Texture_ptr AcceptTextureFileDrop()
	{
		Texture_ptr texture = nullptr;

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_PATH"))
			{
				Path filePath = (char*)payload->Data;

				if (ResourceManager::IsFileType(filePath, FT_IMAGE))
				{
					texture = GEngine->GetResourceManager()->LoadTexture(filePath);
				}
			}

			ImGui::EndDragDropTarget();
		}

		return texture;
	}

	Texture_ptr AcceptCubeMapTextureFileDrop()
	{
		Texture_ptr texture = nullptr;

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_PATH"))
			{
				Path filePath = (char*)payload->Data;

				if (ResourceManager::IsFileType(filePath, FT_CUBEMAP))
				{
					texture = GEngine->GetResourceManager()->LoadCubeMapDef(filePath);
				}
			}

			ImGui::EndDragDropTarget();
		}

		return texture;
	}
}