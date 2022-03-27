#pragma once

#include "Aurora/Core/Types.hpp"
#include "Aurora/Graphics/Material/MaterialDefinition.hpp"
#include "Aurora/Graphics/Material/Material.hpp"

namespace Aurora
{
	class MaterialWindow
	{
	private:
		MaterialDefinition_ptr m_CurrentMaterialDef;
		Material_ptr m_CurrentMaterialInstanceDef;
		bool m_WindowOpened;
		bool m_WindowNeedsFocus;
	public:
		MaterialWindow();

		void Update(double delta);

		void Open(const Path& path);
		void Open(const MaterialDefinition_ptr& materialDef);
		void Open(const Material_ptr& materialInstance);
	};
}