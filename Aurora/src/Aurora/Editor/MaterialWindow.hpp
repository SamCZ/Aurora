#pragma once

#include "WindowBase.hpp"
#include "Aurora/Graphics/Material/MaterialDefinition.hpp"
#include "Aurora/Graphics/Material/Material.hpp"

namespace Aurora
{
	class MaterialWindow : public EditorWindowBase
	{
	private:
		Material_ptr m_CurrentMaterial;
	public:
		MaterialWindow();

		void OnGui() override;

		void Open(const Path& path);
		void Open(const Material_ptr& materialInstance);

		[[nodiscard]] inline const Material_ptr& GetOpened() const { return m_CurrentMaterial; }
	};
}