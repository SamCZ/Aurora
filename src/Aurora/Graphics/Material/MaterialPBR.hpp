#pragma once

#include "Material.hpp"

namespace Aurora
{
	class MaterialPBR : public Material
	{
	public:
		CLASS_OBJ(MaterialPBR, Material)
	public:
		DEFINE_PARAM(MP_ALBEDO_MAP);
		DEFINE_PARAM(MP_NORMAL_MAP);
		DEFINE_PARAM(MP_ROUGHNESS_MAP);
		DEFINE_PARAM(MP_METALLIC_MAP);
		DEFINE_PARAM(MP_AO_MAP);
	private:

	public:
		MaterialPBR();

		void OnShaderReload(ResourceManager* rsm) override;
		void BeginPass(DrawCallState& drawState, EPassType passType) const override;
	};
}
