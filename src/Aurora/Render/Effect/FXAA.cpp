#include "FXAA.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Graphics/RenderManager.hpp"

namespace Aurora
{
	FXAAMaterial::FXAAMaterial() : Material()
	{

	}

	void FXAAMaterial::OnShaderReload(ResourceManager *rsm)
	{
		m_Shader = rsm->LoadShader("FXAA", {
			{EShaderType::Vertex, "Assets/Shaders/PostProcess/fxaa.vss"},
			{EShaderType::Pixel, "Assets/Shaders/PostProcess/fxaa.fss"},
		});
	}

	void FXAAMaterial::BeginPass(DrawCallState& drawState, EPassType passType) const
	{

	}

	void FXAAEffect::Init()
	{

	}

	bool FXAAEffect::CanRender() const
	{
		bool canRender = PostProcessEffect::CanRender();

		if(canRender && GetMaterial() != nullptr && GetMaterial()->GetTypeID() == FXAAMaterial::TypeID())
		{
			return true;
		}

		return false;
	}

	void FXAAEffect::Render(const Texture_ptr& input, const Texture_ptr& output)
	{
		auto mat = FXAAMaterial::SafeCast(GetMaterial());

		DrawCallState drawState;
		drawState.Shader = mat->m_Shader;
		drawState.PrimitiveType = EPrimitiveType::TriangleStrip;
		drawState.ClearDepthTarget = false;
		drawState.ClearColorTarget = false;
		drawState.RasterState.CullMode = ECullMode::None;
		drawState.DepthStencilState.DepthEnable = false;
		drawState.ViewPort = output->GetDesc().GetSize();

		drawState.BindTexture("Texture", input);
		drawState.BindTarget(0, output);

		BEGIN_UB(Vector4, desc)
			desc->x = output->GetDesc().Width;
			desc->y = output->GetDesc().Height;
		END_UB(FXAADesc)

		GetEngine()->GetRenderDevice()->Draw(drawState, {DrawArguments(4)});
		GetEngine()->GetRenderManager()->GetUniformBufferCache().Reset();
	}
}