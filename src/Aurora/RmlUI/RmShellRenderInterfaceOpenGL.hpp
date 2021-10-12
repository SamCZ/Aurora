#pragma once

#include <map>
#include <RmlUi/Core/RenderInterface.h>
#include "Aurora/Graphics/Base/Texture.hpp"
#include "Aurora/Graphics/Base/ShaderBase.hpp"
#include "Aurora/Graphics/Base/Buffer.hpp"
#include "Aurora/Graphics/Base/InputLayout.hpp"

namespace Aurora
{
	class RmShellRenderInterfaceOpenGL : public Rml::RenderInterface
	{
	protected:
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
	private:
		int m_Width;
		int m_Height;
		bool m_TransformEnabled;
		Matrix4 m_Transform;
		std::map<std::string, Texture_ptr> m_RegisteredCustomTextures;
		Scissors m_Scissors;
		Shader_ptr m_ColorShader;
		Shader_ptr m_TexturedShader;

		InputLayout_ptr m_ColorInputLayout;
		InputLayout_ptr m_TexturedInputLayout;
		Buffer_ptr m_VertexBuffer;
		Buffer_ptr m_IndexBuffer;
		Buffer_ptr m_VertexUniformBuffer;
		Buffer_ptr m_ScissorBuffer;
	public:
		RmShellRenderInterfaceOpenGL();
		~RmShellRenderInterfaceOpenGL() override;

		/// Called by RmlUi when it wants to render geometry that it does not wish to optimise.
		void RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation) override;

		/// Called by RmlUi when it wants to compile geometry it believes will be static for the forseeable future.
		Rml::CompiledGeometryHandle CompileGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture) override;

		/// Called by RmlUi when it wants to render application-compiled geometry.
		void RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation) override;
		/// Called by RmlUi when it wants to release application-compiled geometry.
		void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) override;

		/// Called by RmlUi when it wants to enable or disable scissoring to clip content.
		void EnableScissorRegion(bool enable) override;
		/// Called by RmlUi when it wants to change the scissor region.
		void SetScissorRegion(int x, int y, int width, int height) override;

		/// Called by RmlUi when a texture is required by the library.
		bool LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
		/// Called by RmlUi when a texture is required to be built from an internally-generated sequence of pixels.
		bool GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) override;
		/// Called by RmlUi when a loaded texture is no longer required.
		void ReleaseTexture(Rml::TextureHandle texture_handle) override;

		/// Called by RmlUi when it wants to set the current transform matrix to a new matrix.
		void SetTransform(const Rml::Matrix4f* transform) override;

		// Extensions used by the test suite
		struct Image {
			int width = 0;
			int height = 0;
			int num_components = 0;
			Rml::UniquePtr<Rml::byte[]> data;
		};
		Image CaptureScreen();

		// ShellRenderInterfaceExtensions
		void SetViewport(int width, int height);
		void PrepareRenderBuffer();
		void PresentRenderBuffer();

		bool SetCustomTextureHandleForeName(const std::string& name, const Texture_ptr& texture = nullptr);
	};
}