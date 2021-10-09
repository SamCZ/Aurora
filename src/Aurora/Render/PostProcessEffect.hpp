#pragma once

#include "Aurora/Core/Object.hpp"

namespace Aurora
{
	class Material;

	class PostProcessEffect : public ObjectBase
	{
	private:
		std::shared_ptr<Material> m_Material;
	public:
		CLASS_OBJ(PostProcessEffect, ObjectBase)
	public:
		PostProcessEffect() : m_Material(nullptr) {}
		~PostProcessEffect() override = default;

		virtual void Init() = 0;
		virtual void Render() = 0;

		[[nodiscard]] const std::shared_ptr<Material>& GetMaterial() const
		{
			return m_Material;
		}

	protected:
		void SetMaterial(std::shared_ptr<Material> material) { m_Material = std::move(material); }
	};
}