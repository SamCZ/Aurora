#include "MaterialWindow.hpp"

#include "Utils.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Aurora.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Tools/IconsFontAwesome5.hpp"

namespace Aurora
{

	MaterialWindow::MaterialWindow() : m_WindowOpened(false), m_WindowNeedsFocus(true)
	{

	}

	void MaterialWindow::Update(double delta)
	{
		if (!m_WindowOpened)
		{
			m_WindowNeedsFocus = true;
			return;
		}

		if(m_WindowNeedsFocus)
		{
			ImGui::SetNextWindowFocus();
			m_WindowNeedsFocus = false;
		}

		if (ImGui::Begin("MaterialWindow", &m_WindowOpened))
		{
			ImGui::Text("yay");

			if (m_CurrentMaterialDef)
			{
				ImGui::Text(m_CurrentMaterialDef->GetName());
			}
		}
		ImGui::End();
	}

	void MaterialWindow::Open(const Path &path)
	{
		if (ResourceManager::IsFileType(path, FT_MATERIAL_DEF))
		{
			Open(GEngine->GetResourceManager()->GetOrLoadMaterialDefinition(path));
		}
		else if(ResourceManager::IsFileType(path, FT_MATERIAL_INS))
		{
			AU_LOG_WARNING("Opening material instances in material window is not implemented yet!");
			//Open(GEngine->GetResourceManager()->LoadMaterial(path))
		}
		else
		{
			AU_LOG_WARNING("Path ", path.string(), " is not an any material file!");
		}
	}

	void MaterialWindow::Open(const MaterialDefinition_ptr& materialDef)
	{
		if (materialDef)
		{
			m_WindowOpened = true;
			m_WindowNeedsFocus = true;
		}

		m_CurrentMaterialDef = materialDef;
		m_CurrentMaterialInstanceDef = nullptr;
	}

	void MaterialWindow::Open(const Material_ptr& materialInstance)
	{
		if (materialInstance)
		{
			m_WindowOpened = true;
			m_WindowNeedsFocus = true;
		}

		m_CurrentMaterialDef = nullptr;
		m_CurrentMaterialInstanceDef = materialInstance;
	}
}