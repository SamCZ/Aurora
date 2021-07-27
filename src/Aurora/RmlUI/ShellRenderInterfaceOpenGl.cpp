#include "ShellRenderInterfaceOpenGL.hpp"
#include "Aurora/Graphics/OpenGL/GLRenderDevice.hpp"
#include "Aurora/AuroraEngine.hpp"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <type_traits>
#include "Aurora/Graphics/OpenGL/GL.hpp"
#include <iostream>

#include "RmlShaders.hpp"

#define GL_CLAMP_TO_EDGE 0x812F

namespace Aurora
{
	struct TexHandle
	{
		Texture_ptr Texture;
	};

	struct VertexUniform
	{
		Matrix4 Projection;
		Matrix4 ModelMat;
	};

	struct Scissors
	{
		Vector4 sRect;
		Vector4 sSettings;
	};

	GLuint VBO = 0;
	Buffer_ptr vertexBuffer;
	Buffer_ptr indexBuffer;
	Buffer_ptr uniformBuffer;
	Buffer_ptr scissorBuffer;
	Shader_ptr shaderColored;
	Shader_ptr shaderTextured;
	InputLayout_ptr inputLayout;
	Matrix4 g_Transform;
	Sampler_ptr g_Sampler;

	Scissors scissorsData = {};

	ShellRenderInterfaceOpenGL::ShellRenderInterfaceOpenGL() : m_width(0), m_height(0), m_transform_enabled(false)
	{
		glGenBuffers(1, &VBO);

		shaderColored = ASM->CreateShaderProgram({
			{EShaderType::Vertex, RmlVertexShader},
			{EShaderType::Pixel, RmlColorFragmentShader}
		});
		shaderTextured = ASM->CreateShaderProgram({
			{EShaderType::Vertex, RmlVertexShader},
			{EShaderType::Pixel, RmlTexturedFragmentShader}
		});

		vertexBuffer = RD->CreateBuffer(BufferDesc("VB", sizeof(Rml::Vertex) * 3000, sizeof(Rml::Vertex), EBufferType::VertexBuffer, EBufferUsage::DynamicDraw));
		indexBuffer = RD->CreateBuffer(BufferDesc("IB", sizeof(uint32_t) * 3000, sizeof(uint32_t), EBufferType::IndexBuffer, EBufferUsage::DynamicDraw));
		uniformBuffer = RD->CreateBuffer(BufferDesc("UB", sizeof(VertexUniform), 0, EBufferType::UniformBuffer, EBufferUsage::DynamicDraw));
		scissorBuffer = RD->CreateBuffer(BufferDesc("SB", sizeof(Scissors), 0, EBufferType::UniformBuffer, EBufferUsage::DynamicDraw));
		inputLayout = RD->CreateInputLayout({
			VertexAttributeDesc{"in_Pos", GraphicsFormat::RG32_FLOAT, 0, offsetof(Rml::Vertex, position), false, 0},
			VertexAttributeDesc{"in_Color", GraphicsFormat::R32_UINT, 0, offsetof(Rml::Vertex, colour), false, 1},
			VertexAttributeDesc{"in_TexCoord", GraphicsFormat::RG32_FLOAT, 0, offsetof(Rml::Vertex, tex_coord), false, 2},
		});
		g_Transform = glm::identity<Matrix4>();
		g_Sampler = RD->CreateSampler(SamplerDesc());

		scissorsData.sSettings.x = 0;
	}

	void ShellRenderInterfaceOpenGL::RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, const Rml::TextureHandle texture, const Rml::Vector2f& translation)
	{
		auto* renderDevice = static_cast<GLRenderDevice*>(RD);
		GLContextState& contextState = renderDevice->GetContextState();

		auto screenSize = AuroraEngine::GetCurrentThreadContext()->GetWindow()->GetSize();

		VertexUniform vertexUniform = {};
		vertexUniform.Projection = glm::ortho(0.0f, (float)screenSize.x, (float)screenSize.y, 0.0f, -1.0f, 1.0f);
		vertexUniform.ModelMat = g_Transform * glm::translate(Vector3(translation.x, translation.y, 0));
		renderDevice->WriteBuffer(uniformBuffer, &vertexUniform, sizeof(VertexUniform));

		renderDevice->WriteBuffer(scissorBuffer, &scissorsData, sizeof(Scissors));


		renderDevice->WriteBuffer(vertexBuffer, vertices, sizeof(Rml::Vertex) * num_vertices);
		renderDevice->WriteBuffer(indexBuffer, indices, sizeof(uint32_t) * num_indices);


		DrawCallState drawCallState;
		drawCallState.Shader = texture ? shaderTextured : shaderColored;
		drawCallState.InputLayoutHandle = inputLayout;
		drawCallState.SetVertexBuffer(0, vertexBuffer);
		drawCallState.SetIndexBuffer(indexBuffer, EIndexBufferFormat::Uint32);
		drawCallState.BoundUniformBuffers["VertexUniform"] = uniformBuffer;
		drawCallState.BoundUniformBuffers["Scissors"] = scissorBuffer;

		if(texture) {
			auto* texture_handle = (TexHandle*)texture;
			drawCallState.BindTexture("Texture", texture_handle->Texture);
			drawCallState.BindSampler("Texture", g_Sampler);
		}

		drawCallState.ClearColorTarget = false;
		drawCallState.ClearDepthTarget = false;
		drawCallState.DepthStencilState.DepthEnable = false;
		drawCallState.RasterState.CullMode = ECullMode::None;

		renderDevice->DrawIndexed(drawCallState, {DrawArguments(num_indices)});
	}

	Rml::CompiledGeometryHandle ShellRenderInterfaceOpenGL::CompileGeometry(Rml::Vertex* RMLUI_UNUSED_PARAMETER(vertices), int RMLUI_UNUSED_PARAMETER(num_vertices), int* RMLUI_UNUSED_PARAMETER(indices), int RMLUI_UNUSED_PARAMETER(num_indices), const Rml::TextureHandle RMLUI_UNUSED_PARAMETER(texture))
	{
		RMLUI_UNUSED(vertices);
		RMLUI_UNUSED(num_vertices);
		RMLUI_UNUSED(indices);
		RMLUI_UNUSED(num_indices);
		RMLUI_UNUSED(texture);

		return (Rml::CompiledGeometryHandle) nullptr;
	}

	void ShellRenderInterfaceOpenGL::RenderCompiledGeometry(Rml::CompiledGeometryHandle RMLUI_UNUSED_PARAMETER(geometry), const Rml::Vector2f& RMLUI_UNUSED_PARAMETER(translation))
	{
		RMLUI_UNUSED(geometry);
		RMLUI_UNUSED(translation);
	}

	void ShellRenderInterfaceOpenGL::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle RMLUI_UNUSED_PARAMETER(geometry))
	{
		RMLUI_UNUSED(geometry);
	}

	void ShellRenderInterfaceOpenGL::EnableScissorRegion(bool enable)
	{
		auto screenSize = AuroraEngine::GetCurrentThreadContext()->GetWindow()->GetSize();

		scissorsData.sSettings.x = enable;
		scissorsData.sSettings.y = static_cast<int>(screenSize.y);
	}

	void ShellRenderInterfaceOpenGL::SetScissorRegion(int x, int y, int width, int height)
	{
		scissorsData.sRect.x = static_cast<float>(x);
		scissorsData.sRect.y = static_cast<float>(y);
		scissorsData.sRect.z = static_cast<float>(x + width);
		scissorsData.sRect.w = static_cast<float>(y + height);
	}

	bool ShellRenderInterfaceOpenGL::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source)
	{
		Rml::FileInterface* file_interface = Rml::GetFileInterface();
		Rml::FileHandle file_handle = file_interface->Open(source);
		if (!file_handle)
		{
			return false;
		}

		file_interface->Seek(file_handle, 0, SEEK_END);
		size_t buffer_size = file_interface->Tell(file_handle);
		file_interface->Seek(file_handle, 0, SEEK_SET);

		DataBlob dataBlob(buffer_size);
		file_interface->Read(dataBlob.data(), buffer_size, file_handle);
		file_interface->Close(file_handle);

		auto texturePtr = AuroraEngine::AssetManager->LoadTexture(source, GraphicsFormat::RGBA8_UNORM, dataBlob);

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

	bool ShellRenderInterfaceOpenGL::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions)
	{
		TextureDesc textureDesc;
		textureDesc.Width = source_dimensions.x;
		textureDesc.Height = source_dimensions.y;
		textureDesc.ImageFormat = GraphicsFormat::RGBA8_UNORM;
		textureDesc.MipLevels = 1;
		textureDesc.Usage = TextureDesc::EUsage::Default;

		Texture_ptr texturePtr = RD->CreateTexture(textureDesc);
		RD->WriteTexture(texturePtr, 0, 0, source);

		auto* handle = new TexHandle();
		handle->Texture = texturePtr;
		texture_handle = (Rml::TextureHandle) handle;

		return true;
	}

	void ShellRenderInterfaceOpenGL::ReleaseTexture(Rml::TextureHandle texture_handle)
	{
		auto* handle = (TexHandle*)texture_handle;
		delete handle;
	}

	void ShellRenderInterfaceOpenGL::SetTransform(const Rml::Matrix4f* transform)
	{
		m_transform_enabled = (bool)transform;

		if (transform)
		{
			if (std::is_same<Rml::Matrix4f, Rml::ColumnMajorMatrix4f>::value) {
				g_Transform = glm::make_mat4(transform->data());
			}
			else if (std::is_same<Rml::Matrix4f, Rml::RowMajorMatrix4f>::value) {
				g_Transform = glm::make_mat4(transform->Transpose().data());
			}
		}
		else
		{
			g_Transform = glm::identity<Matrix4>();
		}
	}


	ShellRenderInterfaceOpenGL::Image ShellRenderInterfaceOpenGL::CaptureScreen()
	{
		Image image;
		image.num_components = 3;
		image.width = m_width;
		image.height = m_height;

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

	void ShellRenderInterfaceOpenGL::SetViewport(int width, int height)
	{

	}

	void ShellRenderInterfaceOpenGL::PrepareRenderBuffer()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void ShellRenderInterfaceOpenGL::PresentRenderBuffer()
	{
		glDisable(GL_BLEND);
	}

}