#include "MaterialPBR.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

namespace Aurora
{
	DECLARE_PARAM(MaterialPBR, MP_ALBEDO_MAP);
	DECLARE_PARAM(MaterialPBR, MP_NORMAL_MAP);
	DECLARE_PARAM(MaterialPBR, MP_ROUGHNESS_MAP);
	DECLARE_PARAM(MaterialPBR, MP_METALLIC_MAP);
	DECLARE_PARAM(MaterialPBR, MP_AO_MAP);

	MaterialPBR::MaterialPBR()
	{
		SetName("PBR");

		MP_ALBEDO_MAP = CreateTextureParam("Albedo Texture");
		MP_NORMAL_MAP = CreateTextureParam("Normal Texture");
		MP_ROUGHNESS_MAP = CreateTextureParam("Roughness Texture");
		MP_METALLIC_MAP = CreateTextureParam("Metallic Texture");
		MP_AO_MAP = CreateTextureParam("AO Texture");
	}

	void MaterialPBR::OnShaderReload(ResourceManager* rsm)
	{
		// Load shaders
		//SetShader(EPassType::Depth, nullptr);
		SetShader(EPassType::Ambient, rsm->LoadShader("MaterialPBR", {
				{EShaderType::Vertex, "Shaders/World/PBRBasic/ambient.vss"},
				{EShaderType::Pixel, "Shaders/World/PBRBasic/ambient.fss"},
		}));
	}

	void MaterialPBR::BeginPass(DrawCallState& drawState, EPassType passType) const
	{
		Material::BeginPass(drawState, passType);

		switch (passType)
		{
			case EPassType::Depth:
			{
				break;
			}
			case EPassType::Ambient:
			{
				drawState.BindTexture("BaseColor", GetParamTexture(MP_ALBEDO_MAP));
				drawState.BindTexture("NormalMap", GetParamTexture(MP_NORMAL_MAP));
				break;
			}
		}

		// Set constants
	}
}
