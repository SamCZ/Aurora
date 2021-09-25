#pragma once

#include <memory>
#include <cassert>

namespace Aurora
{
	class Material;

	class MaterialSet
	{
	private:
		std::map<uint, std::shared_ptr<Material>> m_Materials;
	public:
		MaterialSet() : m_Materials() {}

		void SetMaterial(uint i, const std::shared_ptr<Material>& material)
		{
			m_Materials[i] = material;
		}

		[[nodiscard]] bool HasMaterial(uint i) const
		{
			return m_Materials.contains(i);
		}

		[[nodiscard]] const std::shared_ptr<Material>& GetMaterial(uint i)
		{
			assert(HasMaterial(i));
			return m_Materials[i];
		}

		std::map<uint, std::shared_ptr<Material>>::size_type GetCount() const
		{
			return m_Materials.size();
		}
	};
}