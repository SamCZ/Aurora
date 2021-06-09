#include "GLRenderDevice.hpp"

#include "GLConversions.hpp"
#include "GLFormatMapping.hpp"
#include "GLShaderProgram.hpp"
#include "GLTexture.hpp"

namespace Aurora
{
	GLRenderDevice::GLRenderDevice() : IRenderDevice(), m_PipelineState()
	{

	}

	void GLRenderDevice::Init()
	{

	}

	Shader_ptr GLRenderDevice::CreateShaderProgram(const ShaderProgramDesc &desc)
	{
		const auto& shaderDescriptions = desc.GetShaderDescriptions();

		if(shaderDescriptions.empty()) {
			AU_LOG_ERROR("No shaders provided !");
			return nullptr;
		}

		if(shaderDescriptions.contains(ShaderType::Compute) && shaderDescriptions.size() > 1) {
			AU_LOG_ERROR("You cannot mix compute shader with basic types of shaders in program ", desc.GetName(), " !");
			return nullptr;
		}

		if(!shaderDescriptions.contains(ShaderType::Vertex) || !shaderDescriptions.contains(ShaderType::Pixel)) {
			AU_LOG_ERROR("You must always provide Vertex and Pixel shader !");
			return nullptr;
		}

		// Compile shaders

		std::vector<GLuint> compiledShaders;
		for(const auto& it : shaderDescriptions) {
			const auto& shaderDesc = it.second;
			const auto& type = shaderDesc.Type;

			std::string source;

			source += "#version 430 core\n";

			if(type == ShaderType::Vertex) {
				//source += "#extension GL_KHR_vulkan_glsl : enable\n";
				//source += "#define gl_VertexID gl_VertexIndex\n";
				//source += "#define gl_InstanceID gl_InstanceIndex\n";
			}

			source += shaderDesc.Source;

			std::string error;
			GLuint shaderID = CompileShaderRaw(source, type, &error);

			if(shaderID == 0) {
				AU_LOG_ERROR("Cannot compile shader ", ShaderType_ToString(type), " in program ", desc.GetName(), "!\n", error);
				return nullptr;
			}

			compiledShaders.push_back(shaderID);
		}

		GLuint programID = glCreateProgram();

		// Attach shaders

		for(auto shaderID : compiledShaders) {
			glAttachShader(programID, shaderID);
		}

		// Link program

		glLinkProgram(programID);
		GLint linkStatus = 0;
		glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus);

		if(linkStatus == GL_FALSE) {
			GLchar error[1024] = { 0 };
			GLsizei logLength = 0;
			glGetProgramInfoLog(programID, sizeof(error), &logLength, error);

			if(desc.HasSetErrorOutput()) {
				desc.GetErrorOutput()(error);
			} else {
				AU_LOG_ERROR("Cannot link shader program ", desc.GetName(), ": ", error);
			}

			return nullptr;
		}

		// Validate program

		glValidateProgram(programID);
		GLint validateStatus = 0;
		glGetProgramiv(programID, GL_VALIDATE_STATUS, &validateStatus);

		if(validateStatus == GL_FALSE) {
			GLchar error[1024] = { 0 };
			GLsizei logLength = 0;
			glGetProgramInfoLog(programID, sizeof(error), &logLength, error);

			if(desc.HasSetErrorOutput()) {
				desc.GetErrorOutput()(error);
			} else {
				AU_LOG_ERROR("Cannot validate shader program ", desc.GetName(), ": ", error);
			}

			return nullptr;
		}

		auto shaderProgram = std::make_shared<GLShaderProgram>(programID, desc);

		// Cleanup before returning shader program object
		for(auto shaderID : compiledShaders) {
			glDetachShader(programID, shaderID);
			glDeleteShader(shaderID);
		}

		return shaderProgram;
	}

	GLuint GLRenderDevice::CompileShaderRaw(const std::string &sourceString, const ShaderType &shaderType, std::string *errorOutput)
	{
		GLenum type;
		switch (shaderType)
		{
			case ShaderType::Vertex:     type = GL_VERTEX_SHADER; break;
			case ShaderType::Hull:       type = GL_TESS_CONTROL_SHADER; break;
			case ShaderType::Domain:     type = GL_TESS_EVALUATION_SHADER; break;
			case ShaderType::Geometry:   type = GL_GEOMETRY_SHADER; break;
			case ShaderType::Pixel:      type = GL_FRAGMENT_SHADER; break;
			case ShaderType::Compute:    type = GL_COMPUTE_SHADER; break;
			default:
				AU_LOG_ERROR("Unrecognized shader type ", ShaderType_ToString(shaderType));
				return 0;
		}

		GLuint shaderID = glCreateShader(type);

		const GLchar* source = sourceString.data();
		auto length = static_cast<GLint>(sourceString.length());
		glShaderSource(shaderID, 1, &source, &length);
		glCompileShader(shaderID);

		GLint compileStatus = 0;
		GLchar error[1024] = { 0 };
		GLsizei logLength = 0;

		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileStatus);
		if(compileStatus == GL_FALSE) {
			glGetShaderInfoLog(shaderID, sizeof(error), &logLength, error);
			if(errorOutput) {
				*errorOutput = error;
			} else {
				AU_LOG_ERROR("Shader compile error!\n", error);
			}
			return 0;
		}

		return shaderID;
	}

	void GLRenderDevice::ApplyShader(const Shader_ptr &shader)
	{
		if(shader == nullptr && m_PipelineState.LastShaderHandle != 0) {
			m_PipelineState.LastShaderHandle = 0;
			glUseProgram(0);
		}

		auto handle = static_cast<GLShaderProgram*>(shader.get())->Handle(); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		if(handle != m_PipelineState.LastShaderHandle) {
			m_PipelineState.LastShaderHandle = handle;
			glUseProgram(handle);
		}
	}

	Texture_ptr GLRenderDevice::CreateTexture(const TextureDesc &desc, TextureData textureData)
	{
		const FormatMapping& formatMapping = GetFormatMapping(desc.ImageFormat);

		GLuint handle = 0;
		glGenTextures(1, &handle);

		GLenum bindTarget = GL_NONE;
		uint32_t numLayers = 1;

		if (desc.IsCubeMap)
		{
			bindTarget = GL_TEXTURE_CUBE_MAP;
			glBindTexture(bindTarget, handle);

			glTexStorage2D(
					bindTarget,
					(GLsizei)desc.MipLevels,
					formatMapping.InternalFormat,
					(GLsizei)desc.Width,
					(GLsizei)desc.Height);

			numLayers = 6;

			CHECK_GL_ERROR();
		}
		else if (desc.IsArray)
		{
			bindTarget = GL_TEXTURE_2D_ARRAY;
			glBindTexture(bindTarget, handle);

			glTexStorage3D(bindTarget,
						   (GLsizei)desc.MipLevels,
						   formatMapping.InternalFormat,
						   (GLsizei)desc.Width,
						   (GLsizei)desc.Height,
						   (GLsizei)desc.DepthOrArraySize);

			numLayers = desc.DepthOrArraySize;

			CHECK_GL_ERROR();
		}
		else if (desc.DepthOrArraySize > 0)
		{
			bindTarget = GL_TEXTURE_3D;
			glBindTexture(bindTarget, handle);

			glTexStorage3D(bindTarget,
						   (GLsizei)desc.MipLevels,
						   formatMapping.InternalFormat,
						   (GLsizei)desc.Width,
						   (GLsizei)desc.Height,
						   (GLsizei) desc.DepthOrArraySize);

			CHECK_GL_ERROR();
		}
		else if (desc.SampleCount > 1)
		{
			//bindTarget = GL_TEXTURE_2D_MULTISAMPLE;
			glBindTexture(bindTarget, handle);

			/*glTexStorage2DMultisample(bindTarget,
									  (GLsizei)desc.SampleCount,
									  formatMapping.InternalFormat,
									  (GLsizei)desc.Width,
									  (GLsizei)desc.Height,
									  GL_FALSE);*/

			CHECK_GL_ERROR();
		}
		else
		{
			bindTarget = GL_TEXTURE_2D;
			glBindTexture(bindTarget, handle);

			glTexStorage2D(
					bindTarget,
					(GLsizei)desc.MipLevels,
					formatMapping.InternalFormat,
					(GLsizei)desc.Width,
					(GLsizei)desc.Height);

			CHECK_GL_ERROR();
		}

		if (desc.SampleCount == 1)
		{
			glTexParameteri(bindTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(bindTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			CHECK_GL_ERROR();
		}

		glBindTexture(bindTarget, 0);

		GLuint srgbView = GL_NONE;

		if (formatMapping.AbstractFormat == GraphicsFormat::SRGBA8_UNORM)
		{
			glGenTextures(1, &srgbView);

			/*glTextureView(
					srgbView,
					bindTarget,
					handle,
					GL_SRGB8_ALPHA8,
					0,
					desc.MipLevels,
					0,
					numLayers);*/

			CHECK_GL_ERROR();

			if (desc.SampleCount == 1)
			{
				glBindTexture(bindTarget, srgbView);
				glTexParameteri(bindTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(bindTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glBindTexture(bindTarget, 0);
				CHECK_GL_ERROR();
			}
		}

		// TODO: Write @textureData

		return std::make_shared<GLTexture>(desc, formatMapping, handle, srgbView, bindTarget);
	}

	void GLRenderDevice::WriteTexture(const Texture_ptr &texture, uint32_t subresource, const void *data)
	{
		auto* glTexture = static_cast<GLTexture*>(texture.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		glBindTexture(glTexture->BindTarget(), glTexture->Handle());

		const auto& desc = texture->GetDesc();

		if (desc.IsArray || desc.DepthOrArraySize > 0)
		{
			glTexSubImage3D(glTexture->BindTarget(), 0/*level*/, 0, 0, (GLint)subresource, (GLsizei)desc.Width, (GLsizei)desc.Height, 1/*depth*/, glTexture->Format().BaseFormat, glTexture->Format().Type, data);
			CHECK_GL_ERROR();
			if(true) {
				glGenerateMipmap( GL_TEXTURE_2D_ARRAY );
			}
		}
		else
		{
			uint32_t width = std::max<uint32_t>(1, desc.Width >> subresource);
			uint32_t height = std::max<uint32_t>(1, desc.Height >> subresource);
			glTexSubImage2D(glTexture->BindTarget(), (GLint)subresource, 0, 0, (GLsizei)width, (GLsizei)height, glTexture->Format().BaseFormat, glTexture->Format().Type, data);
			CHECK_GL_ERROR();
		}

		glBindTexture(glTexture->BindTarget(), 0);
	}

	void GLRenderDevice::ClearTextureFloat(const Texture_ptr &texture, const Color &clearColor)
	{
		auto* glTexture = static_cast<GLTexture*>(texture.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		for (GLint nMipLevel = 0; nMipLevel < glTexture->GetDesc().MipLevels; ++nMipLevel)
		{
			//glClearTexImage(glTexture->Handle(), nMipLevel, glTexture->Format().BaseFormat, GL_FLOAT, &clearColor);
			CHECK_GL_ERROR();
		}
	}

	void GLRenderDevice::ClearTextureUInt(const Texture_ptr &texture, uint32_t clearColor)
	{
		auto* glTexture = static_cast<GLTexture*>(texture.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		uint32_t colors[4] = { clearColor, clearColor, clearColor, clearColor };

		for (GLint nMipLevel = 0; nMipLevel < glTexture->GetDesc().MipLevels; ++nMipLevel)
		{
			//glClearTexImage(glTexture->Handle(), nMipLevel, glTexture->Format().BaseFormat, GL_UNSIGNED_INT, colors);
			CHECK_GL_ERROR();
		}
	}

	Buffer_ptr GLRenderDevice::CreateBuffer(const BufferDesc &desc, const void *data)
	{
#if PLATFORM_ANDROID
		const uint8_t GL_SHADER_STORAGE_BUFFER = 1;
		const uint8_t GL_TEXTURE_BUFFER = 4;
#endif
		GLenum bindTarget = desc.CanHaveUAVs || desc.StructStride ? GL_SHADER_STORAGE_BUFFER : GL_TEXTURE_BUFFER;
		GLuint bufferHandle = 0;
		GLuint ssboHandle = 0;

		GLenum usage = desc.CanHaveUAVs ? GL_STREAM_COPY : GL_STREAM_DRAW;

		glGenBuffers(1, &bufferHandle);
		glBindBuffer(bindTarget, bufferHandle);

		glBufferData(bindTarget, desc.ByteSize, data, usage);
		CHECK_GL_ERROR();

		glBindBuffer(bindTarget, GL_NONE);


		/*glGenTextures(1, &ssboHandle);
		glBindTexture(GL_TEXTURE_BUFFER, ssboHandle);

		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, bufferHandle);
		CHECK_GL_ERROR();

		glBindTexture(GL_TEXTURE_BUFFER, GL_NONE);*/

		return nullptr;
	}



}