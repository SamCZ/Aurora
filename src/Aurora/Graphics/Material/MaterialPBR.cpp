#include "MaterialPBR.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Graphics/RenderManager.hpp"
#include "Shaders/World/PBRBasic/cb_pbr.h"

namespace Aurora
{
	DECLARE_PARAM(MaterialPBR, MP_ALBEDO_MAP);
	DECLARE_PARAM(MaterialPBR, MP_NORMAL_MAP);
	DECLARE_PARAM(MaterialPBR, MP_ROUGHNESS_MAP);
	DECLARE_PARAM(MaterialPBR, MP_METALLIC_MAP);
	DECLARE_PARAM(MaterialPBR, MP_METALLIC_ROUGNESS_MAP);
	DECLARE_PARAM(MaterialPBR, MP_AO_MAP);
	DECLARE_PARAM(MaterialPBR, MP_EMISSION_MAP);

	DECLARE_PARAM(MaterialPBR, MP_BASE_COLOR);

	MaterialPBR::MaterialPBR() : Material()
	{
		SetName("PBR");

		MP_ALBEDO_MAP = CreateTextureParam("Albedo Texture");
		MP_NORMAL_MAP = CreateTextureParam("Normal Texture");
		MP_ROUGHNESS_MAP = CreateTextureParam("Roughness Texture");
		MP_METALLIC_MAP = CreateTextureParam("Metallic Texture");
		MP_AO_MAP = CreateTextureParam("AO Texture");
		MP_EMISSION_MAP = CreateTextureParam("Emission Texture");
		MP_METALLIC_ROUGNESS_MAP = CreateTextureParam("MR Texture");

		MP_BASE_COLOR = CreateVec3Param("BaseColor", {0, 0, 0});

		m_PBRBuffer = GetEngine()->GetRenderDevice()->CreateBuffer(BufferDesc("PBRConstants", sizeof(PBRConstants), EBufferType::UniformBuffer));
	}

	void MaterialPBR::OnShaderReload(ResourceManager* rsm)
	{
		// Load shaders
		SetShader(EPassType::Depth, rsm->LoadShader("MaterialPBR-Depth", {
				{EShaderType::Vertex, "Assets/Shaders/World/PBRBasic/depth.vss"},
				{EShaderType::Pixel, "Assets/Shaders/World/PBRBasic/depth.fss"},
		}));

		SetShader(EPassType::Ambient, rsm->LoadShader("MaterialPBR-Ambient", {
				{EShaderType::Vertex, "Assets/Shaders/World/PBRBasic/ambient.vss"},
				{EShaderType::Pixel, "Assets/Shaders/World/PBRBasic/ambient.fss"},
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
				drawState.BindTexture("BaseColorMap", GetParamTexture(MP_ALBEDO_MAP));
				drawState.BindTexture("NormalMap", GetParamTexture(MP_NORMAL_MAP));
				drawState.BindTexture("EmissionMap", GetParamTexture(MP_EMISSION_MAP));
				drawState.BindTexture("MetallicRoughnessMap", GetParamTexture(MP_METALLIC_ROUGNESS_MAP));

				drawState.BindSampler("BaseColorMap", Samplers::WrapWrapLinearLinear);
				drawState.BindSampler("NormalMap", Samplers::WrapWrapLinearLinear);
				drawState.BindSampler("EmissionMap", Samplers::WrapWrapLinearLinear);
				drawState.BindSampler("MetallicRoughnessMap", Samplers::WrapWrapLinearLinear);

				BEGIN_UBW(PBRConstants, desc)
					desc->BaseColor = Vector4(GetParamVec3Value(MP_BASE_COLOR), 0);
				END_UBW(drawState, m_PBRBuffer, "PBRConstants")

				break;
			}
		}

		// Set constants
	}
}
