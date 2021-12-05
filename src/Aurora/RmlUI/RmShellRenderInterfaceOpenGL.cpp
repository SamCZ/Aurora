#include "RmShellRenderInterfaceOpenGL.hpp"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>

#include "Aurora/Aurora.hpp"
#include "Aurora/Core/Types.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Graphics/OpenGL/GL.hpp"
#include "Aurora/Graphics/RenderManager.hpp"

namespace Aurora
{
	RmShellRenderInterfaceOpenGL::RmShellRenderInterfaceOpenGL()
	: m_Width(0), m_Height(0), m_TransformEnabled(false), m_RegisteredCustomTextures(), m_Scissors(), m_Transform(glm::identity<Matrix4>())
	{
		m_ColorShader = GetEngine()->GetResourceManager()->LoadShader("RmlUIColored", {
			{EShaderType::Vertex, "Assets/Shaders/RmlUI/vs_color.vss"},
			{EShaderType::Pixel, "Assets/Shaders/RmlUI/ps_color.fss"},
		});

		m_TexturedShader = GetEngine()->GetResourceManager()->LoadShader("RmlUITextured", {
			{EShaderType::Vertex, "Assets/Shaders/RmlUI/vs_textured.vss"},
			{EShaderType::Pixel, "Assets/Shaders/RmlUI/ps_textured.fss"},
		});

		m_ColorInputLayout = GetEngine()->GetRenderDevice()->CreateInputLayout({
			VertexAttributeDesc{"in_Pos", GraphicsFormat::RG32_FLOAT, 0, offsetof(Rml::Vertex, position), 0, sizeof(Rml::Vertex), false, false},
			VertexAttributeDesc{"in_Color", GraphicsFormat::R32_UINT, 0, offsetof(Rml::Vertex, colour), 1, sizeof(Rml::Vertex), false, false}
		});

		m_TexturedInputLayout = GetEngine()->GetRenderDevice()->CreateInputLayout({
			VertexAttributeDesc{"in_Pos", GraphicsFormat::RG32_FLOAT, 0, offsetof(Rml::Vertex, position), 0, sizeof(Rml::Vertex), false, false},
			VertexAttributeDesc{"in_Color", GraphicsFormat::R32_UINT, 0, offsetof(Rml::Vertex, colour), 1, sizeof(Rml::Vertex), false, false},
			VertexAttributeDesc{"in_TexCoord", GraphicsFormat::RG32_FLOAT, 0, offsetof(Rml::Vertex, tex_coord), 2, sizeof(Rml::Vertex), false, true}
		});

		m_VertexBuffer = GetEngine()->GetRenderDevice()->CreateBuffer(BufferDesc("RmlVertexBuffer", sizeof(Rml::Vertex) * 3000, EBufferType::VertexBuffer, EBufferUsage::DynamicDraw));
		m_IndexBuffer = GetEngine()->GetRenderDevice()->CreateBuffer(BufferDesc("RmlIndexBuffer", sizeof(uint32_t) * 3000, EBufferType::IndexBuffer, EBufferUsage::DynamicDraw));

		m_VertexUniformBuffer = GetEngine()->GetRenderDevice()->CreateBuffer(BufferDesc("UB", sizeof(VertexUniform), EBufferType::UniformBuffer, EBufferUsage::DynamicDraw));
		m_ScissorBuffer = GetEngine()->GetRenderDevice()->CreateBuffer(BufferDesc("SB", sizeof(Scissors), EBufferType::UniformBuffer, EBufferUsage::DynamicDraw));
	}

	RmShellRenderInterfaceOpenGL::~RmShellRenderInterfaceOpenGL() = default;

	void RmShellRenderInterfaceOpenGL::RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, const Rml::TextureHandle texture, const Rml::Vector2f& translation)
	{
		auto screenSize = GetEngine()->GetWindow()->GetSize();

		if(m_LastScreenSize != screenSize)
		{
			m_LastScreenSize = screenSize;
			m_CurrentProjection = glm::ortho(0.0f, (float)screenSize.x, (float)screenSize.y, 0.0f, 0.0f, 1.0f);
		}

		au_assert((sizeof(Rml::Vertex) * num_vertices) <= sizeof(Rml::Vertex) * 3000);
		au_assert((sizeof(uint32_t) * num_indices) <= sizeof(uint32_t) * 3000);

		GetEngine()->GetRenderDevice()->WriteBuffer(m_VertexBuffer, vertices, sizeof(Rml::Vertex) * num_vertices, 0);
		GetEngine()->GetRenderDevice()->WriteBuffer(m_IndexBuffer, indices, sizeof(uint32_t) * num_indices, 0);

		DrawCallState drawCallState;
		drawCallState.ViewPort = screenSize;
		drawCallState.Shader = texture ? m_TexturedShader : m_ColorShader;
		drawCallState.InputLayoutHandle = texture ? m_TexturedInputLayout : m_ColorInputLayout;

		drawCallState.SetVertexBuffer(0, m_VertexBuffer);
		drawCallState.SetIndexBuffer(m_IndexBuffer, EIndexBufferFormat::Uint32);

		if(texture) {
			auto* texture_handle = (TexHandle*)texture;
			drawCallState.BindTexture("Texture", texture_handle->Texture);
			drawCallState.BindSampler("Texture", Samplers::WrapWrapNearNearestFarLinear);
		}

		drawCallState.DepthStencilState.DepthEnable = false;
		drawCallState.ClearColorTarget = false;
		drawCallState.ClearDepthTarget = false;

		drawCallState.RasterState.CullMode = ECullMode::None;

		BEGIN_UBW(VertexUniform, desc);
			desc->Projection = m_CurrentProjection;

			if(m_TransformEnabled)
			{
				desc->ModelMat = m_Transform * glm::translate(Vector3(translation.x, translation.y, 0));
			}
			else
			{
				desc->ModelMat = glm::translate(Vector3(translation.x, translation.y, 0));
			}
		END_UBW(drawCallState, m_VertexUniformBuffer, "VertexUniform");

		BEGIN_UBW(Scissors, desc);
			*desc = m_Scissors;
		END_UBW(drawCallState, m_ScissorBuffer, "Scissors");

		GetEngine()->GetRenderDevice()->DrawIndexed(drawCallState, {DrawArguments(num_indices)});
	}

	Rml::CompiledGeometryHandle RmShellRenderInterfaceOpenGL::CompileGeometry(Rml::Vertex* RMLUI_UNUSED_PARAMETER(vertices), int RMLUI_UNUSED_PARAMETER(num_vertices), int* RMLUI_UNUSED_PARAMETER(indices), int RMLUI_UNUSED_PARAMETER(num_indices), const Rml::TextureHandle RMLUI_UNUSED_PARAMETER(texture))
	{
		RMLUI_UNUSED(vertices);
		RMLUI_UNUSED(num_vertices);
		RMLUI_UNUSED(indices);
		RMLUI_UNUSED(num_indices);
		RMLUI_UNUSED(texture);

		return (Rml::CompiledGeometryHandle) nullptr;
	}

	void RmShellRenderInterfaceOpenGL::RenderCompiledGeometry(Rml::CompiledGeometryHandle RMLUI_UNUSED_PARAMETER(geometry), const Rml::Vector2f& RMLUI_UNUSED_PARAMETER(translation))
	{
		RMLUI_UNUSED(geometry);
		RMLUI_UNUSED(translation);
	}

	void RmShellRenderInterfaceOpenGL::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle RMLUI_UNUSED_PARAMETER(geometry))
	{
		RMLUI_UNUSED(geometry);
	}

	void RmShellRenderInterfaceOpenGL::EnableScissorRegion(bool enable)
	{
		auto screenSize = GetEngine()->GetWindow()->GetSize();

		m_Scissors.sSettings.x = enable;
		m_Scissors.sSettings.y = static_cast<float>(screenSize.y);
	}

	void RmShellRenderInterfaceOpenGL::SetScissorRegion(int x, int y, int width, int height)
	{
		m_Scissors.sRect.x = static_cast<float>(x);
		m_Scissors.sRect.y = static_cast<float>(y);
		m_Scissors.sRect.z = static_cast<float>(x + width);
		m_Scissors.sRect.w = static_cast<float>(y + height);
	}

	bool RmShellRenderInterfaceOpenGL::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source)
	{
		Path path(source);

		if(source.starts_with(".."))
		{
			path = source.substr(3);
		}

		{
			auto customTexture = m_RegisteredCustomTextures.find(path.filename().string());

			if(customTexture != m_RegisteredCustomTextures.end())
			{
				const Texture_ptr& customTextureHandle = customTexture->second;

				texture_dimensions.x = static_cast<int>(customTextureHandle->GetDesc().Width);
				texture_dimensions.y = static_cast<int>(customTextureHandle->GetDesc().Height);

				auto* handle = new TexHandle();
				handle->Texture = customTextureHandle;
				texture_handle = (Rml::TextureHandle) handle;

				return true;
			}
		}

		auto texturePtr = GetEngine()->GetResourceManager()->LoadTexture(path, GraphicsFormat::RGBA8_UNORM, {false});

		if(!texturePtr) {
			return false;
		}

		texture_dimensions.x = static_cast<int>(texturePtr->GetDesc().Width);
		texture_dimensions.y = static_cast<int>(texturePtr->GetDesc().Height);

		auto* handle = new TexHandle();
		handle->Texture = texturePtr;
		texture_handle = (Rml::TextureHandle) handle;

		return true;
	}

	bool RmShellRenderInterfaceOpenGL::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions)
	{
		TextureDesc textureDesc;
		textureDesc.Width = source_dimensions.x;
		textureDesc.Height = source_dimensions.y;
		textureDesc.ImageFormat = GraphicsFormat::RGBA8_UNORM;
		textureDesc.MipLevels = 1;
		textureDesc.Usage = TextureDesc::EUsage::Default;

		Texture_ptr texturePtr = GetEngine()->GetRenderDevice()->CreateTexture(textureDesc);
		GetEngine()->GetRenderDevice()->WriteTexture(texturePtr, 0, 0, source);

		auto* handle = new TexHandle();
		handle->Texture = texturePtr;
		texture_handle = (Rml::TextureHandle) handle;

		return true;
	}

	void RmShellRenderInterfaceOpenGL::ReleaseTexture(Rml::TextureHandle texture_handle)
	{
		auto* handle = (TexHandle*)texture_handle;
		delete handle;
	}

	void RmShellRenderInterfaceOpenGL::SetTransform(const Rml::Matrix4f* transform)
	{
		m_TransformEnabled = (bool)transform;

		if (transform)
		{
			if (std::is_same<Rml::Matrix4f, Rml::ColumnMajorMatrix4f>::value) {
				m_Transform = glm::make_mat4(transform->data());
			}
			else if (std::is_same<Rml::Matrix4f, Rml::RowMajorMatrix4f>::value) {
				m_Transform = glm::make_mat4(transform->Transpose().data());
			}
		}
		else
		{
			m_Transform = glm::identity<Matrix4>();
		}
	}

	RmShellRenderInterfaceOpenGL::Image RmShellRenderInterfaceOpenGL::CaptureScreen()
	{
		Image image;
		image.num_components = 3;
		image.width = m_Width;
		image.height = m_Height;

		const int byte_size = image.width * image.height * image.num_components;
		image.data = Rml::UniquePtr<Rml::byte[]>(new Rml::byte[byte_size]);

		glReadPixels(0, 0, image.width, image.height, GL_RGB, GL_UNSIGNED_BYTE, image.data.get());

		bool result = true;
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			result = false;
			Rml::Log::Message(Rml::Log::LT_ERROR, "Could not capture screenshot, got GL error: 0x%x", err);
		}

		if (!result)
			return Image();

		return image;
	}

	void RmShellRenderInterfaceOpenGL::SetViewport(int width, int height)
	{
		m_Width = width;
		m_Height = height;
	}

	void RmShellRenderInterfaceOpenGL::PrepareRenderBuffer()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void RmShellRenderInterfaceOpenGL::PresentRenderBuffer()
	{
		glDisable(GL_BLEND);
	}

	bool RmShellRenderInterfaceOpenGL::SetCustomTextureHandleForeName(const std::string &name, const Texture_ptr &texture)
	{
		m_RegisteredCustomTextures[name] = texture;
		return true;
	}
}