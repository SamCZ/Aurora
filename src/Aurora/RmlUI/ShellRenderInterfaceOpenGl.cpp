#include "ShellRenderInterfaceOpenGL.hpp"
#include "Aurora/Graphics/OpenGL/GLRenderDevice.hpp"
#include "Aurora/AuroraEngine.hpp"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <type_traits>
#include "Aurora/Graphics/OpenGL/GL.hpp"
#include <iostream>

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

	GLuint VBO = 0;
	Buffer_ptr vertexBuffer;
	Buffer_ptr indexBuffer;
	Buffer_ptr uniformBuffer;
	Shader_ptr shaderColored;
	Shader_ptr shaderTextured;
	InputLayout_ptr inputLayout;
	Matrix4 g_Transform;
	Sampler_ptr g_Sampler;

	ShellRenderInterfaceOpenGL::ShellRenderInterfaceOpenGL() : m_width(0), m_height(0), m_transform_enabled(false)
	{
		glGenBuffers(1, &VBO);

		shaderColored = ASM->LoadShaderFolder("Assets/Shaders/RmlUI/Color");
		shaderTextured = ASM->LoadShaderFolder("Assets/Shaders/RmlUI/Textured");

		vertexBuffer = RD->CreateBuffer(BufferDesc("VB", sizeof(Rml::Vertex) * 3000, sizeof(Rml::Vertex), EBufferType::VertexBuffer, EBufferUsage::DynamicDraw));
		indexBuffer = RD->CreateBuffer(BufferDesc("IB", sizeof(uint32_t) * 3000, sizeof(uint32_t), EBufferType::IndexBuffer, EBufferUsage::DynamicDraw));
		uniformBuffer = RD->CreateBuffer(BufferDesc("UB", sizeof(VertexUniform), 0, EBufferType::UniformBuffer, EBufferUsage::DynamicDraw));
		inputLayout = RD->CreateInputLayout({
			VertexAttributeDesc{"in_Pos", GraphicsFormat::RG32_FLOAT, 0, offsetof(Rml::Vertex, position), false, 0},
			VertexAttributeDesc{"in_Color", GraphicsFormat::R32_UINT, 0, offsetof(Rml::Vertex, colour), false, 1},
			VertexAttributeDesc{"in_TexCoord", GraphicsFormat::RG32_FLOAT, 0, offsetof(Rml::Vertex, tex_coord), false, 2},
		});
		g_Transform = glm::identity<Matrix4>();
		g_Sampler = RD->CreateSampler(SamplerDesc());
	}

// Called by RmlUi when it wants to render geometry that it does not wish to optimise.
	void ShellRenderInterfaceOpenGL::RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, const Rml::TextureHandle texture, const Rml::Vector2f& translation)
	{
		auto* renderDevice = static_cast<GLRenderDevice*>(RD);
		GLContextState& contextState = renderDevice->GetContextState();

		auto screenSize = AuroraEngine::GetCurrentThreadContext()->GetWindow()->GetSize();

		VertexUniform vertexUniform = {};
		vertexUniform.Projection = glm::ortho(0.0f, (float)screenSize.x, (float)screenSize.y, 0.0f, -1.0f, 1.0f);
		vertexUniform.ModelMat = g_Transform * glm::translate(Vector3(translation.x, translation.y, 0));
		renderDevice->WriteBuffer(uniformBuffer, &vertexUniform, sizeof(VertexUniform));


		renderDevice->WriteBuffer(vertexBuffer, vertices, sizeof(Rml::Vertex) * num_vertices);
		renderDevice->WriteBuffer(indexBuffer, indices, sizeof(uint32_t) * num_indices);


		DrawCallState drawCallState;
		drawCallState.Shader = texture ? shaderTextured : shaderColored;
		drawCallState.InputLayoutHandle = inputLayout;
		drawCallState.SetVertexBuffer(0, vertexBuffer);
		drawCallState.SetIndexBuffer(indexBuffer, EIndexBufferFormat::Uint32);
		drawCallState.BoundUniformBuffers["VertexUniform"] = uniformBuffer;

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

// Called by RmlUi when it wants to compile geometry it believes will be static for the forseeable future.
	Rml::CompiledGeometryHandle ShellRenderInterfaceOpenGL::CompileGeometry(Rml::Vertex* RMLUI_UNUSED_PARAMETER(vertices), int RMLUI_UNUSED_PARAMETER(num_vertices), int* RMLUI_UNUSED_PARAMETER(indices), int RMLUI_UNUSED_PARAMETER(num_indices), const Rml::TextureHandle RMLUI_UNUSED_PARAMETER(texture))
	{
		RMLUI_UNUSED(vertices);
		RMLUI_UNUSED(num_vertices);
		RMLUI_UNUSED(indices);
		RMLUI_UNUSED(num_indices);
		RMLUI_UNUSED(texture);

		return (Rml::CompiledGeometryHandle) nullptr;
	}

// Called by RmlUi when it wants to render application-compiled geometry.
	void ShellRenderInterfaceOpenGL::RenderCompiledGeometry(Rml::CompiledGeometryHandle RMLUI_UNUSED_PARAMETER(geometry), const Rml::Vector2f& RMLUI_UNUSED_PARAMETER(translation))
	{
		RMLUI_UNUSED(geometry);
		RMLUI_UNUSED(translation);
	}

// Called by RmlUi when it wants to release application-compiled geometry.
	void ShellRenderInterfaceOpenGL::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle RMLUI_UNUSED_PARAMETER(geometry))
	{
		RMLUI_UNUSED(geometry);
	}

// Called by RmlUi when it wants to enable or disable scissoring to clip content.
	void ShellRenderInterfaceOpenGL::EnableScissorRegion(bool enable)
	{
		/*if (enable) {
			if (!m_transform_enabled) {
				glEnable(GL_SCISSOR_TEST);
				glDisable(GL_STENCIL_TEST);
			} else {
				glDisable(GL_SCISSOR_TEST);
				glEnable(GL_STENCIL_TEST);
			}
		} else {
			glDisable(GL_SCISSOR_TEST);
			glDisable(GL_STENCIL_TEST);
		}*/
	}

// Called by RmlUi when it wants to change the scissor region.
	void ShellRenderInterfaceOpenGL::SetScissorRegion(int x, int y, int width, int height)
	{
		/*AU_LOG_INFO("SetScissorRegion");
		if (!m_transform_enabled) {
			glScissor(x, m_height - (y + height), width, height);
		} else {
			// clear the stencil buffer
			glStencilMask(GLuint(-1));
			glClear(GL_STENCIL_BUFFER_BIT);

			// fill the stencil buffer
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDepthMask(GL_FALSE);
			glStencilFunc(GL_NEVER, 1, GLuint(-1));
			glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

			float fx = (float)x;
			float fy = (float)y;
			float fwidth = (float)width;
			float fheight = (float)height;

			// draw transformed quad
			GLfloat vertices[] = {
					fx, fy, 0,
					fx, fy + fheight, 0,
					fx + fwidth, fy + fheight, 0,
					fx + fwidth, fy, 0
			};
			glDisableClientState(GL_COLOR_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, vertices);
			GLushort indices[] = { 1, 2, 0, 3 };
			glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices);
			glEnableClientState(GL_COLOR_ARRAY);

			// prepare for drawing the real thing
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthMask(GL_TRUE);
			glStencilMask(0);
			glStencilFunc(GL_EQUAL, 1, GLuint(-1));
		}*/
	}

// Set to byte packing, or the compiler will expand our struct, which means it won't read correctly from file
#pragma pack(1)
	struct TGAHeader
	{
		char  idLength;
		char  colourMapType;
		char  dataType;
		short int colourMapOrigin;
		short int colourMapLength;
		char  colourMapDepth;
		short int xOrigin;
		short int yOrigin;
		short int width;
		short int height;
		char  bitsPerPixel;
		char  imageDescriptor;
	};
// Restore packing
#pragma pack()

// Called by RmlUi when a texture is required by the library.
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

		auto texturePtr = AuroraEngine::AssetManager->LoadTexture(source, dataBlob);

		if(!texturePtr) {
			return false;
		}

		texture_dimensions.x = texturePtr->GetDesc().Width;
		texture_dimensions.y = texturePtr->GetDesc().Height;

		auto* handle = new TexHandle();
		handle->Texture = texturePtr;
		texture_handle = (Rml::TextureHandle) handle;

		return true;
		/*
		RMLUI_ASSERTMSG(buffer_size > sizeof(TGAHeader), "Texture file size is smaller than TGAHeader, file must be corrupt or otherwise invalid");
		if(buffer_size <= sizeof(TGAHeader))
		{
			file_interface->Close(file_handle);
			return false;
		}

		char* buffer = new char[buffer_size];
		file_interface->Read(buffer, buffer_size, file_handle);
		file_interface->Close(file_handle);

		TGAHeader header;
		memcpy(&header, buffer, sizeof(TGAHeader));

		int color_mode = header.bitsPerPixel / 8;
		int image_size = header.width * header.height * 4; // We always make 32bit textures

		if (header.dataType != 2)
		{
			Rml::Log::Message(Rml::Log::LT_ERROR, "Only 24/32bit uncompressed TGAs are supported.");
			return false;
		}

		// Ensure we have at least 3 colors
		if (color_mode < 3)
		{
			Rml::Log::Message(Rml::Log::LT_ERROR, "Only 24 and 32bit textures are supported");
			return false;
		}

		const char* image_src = buffer + sizeof(TGAHeader);
		unsigned char* image_dest = new unsigned char[image_size];

		// Targa is BGR, swap to RGB and flip Y axis
		for (long y = 0; y < header.height; y++)
		{
			long read_index = y * header.width * color_mode;
			long write_index = ((header.imageDescriptor & 32) != 0) ? read_index : (header.height - y - 1) * header.width * color_mode;
			for (long x = 0; x < header.width; x++)
			{
				image_dest[write_index] = image_src[read_index+2];
				image_dest[write_index+1] = image_src[read_index+1];
				image_dest[write_index+2] = image_src[read_index];
				if (color_mode == 4)
					image_dest[write_index+3] = image_src[read_index+3];
				else
					image_dest[write_index+3] = 255;

				write_index += 4;
				read_index += color_mode;
			}
		}

		texture_dimensions.x = header.width;
		texture_dimensions.y = header.height;

		bool success = GenerateTexture(texture_handle, image_dest, texture_dimensions);

		delete [] image_dest;
		delete [] buffer;

		return success;*/
	}

// Called by RmlUi when a texture is required to be built from an internally-generated sequence of pixels.
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

// Called by RmlUi when a loaded texture is no longer required.
	void ShellRenderInterfaceOpenGL::ReleaseTexture(Rml::TextureHandle texture_handle)
	{
		auto* handle = (TexHandle*)texture_handle;
		delete handle;
	}

// Called by RmlUi when it wants to set the current transform matrix to a new matrix.
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