#pragma once

#include "Material.hpp"

namespace Aurora
{
	class AU_API MaterialPBR : public Material
	{
	public:
		CLASS_OBJ(MaterialPBR, Material)
	public:
		DEFINE_PARAM(MP_ALBEDO_MAP);
		DEFINE_PARAM(MP_NORMAL_MAP);
		DEFINE_PARAM(MP_ROUGHNESS_MAP);
		DEFINE_PARAM(MP_METALLIC_MAP);
		DEFINE_PARAM(MP_METALLIC_ROUGNESS_MAP);
		DEFINE_PARAM(MP_AO_MAP);
		DEFINE_PARAM(MP_EMISSION_MAP);

		DEFINE_PARAM(MP_BASE_COLOR);

	private:
		Buffer_ptr m_PBRBuffer; // TODO: This needs to be only one instance, implement material classes!
	public:
		MaterialPBR();

		void OnShaderReload(ResourceManager* rsm) override;
		void BeginPass(DrawCallState& drawState, EPassType passType) const override;
	};
}
