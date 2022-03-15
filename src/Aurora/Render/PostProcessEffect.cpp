#include "PostProcessEffect.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Graphics/RenderManager.hpp"

namespace Aurora
{

	DrawCallState PostProcessEffect::PrepareState(const Shader_ptr &shader)
	{
		DrawCallState drawState;
		drawState.Shader = shader;
		drawState.PrimitiveType = EPrimitiveType::TriangleStrip;
		drawState.ClearDepthTarget = false;
		drawState.ClearColorTarget = false;
		drawState.RasterState.CullMode = ECullMode::None;
		drawState.DepthStencilState.DepthEnable = false;
		return drawState;
	}

	void PostProcessEffect::RenderState(const DrawCallState& state)
	{
		GEngine->GetRenderDevice()->Draw(state, {DrawArguments(4)});
		GEngine->GetRenderManager()->GetUniformBufferCache().Reset();
	}
}