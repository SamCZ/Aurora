#include "DShape.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Framework/CameraComponent.hpp"
#include "Aurora/Framework/Lights.hpp"
#include "Aurora/Graphics/VgRender.hpp"

namespace Aurora
{
	std::vector<ShapeStructs::LineShape> DShapes::m_LineShapes;
	std::vector<ShapeStructs::BoxShape> DShapes::m_BoxShapes;
	std::vector<ShapeStructs::SphereShape> DShapes::m_SphereShapes;
	std::vector<ShapeStructs::ArrowShape> DShapes::m_ArrowShapes;
	std::vector<ShapeStructs::TextShape> DShapes::m_TextShapes;
	std::vector<ShapeStructs::TextShape> DShapes::m_OnScreenTextShapes;
	std::vector<ShapeStructs::TriangleShape> DShapes::m_TriangleShapes;

	struct BaseShapeVertex
	{
		Vector3 Position;
		Vector3 Color;
	};

	Buffer_ptr g_LineBuffer = nullptr;
	InputLayout_ptr g_LineInputLayout = nullptr;
	Shader_ptr g_LineShader = nullptr;

	void DShapes::Init()
	{
		uint32_t maxLines = 200000;
		uint32_t lineVertexBufferSize = (sizeof(Vector3) * 2) * maxLines;
		AU_LOG_INFO("Allocated ", FormatBytes(lineVertexBufferSize), " for debug line render.");

		g_LineBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("DebugLinesVBuffer", lineVertexBufferSize, EBufferType::VertexBuffer, EBufferUsage::DynamicDraw));
		g_LineInputLayout = GEngine->GetRenderDevice()->CreateInputLayout({
			{"POSITION", GraphicsFormat::RGB32_FLOAT, 0, offsetof(BaseShapeVertex, Position), 0, sizeof(BaseShapeVertex), false, false },
			{"COLOR", GraphicsFormat::RGB32_FLOAT, 0, offsetof(BaseShapeVertex, Color), 1, sizeof(BaseShapeVertex), false, false }
		});

		g_LineShader = GEngine->GetResourceManager()->LoadShader("DebugShaderBase", {
			{EShaderType::Vertex, "Assets/Shaders/World/Debug/base_shape.vss"},
			{EShaderType::Pixel, "Assets/Shaders/World/Debug/base_shape.fss"}
		});
	}

	void DShapes::Frustum(const Matrix4& imvp, const Color& col, const float z0, const float z1)
	{
		auto worldPointsToCover = DirectionalLightComponent::FrustumCorners(imvp, z0, z1);

		auto nearMid = (worldPointsToCover[0] +
			worldPointsToCover[1] +
			worldPointsToCover[3] +
			worldPointsToCover[2]) * 0.25f;

		Line(nearMid, worldPointsToCover[1], col);
		Line(nearMid, worldPointsToCover[3], col);
		Line(nearMid, worldPointsToCover[2], col);
		Line(nearMid, worldPointsToCover[0], col);

		Line(worldPointsToCover[0], worldPointsToCover[1], col);
		Line(worldPointsToCover[1], worldPointsToCover[3], col);
		Line(worldPointsToCover[3], worldPointsToCover[2], col);
		Line(worldPointsToCover[2], worldPointsToCover[0], col);

		Line(worldPointsToCover[0], worldPointsToCover[4], col);
		Line(worldPointsToCover[1], worldPointsToCover[5], col);
		Line(worldPointsToCover[2], worldPointsToCover[6], col);
		Line(worldPointsToCover[3], worldPointsToCover[7], col);

		Line(worldPointsToCover[4], worldPointsToCover[5], col);
		Line(worldPointsToCover[5], worldPointsToCover[7], col);
		Line(worldPointsToCover[7], worldPointsToCover[6], col);
		Line(worldPointsToCover[6], worldPointsToCover[4], col);
	}

	void DShapes::Render(DrawCallState &drawState)
	{
		{ // Render lines
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
				vertices->Color = currentShape.Color;
				vertices++;
				vertices->Position = currentShape.P1;
				vertices->Color = currentShape.Color;
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
						vertices->Color = nextShape.Color;
						vertices++;
						vertices->Position = nextShape.P1;
						vertices->Color = nextShape.Color;
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
		}

		{ // Render triangles
			std::sort(m_TriangleShapes.begin(), m_TriangleShapes.end(), [](const auto& left, const auto& right) -> bool
			{

				if(left.UseDepthBuffer < right.UseDepthBuffer) return true;
				if(left.Thickness < right.Thickness) return true;

				return false;
			});

			drawState.SetVertexBuffer(0, g_LineBuffer);
			drawState.InputLayoutHandle = g_LineInputLayout;
			drawState.Shader = g_LineShader;

			for (int i = 0; i < m_TriangleShapes.size(); ++i)
			{
				const ShapeStructs::TriangleShape& currentShape = m_TriangleShapes[i];

				BaseShapeVertex* vertices = GEngine->GetRenderDevice()->MapBuffer<BaseShapeVertex>(g_LineBuffer, EBufferAccess::WriteOnly);

				vertices->Position = currentShape.P0;
				vertices->Color = currentShape.Color;
				vertices++;
				vertices->Position = currentShape.P1;
				vertices->Color = currentShape.Color;
				vertices++;
				vertices->Position = currentShape.P2;
				vertices->Color = currentShape.Color;
				vertices++;

				uint32_t renderTriCount = 1;

				// If not at the end of an array
				if(i != m_TriangleShapes.size() - 1)
				{
					for (int j = i + 1; j < m_TriangleShapes.size(); ++j)
					{
						const ShapeStructs::TriangleShape& nextShape = m_TriangleShapes[j];

						if(nextShape.Thickness != currentShape.Thickness || nextShape.UseDepthBuffer != currentShape.UseDepthBuffer)
							break;

						vertices->Position = nextShape.P0;
						vertices->Color = nextShape.Color;
						vertices++;
						vertices->Position = nextShape.P1;
						vertices->Color = nextShape.Color;
						vertices++;
						vertices->Position = nextShape.P2;
						vertices->Color = nextShape.Color;
						vertices++;

						renderTriCount++;
						i++;
					}
				}

				GEngine->GetRenderDevice()->UnmapBuffer(g_LineBuffer);

				drawState.PrimitiveType = EPrimitiveType::TriangleList;
				drawState.RasterState.LineWidth = currentShape.Thickness;
				drawState.DepthStencilState.DepthEnable = currentShape.UseDepthBuffer;
				drawState.RasterState.CullMode = ECullMode::None;

				DrawArguments drawArguments;
				drawArguments.VertexCount = renderTriCount * 3;

				GEngine->GetRenderDevice()->Draw(drawState, {drawArguments});
			}
		}

		// TODO: Implement box, sphere and arrow shape render

		m_LineShapes.clear();
		m_BoxShapes.clear();
		m_SphereShapes.clear();
		m_ArrowShapes.clear();
		m_TriangleShapes.clear();
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

		for(const ShapeStructs::TextShape& shape : m_OnScreenTextShapes)
		{
			GEngine->GetVgRender()->DrawString(shape.Text, Vector2(shape.Position.x, shape.Position.y), shape.Color, 12.0f, VgAlign::Left, VgAlign::Center);
		}


		m_TextShapes.clear();
		m_OnScreenTextShapes.clear();
	}

	void DShapes::Reset()
	{
		m_TextShapes.clear();
		m_OnScreenTextShapes.clear();

		// Todo implement timeout
	}

	void DShapes::Destroy()
	{
		g_LineBuffer.reset();
		g_LineInputLayout.reset();
		g_LineShader.reset();
	}
}