#include "DShape.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Framework/CameraComponent.hpp"

#include "Aurora/Render/VgRender.hpp"

namespace Aurora
{
	std::vector<ShapeStructs::LineShape> DShapes::m_LineShapes;
	std::vector<ShapeStructs::BoxShape> DShapes::m_BoxShapes;
	std::vector<ShapeStructs::SphereShape> DShapes::m_SphereShapes;
	std::vector<ShapeStructs::ArrowShape> DShapes::m_ArrowShapes;
	std::vector<ShapeStructs::TextShape> DShapes::m_TextShapes;

	struct BaseShapeVertex
	{
		Vector3 Position;
		uint32_t Color;
	};

	Buffer_ptr g_LineBuffer = nullptr;
	InputLayout_ptr g_LineInputLayout = nullptr;
	Shader_ptr g_LineShader = nullptr;

	void DShapes::Init()
	{
		uint32_t maxLines = 20000;
		uint32_t lineVertexBufferSize = (sizeof(Vector3) * 2) * maxLines;
		AU_LOG_INFO("Allocated ", FormatBytes(lineVertexBufferSize), " for debug line render.");

		g_LineBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("DebugLinesVBuffer", lineVertexBufferSize, EBufferType::VertexBuffer, EBufferUsage::DynamicDraw, true));
		g_LineInputLayout = GEngine->GetRenderDevice()->CreateInputLayout({
			{"POSITION", GraphicsFormat::RGB32_FLOAT, 0, offsetof(BaseShapeVertex, Position), 0, sizeof(BaseShapeVertex), false, true },
			{"COLOR", GraphicsFormat::R32_UINT, 0, offsetof(BaseShapeVertex, Color), 1, sizeof(BaseShapeVertex), false, true }
		});

		g_LineShader = GEngine->GetResourceManager()->LoadShader("DebugShaderBase", {
			{EShaderType::Vertex, "Assets/Shaders/World/Debug/base_shape.vss"},
			{EShaderType::Pixel, "Assets/Shaders/World/Debug/base_shape.fss"}
		});
	}

	void DShapes::Render(DrawCallState &drawState)
	{
		std::sort(m_LineShapes.begin(), m_LineShapes.end(), [](const ShapeStructs::LineShape& left, const ShapeStructs::LineShape& right) -> bool
		{

			if(left.UseDepthBuffer < right.UseDepthBuffer) return true;
			if(left.Thickness < right.Thickness) return true;

			return false;
		});

		drawState.SetVertexBuffer(0, g_LineBuffer);
		drawState.InputLayoutHandle = g_LineInputLayout;
		drawState.Shader = g_LineShader;

		int lineDrawCount = 0;

		for (int i = 0; i < m_LineShapes.size(); ++i)
		{
			const ShapeStructs::LineShape& currentShape = m_LineShapes[i];

			BaseShapeVertex* vertices = GEngine->GetRenderDevice()->MapBuffer<BaseShapeVertex>(g_LineBuffer, EBufferAccess::WriteOnly);

			vertices->Position = currentShape.P0;
			vertices->Color = currentShape.Color.rgba;
			vertices++;
			vertices->Position = currentShape.P1;
			vertices->Color = currentShape.Color.rgba;
			vertices++;

			uint32_t renderLineCount = 1;

			 // If not at the end of an array
			if(i != m_LineShapes.size() - 1)
			{
				for (int j = i + 1; j < m_LineShapes.size(); ++j)
				{
					const ShapeStructs::LineShape& nextShape = m_LineShapes[j];

					if(nextShape.Thickness != currentShape.Thickness || nextShape.UseDepthBuffer != currentShape.UseDepthBuffer)
						break;

					vertices->Position = nextShape.P0;
					vertices->Color = nextShape.Color.rgba;
					vertices++;
					vertices->Position = nextShape.P1;
					vertices->Color = nextShape.Color.rgba;
					vertices++;

					renderLineCount++;
					i++;
				}
			}

			GEngine->GetRenderDevice()->UnmapBuffer(g_LineBuffer);

			drawState.PrimitiveType = EPrimitiveType::LineList;
			drawState.RasterState.LineWidth = currentShape.Thickness;
			drawState.DepthStencilState.DepthEnable = currentShape.UseDepthBuffer;

			DrawArguments drawArguments;
			drawArguments.VertexCount = renderLineCount * 2;
			GEngine->GetRenderDevice()->Draw(drawState, {drawArguments});

			lineDrawCount++;
		}

		// TODO: Implement box, sphere and arrow shape render

		m_LineShapes.clear();
		m_BoxShapes.clear();
		m_SphereShapes.clear();
		m_ArrowShapes.clear();
	}

	void DShapes::RenderText(CameraComponent* camera)
	{
		for(const ShapeStructs::TextShape& shape : m_TextShapes)
		{
			Vector2 coords;
			if(camera->GetScreenCoordinatesFromWorld(shape.Position, coords))
			{
				GEngine->GetVgRender()->DrawString(shape.Text, coords, shape.Color, 12.0f, VgAlign::Center, VgAlign::Center);
			}
		}

		m_TextShapes.clear();
	}

	void DShapes::Reset()
	{
		m_TextShapes.clear();

		// Todo implement timeout
	}

	void DShapes::Destroy()
	{
		g_LineBuffer.reset();
		g_LineInputLayout.reset();
		g_LineShader.reset();
	}
}