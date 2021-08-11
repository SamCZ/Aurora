#include <Aurora/Core/Crc.hpp>
#include "GLRenderDevice.hpp"

#include "GLConversions.hpp"
#include "GLFormatMapping.hpp"
#include "GLShaderProgram.hpp"
#include "GLTexture.hpp"
#include "GLBuffer.hpp"
#include "GLSampler.hpp"

namespace Aurora
{
	GLBuffer* GetBuffer(const Buffer_ptr& buffer)
	{
		return static_cast<GLBuffer*>(buffer.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
	}

	GLTexture* GetTexture(const Texture_ptr& texture)
	{
		return static_cast<GLTexture*>(texture.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
	}

	GLSampler* GetSampler(const Sampler_ptr& sampler)
	{
		return static_cast<GLSampler*>(sampler.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
	}

	GLShaderProgram* GetShader(const Shader_ptr& shader)
	{
		return static_cast<GLShaderProgram*>(shader.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
	}

	GLRenderDevice::GLRenderDevice() : IRenderDevice(), m_nVAO(0), m_nVAOEmpty(0), m_LastVao(0), m_ContextState()
	{

	}

	GLRenderDevice::~GLRenderDevice()
	{
		glDeleteVertexArrays(1, &m_nVAO);
		glDeleteVertexArrays(1, &m_nVAOEmpty);
	}

	void GLRenderDevice::Init()
	{
		glGenVertexArrays(1, &m_nVAO);
		glGenVertexArrays(1, &m_nVAOEmpty);
		glBindVertexArray(m_nVAO);


		glDepthRangef(0.0f, 1.0f);
		// Enable depth remapping to [0, 1] interval
		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
        const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model

        AU_LOG_INFO("GPU Vendor: ", vendor);
        AU_LOG_INFO("GPU Renderer: ", renderer);
	}

	Shader_ptr GLRenderDevice::CreateShaderProgram(const ShaderProgramDesc &desc)
	{
		const auto& shaderDescriptions = desc.GetShaderDescriptions();

		if(shaderDescriptions.empty()) {
			AU_LOG_ERROR("No shaders provided !");
			return nullptr;
		}

		if(shaderDescriptions.contains(EShaderType::Compute) && shaderDescriptions.size() > 1) {
			AU_LOG_ERROR("You cannot mix compute shader with basic types of shaders in program ", desc.GetName(), " !");
			return nullptr;
		}

		/*if(!shaderDescriptions.contains(EShaderType::Vertex) || !shaderDescriptions.contains(EShaderType::Pixel) || !shaderDescriptions.contains(EShaderType::Compute)) {
			AU_LOG_ERROR("You must always provide Vertex and Pixel or Compute shader !");
			return nullptr;
		}*/

		// Compile shaders

		std::vector<GLuint> compiledShaders;
		for(const auto& it : shaderDescriptions) {
			const auto& shaderDesc = it.second;
			const auto& type = shaderDesc.Type;

			std::string source;

			source += "#version 430 core\n";
			source += "layout(std140) uniform;\n";

			if(type == EShaderType::Vertex) {
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

	GLuint GLRenderDevice::CompileShaderRaw(const std::string &sourceString, const EShaderType &shaderType, std::string *errorOutput)
	{
		GLenum type;
		switch (shaderType)
		{
			case EShaderType::Vertex: type = GL_VERTEX_SHADER; break;
			case EShaderType::Hull: type = GL_TESS_CONTROL_SHADER; break;
			case EShaderType::Domain: type = GL_TESS_EVALUATION_SHADER; break;
			case EShaderType::Geometry: type = GL_GEOMETRY_SHADER; break;
			case EShaderType::Pixel: type = GL_FRAGMENT_SHADER; break;
			case EShaderType::Compute: type = GL_COMPUTE_SHADER; break;
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

	void GLRenderDevice::SetShader(const Shader_ptr &shader)
	{
		m_ContextState.SetShader(GetShader(shader));
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

			glTextureView(
					srgbView,
					bindTarget,
					handle,
					GL_SRGB8_ALPHA8,
					0,
					desc.MipLevels,
					0,
					numLayers);

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

	void GLRenderDevice::WriteTexture(const Texture_ptr &texture, uint32_t mipLevel, uint32_t subresource, const void *data)
	{
		auto* glTexture = static_cast<GLTexture*>(texture.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		glBindTexture(glTexture->BindTarget(), glTexture->Handle());

		const auto& desc = texture->GetDesc();

		uint32_t width = std::max<uint32_t>(1, desc.Width >> mipLevel);
		uint32_t height = std::max<uint32_t>(1, desc.Height >> mipLevel);

		if(desc.IsCubeMap)
		{
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + subresource, (GLint)mipLevel, 0, 0, (GLsizei)width, (GLsizei)height, glTexture->Format().BaseFormat, glTexture->Format().Type, data);
			CHECK_GL_ERROR();
		} else if (desc.IsArray || desc.DepthOrArraySize > 0)
		{
			glTexSubImage3D(glTexture->BindTarget(), GLint(mipLevel), 0, 0, (GLint)subresource, (GLsizei)width, (GLsizei)height, 1/*depth*/, glTexture->Format().BaseFormat, glTexture->Format().Type, data);
			CHECK_GL_ERROR();
			if(true) {
				glGenerateMipmap( GL_TEXTURE_2D_ARRAY );
			}
		}
		else
		{
			glTexSubImage2D(glTexture->BindTarget(), (GLint)mipLevel, 0, 0, (GLsizei)width, (GLsizei)height, glTexture->Format().BaseFormat, glTexture->Format().Type, data);
			CHECK_GL_ERROR();
		}

		glBindTexture(glTexture->BindTarget(), 0);
	}

	void GLRenderDevice::ClearTextureFloat(const Texture_ptr &texture, const Color &clearColor)
	{
		auto* glTexture = static_cast<GLTexture*>(texture.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		for (GLint nMipLevel = 0; nMipLevel < glTexture->GetDesc().MipLevels; ++nMipLevel)
		{
			glClearTexImage(glTexture->Handle(), nMipLevel, glTexture->Format().BaseFormat, GL_FLOAT, &clearColor);
			CHECK_GL_ERROR();
		}
	}

	void GLRenderDevice::ClearTextureUInt(const Texture_ptr &texture, uint32_t clearColor)
	{
		auto* glTexture = static_cast<GLTexture*>(texture.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		uint32_t colors[4] = { clearColor, clearColor, clearColor, clearColor };

		for (GLint nMipLevel = 0; nMipLevel < glTexture->GetDesc().MipLevels; ++nMipLevel)
		{
			glClearTexImage(glTexture->Handle(), nMipLevel, glTexture->Format().BaseFormat, GL_UNSIGNED_INT, colors);
			CHECK_GL_ERROR();
		}
	}

	void GLRenderDevice::GenerateMipmaps(const Texture_ptr &texture)
	{
		auto* glTexture = static_cast<GLTexture*>(texture.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		glBindTexture(glTexture->BindTarget(), glTexture->Handle());
		glGenerateMipmap(glTexture->BindTarget());
		glBindTexture(glTexture->BindTarget(), 0);
	}

	Buffer_ptr GLRenderDevice::CreateBuffer(const BufferDesc &desc, const void *data)
	{
		GLuint handle = 0;
		GLenum bindTarget = ConvertBufferType(desc.Type);
		GLenum usage = ConvertUsage(desc.Usage);

		if(desc.Type != EBufferType::TextureBuffer) {
			glGenBuffers(1, &handle);
			glBindBuffer(bindTarget, handle);

			glBufferData(bindTarget, desc.ByteSize, data, usage);
			CHECK_GL_ERROR();

			glBindBuffer(bindTarget, GL_NONE);
		} else {
			glGenTextures(1, &handle);
			glBindTexture(GL_TEXTURE_BUFFER, handle);

			glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, handle);
			CHECK_GL_ERROR();

			glBindTexture(GL_TEXTURE_BUFFER, GL_NONE);
		}

		return std::make_shared<GLBuffer>(desc, handle, bindTarget, usage);
	}

	void GLRenderDevice::WriteBuffer(const Buffer_ptr &buffer, const void *data, size_t dataSize)
	{
		if(buffer == nullptr) {
			return;
		}

		auto* glBuffer = static_cast<GLBuffer*>(buffer.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		glBindBuffer(glBuffer->BindTarget(), glBuffer->Handle());

		assert(glBuffer->GetDesc().ByteSize >= dataSize);

		if (dataSize > glBuffer->GetDesc().ByteSize)
			dataSize = glBuffer->GetDesc().ByteSize;

		glBufferSubData(glBuffer->BindTarget(), 0, GLsizeiptr(dataSize), data);
		CHECK_GL_ERROR();

		glBindBuffer(glBuffer->BindTarget(), GL_NONE);
	}

	void GLRenderDevice::ClearBufferUInt(const Buffer_ptr &buffer, uint32_t clearValue)
	{
		if(buffer == nullptr) {
			return;
		}

		auto* glBuffer = static_cast<GLBuffer*>(buffer.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		glBindBuffer(glBuffer->BindTarget(), glBuffer->Handle());

		glClearBufferData(glBuffer->BindTarget(), GL_R32UI, GL_RED, GL_UNSIGNED_INT, &clearValue);
		CHECK_GL_ERROR();

		glBindBuffer(glBuffer->BindTarget(), GL_NONE);
	}

	void GLRenderDevice::CopyToBuffer(const Buffer_ptr &dest, uint32_t destOffsetBytes, const Buffer_ptr &src, uint32_t srcOffsetBytes, size_t dataSizeBytes)
	{
		if(dest == nullptr) {
			return;
		}

		auto* glBuffer = static_cast<GLBuffer*>(dest.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		glBindBuffer(GL_COPY_WRITE_BUFFER, glBuffer->Handle());
		glBindBuffer(GL_COPY_READ_BUFFER, glBuffer->Handle());

		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffsetBytes, destOffsetBytes, GLsizeiptr(dataSizeBytes));

		CHECK_GL_ERROR();

		glBindBuffer(GL_COPY_READ_BUFFER, GL_NONE);
		glBindBuffer(GL_COPY_WRITE_BUFFER, GL_NONE);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void GLRenderDevice::ReadBuffer(const Buffer_ptr &buffer, void *data, size_t *dataSize)
	{
		if(buffer == nullptr) {
			return;
		}

		auto* glBuffer = static_cast<GLBuffer*>(buffer.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		auto nBytesToRead = uint32_t(*dataSize);
		if (nBytesToRead > glBuffer->GetDesc().ByteSize)
		{
			nBytesToRead = glBuffer->GetDesc().ByteSize;
		}

		/*glBindBuffer(GL_SHADER_STORAGE_BUFFER, glBuffer->Handle());

		void* pMappedData = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, nBytesToRead, GL_MAP_READ_BIT);
		if (pMappedData)
		{
			memcpy(data, pMappedData, nBytesToRead);
			*dataSize = nBytesToRead;
		}
		else
		{
			*dataSize = 0;
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, GL_NONE);*/

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, glBuffer->Handle());
		GLvoid* pMappedData = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		memcpy(data, pMappedData, nBytesToRead);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}

	Sampler_ptr GLRenderDevice::CreateSampler(const SamplerDesc &desc)
	{
		GLuint handle = 0;
		glGenSamplers(1, &handle);

		glSamplerParameteri(handle, GL_TEXTURE_WRAP_S, ConvertWrapMode(desc.WrapMode[0]));
		glSamplerParameteri(handle, GL_TEXTURE_WRAP_T, ConvertWrapMode(desc.WrapMode[1]));
		glSamplerParameteri(handle, GL_TEXTURE_WRAP_R, ConvertWrapMode(desc.WrapMode[2]));
		glSamplerParameterfv(handle, GL_TEXTURE_BORDER_COLOR, (const float*)&desc.BorderColor);
		glSamplerParameterf(handle, GL_TEXTURE_LOD_BIAS, desc.MipBias);

		if (desc.Anisotropy > 1.f)
		{
			glSamplerParameterf(handle, GL_TEXTURE_MAX_ANISOTROPY_EXT, desc.Anisotropy);
		}

		if (desc.ShadowCompare)
		{
			glSamplerParameteri(handle, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
			glSamplerParameteri(handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glSamplerParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glSamplerParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glSamplerParameteri(handle, GL_TEXTURE_MIN_FILTER, desc.MipFilter ? GL_LINEAR_MIPMAP_LINEAR : (desc.MinFilter ? GL_LINEAR : GL_NEAREST));
			glSamplerParameteri(handle, GL_TEXTURE_MAG_FILTER, desc.MagFilter ? GL_LINEAR : GL_NEAREST);
		}

		return std::make_shared<GLSampler>(desc, handle);
	}

	InputLayout_ptr GLRenderDevice::CreateInputLayout(const std::vector<VertexAttributeDesc> &desc)
	{
		return std::make_shared<BasicInputLayout>(desc);
	}

	void GLRenderDevice::Draw(const DrawCallState &state, const std::vector<DrawArguments>& args)
	{
		if(state.Shader == nullptr) {
			AU_LOG_ERROR("Cannot draw without shader !");
			throw;
			return;
		}

		ApplyDrawCallState(state);

		GLenum primitiveType = ConvertPrimType(state.PrimitiveType);

		for(const auto& drawArg : args) {
			glDrawArraysInstanced(primitiveType, GLint(drawArg.StartVertexLocation), GLint(drawArg.VertexCount), GLint(drawArg.InstanceCount));
			CHECK_GL_ERROR();
		}
	}

	void GLRenderDevice::DrawIndexed(const DrawCallState &state, const std::vector<DrawArguments> &args)
	{
		if(state.IndexBuffer.Buffer == nullptr || state.Shader == nullptr) {
			AU_LOG_ERROR("Cannot draw with these arguments !");
			throw;
			return;
		}

		ApplyDrawCallState(state);

		CHECK_GL_ERROR();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GetBuffer(state.IndexBuffer.Buffer)->Handle());

		GLenum primitiveType = ConvertPrimType(state.PrimitiveType);
		GLenum ibFormat = ConvertIndexBufferFormat(state.IndexBuffer.Format);

		for(const auto& drawArg : args) {
			uint32_t indexOffset = drawArg.StartIndexLocation * 4 + state.IndexBufferOffset;

			if(drawArg.InstanceCount > 0) {
				glDrawElementsInstancedBaseVertex(primitiveType, GLsizei(drawArg.VertexCount), ibFormat, (const void*)size_t(indexOffset), GLsizei(drawArg.InstanceCount), GLint(drawArg.StartVertexLocation));
			} else {
				glDrawElementsBaseVertex(primitiveType, GLsizei(drawArg.VertexCount), ibFormat, (const void*)size_t(indexOffset), GLint(drawArg.StartVertexLocation));
			}
		}

		/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);
		glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);*/
	}

	void GLRenderDevice::DrawIndirect(const DrawCallState &state, const Buffer_ptr &indirectParams, uint32_t offsetBytes)
	{
		// TODO: Indirect render
	}

	void GLRenderDevice::ApplyDrawCallState(const DrawCallState &state)
	{
		SetShader(state.Shader);

		BindShaderInputs(state);
		BindShaderResources(state);

		BindRenderTargets(state);

		SetBlendState(state);
		SetRasterState(state.RasterState);

		SetDepthStencilState(state.DepthStencilState);

		ClearRenderTargets(state);
	}

	void GLRenderDevice::BindShaderInputs(const DrawCallState &state)
	{
		auto glShader = GetShader(state.Shader);
		const auto& inputVars = glShader->GetInputVariables();

		if(inputVars.empty() || state.InputLayoutHandle == nullptr) {
			if(m_LastVao != m_nVAOEmpty) {
				m_LastVao = m_nVAOEmpty;
				glBindVertexArray(m_nVAOEmpty);
			}
			return;
		}

		if(m_LastVao != m_nVAO) {
			m_LastVao = m_nVAO;
			glBindVertexArray(m_nVAO);
		}

		for(const auto& var : inputVars) {
			uint8_t location = var.first;
			const ShaderInputVariable& inputVariable = var.second;
			VertexAttributeDesc layoutAttribute;

			if(!state.InputLayoutHandle->GetDescriptorBySemanticID(location, layoutAttribute)) {
				AU_LOG_FATAL("Input layout from DrawState is not supported by that in the shader !");
				continue;
			}

			if(inputVariable.Format != layoutAttribute.Format) {
				AU_LOG_FATAL("Input descriptor format is not the same !");
				throw;
			};

			const FormatMapping& formatMapping = GetFormatMapping(layoutAttribute.Format);

			auto vertexBufferIt = state.VertexBuffers.find(layoutAttribute.BufferIndex);
			auto glBuffer = GetBuffer(vertexBufferIt->second);
			glBindBuffer(GL_ARRAY_BUFFER, glBuffer->Handle());

			glEnableVertexAttribArray(GLuint(location));

			if(formatMapping.Type == GL_INT || formatMapping.Type == GL_UNSIGNED_INT) {
				glVertexAttribIPointer(
						GLuint(location),
						GLint(formatMapping.Components),
						formatMapping.Type,
						GLsizei(glBuffer->GetDesc().Stride),
						(const void*)size_t(layoutAttribute.Offset));
			} else {
				glVertexAttribPointer(
						GLuint(location), // location
						GLint(formatMapping.Components),
						formatMapping.Type,
						GL_FALSE,// Is normalized
						GLsizei(glBuffer->GetDesc().Stride),
						(const void*)size_t(layoutAttribute.Offset));
			}

			glVertexAttribDivisor(GLuint(location), layoutAttribute.IsInstanced ? 1 : 0);
		}

		//glBindBuffer(GL_ARRAY_BUFFER, GL_NONE); Do not do this, android render will fail !

		static GLint nMaxVertexAttrs = 0;
		if(nMaxVertexAttrs == 0)
			glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nMaxVertexAttrs);

		for (auto semanticIndex = 0; semanticIndex < nMaxVertexAttrs; semanticIndex++)
		{
			bool foundSemantic = false;
			for(const auto& var : inputVars)
			{
				if(semanticIndex == var.first) {
					foundSemantic = true;
					break;
				}
			}
			if(!foundSemantic) {
				glDisableVertexAttribArray(GLuint(semanticIndex));
			}
		}
	}

	void GLRenderDevice::Dispatch(const DispatchState &state, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ)
	{
		ApplyDispatchState(state);

		glDispatchCompute(GLuint(groupsX), GLuint(groupsY), GLuint(groupsZ));

		//GL_SHADER_IMAGE_ACCESS_BARRIER_BIT, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT, GL_BUFFER_UPDATE_BARRIER_BIT
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
	}

	void GLRenderDevice::DispatchIndirect(const DispatchState &state, const Buffer_ptr &indirectParams, uint32_t offsetBytes)
	{
		assert(state.Shader != nullptr);

		ApplyDispatchState(state);

		glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, GetBuffer(indirectParams)->Handle());
		glDispatchComputeIndirect(offsetBytes);
		CHECK_GL_ERROR();
		glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, GL_NONE);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
	}

	void GLRenderDevice::InvalidateState()
	{
		m_ContextState.Invalidate();
	}

	void GLRenderDevice::ApplyDispatchState(const DispatchState &state)
	{
		SetShader(state.Shader);
		BindShaderResources(state);
	}

	void GLRenderDevice::BindShaderResources(const BaseState& state)
	{
		CHECK_GL_ERROR();

		if(state.Shader == nullptr) return;

		auto shader = static_cast<GLShaderProgram*>(state.Shader.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		for(const auto& imageResource : shader->GetGLResource().GetSamplers()) {
			auto boundTextureIt = state.BoundTextures.find(imageResource.Name);

			const TextureBinding* targetTextureBinding = nullptr;

			if(boundTextureIt == state.BoundTextures.end()) {
				m_ContextState.BindTexture(imageResource.Binding, nullptr);
				continue;
			}

			targetTextureBinding = &boundTextureIt->second;

			if(targetTextureBinding == nullptr) {
				m_ContextState.BindTexture(imageResource.Binding, nullptr);
				// TODO: Set placeholder texture or throw error
				continue;
			}

			auto* glTexture = GetTexture(targetTextureBinding->Texture);

			if(glTexture == nullptr) {
				m_ContextState.BindTexture(imageResource.Binding, nullptr);
				// TODO: Set placeholder texture or throw error
				continue;
			}

			const TextureDesc& textureDesc = glTexture->GetDesc();

			m_ContextState.BindTexture(imageResource.Binding, glTexture);
		}

		for(const auto& imageResource : shader->GetGLResource().GetImages()) {
			auto boundTextureIt = state.BoundTextures.find(imageResource.Name);

			const TextureBinding* targetTextureBinding = nullptr;

			if(boundTextureIt == state.BoundTextures.end()) {
				m_ContextState.BindImage(imageResource.Binding, nullptr, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB16F);
				continue;
			}

			targetTextureBinding = &boundTextureIt->second;

			if(targetTextureBinding == nullptr) {
				// TODO: Set placeholder texture or throw error
				continue;
			}

			auto* glTexture = GetTexture(targetTextureBinding->Texture);

			const TextureDesc& textureDesc = glTexture->GetDesc();

			if(textureDesc.IsUAV && targetTextureBinding->IsUAV) {
				GLenum format = glTexture->Format().InternalFormat;
				GLenum access = GL_WRITE_ONLY;
				bool layered = false;
				GLint level = 0;
				GLint layer = 0;

				switch (targetTextureBinding->Access) {
					case TextureBinding::EAccess::Read: access = GL_READ_ONLY; break;
					case TextureBinding::EAccess::Write: access = GL_WRITE_ONLY; break;
					case TextureBinding::EAccess::ReadAndWrite: access = GL_READ_WRITE; break;
				}

				m_ContextState.BindImage(imageResource.Binding, glTexture, level, layered, layer, access, format);
			} else {
				AU_LOG_WARNING("Trying to bind image as UAV but somewhere the texture is not marked as UAV");
			}
		}

		// binding samplers
		// TODO: Samplers
		for(const auto& samplerResource : shader->GetGLResource().GetSamplers()) {
			auto boundSamplerIt = state.BoundSamplers.find(samplerResource.Name);

			Sampler_ptr targetSampler = nullptr;

			if(boundSamplerIt != state.BoundSamplers.end()) {
				targetSampler = boundSamplerIt->second;
			}

			m_ContextState.BindSampler(samplerResource.Binding, GetSampler(targetSampler));
		}

		// binding constant buffers
		for(const auto& uniformResource : shader->GetGLResource().GetUniformBlocks()) {
			auto uniformBufferIt = state.BoundUniformBuffers.find(uniformResource.Name);

			Buffer_ptr uniformBufferHandle = nullptr;

			if(uniformBufferIt != state.BoundUniformBuffers.end()) {
				uniformBufferHandle = uniformBufferIt->second;
			}

			m_ContextState.BindUniformBuffer(uniformResource.Binding, GetBuffer(uniformBufferHandle));
		}


		// binding ssbo`s
		// TODO: Complete SSBO
		for(const auto& uniformResource : shader->GetGLResource().GetStorageBlocks())
		{
			auto ssboIt = state.SSBOBuffers.find(uniformResource.Name);

			Buffer_ptr ssboBufferHandle = nullptr;

			if(ssboIt != state.SSBOBuffers.end())
			{
				ssboBufferHandle = ssboIt->second;
			}

			m_ContextState.BindStorageBlock(uniformResource.Binding, GetBuffer(ssboBufferHandle));
		}

		/*
		for (uint32_t nBuffer = 0; nBuffer < state.bufferBindingCount; ++nBuffer)
		{
			const BufferBinding& binding = state.buffers[nBuffer];

			if (binding.isWritable || binding.buffer->desc.StructStride > 0)
			{
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding.slot, binding.buffer->bufferHandle);
				m_vecBoundBuffers.push_back(binding.slot);
			}
			else
			{
				glActiveTexture(GL_TEXTURE0 + binding.slot);
				glBindTexture(GL_TEXTURE_BUFFER, binding.buffer->ssboHandle);
				glActiveTexture(GL_TEXTURE0);
				m_vecBoundTextures.push_back(std::make_pair(binding.slot, GL_TEXTURE_BUFFER));
			}
		}*/
	}

	void GLRenderDevice::BindRenderTargets(const DrawCallState &state)
	{
		FrameBuffer_ptr framebuffer = GetCachedFrameBuffer(state);

		if (framebuffer != m_CurrentFrameBuffer)
		{
			if (framebuffer)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->Handle);
				glDrawBuffers(GLsizei(framebuffer->NumBuffers), framebuffer->DrawBuffers);
			}
			else
			{
				glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
			}

			m_CurrentFrameBuffer = framebuffer;
		}

		glViewport(0, 0, state.ViewPort.x, state.ViewPort.y);
	}

	FrameBuffer_ptr GLRenderDevice::GetCachedFrameBuffer(const DrawCallState &state)
	{
		if(!state.HasAnyRenderTarget) {
			return nullptr;
		}

		CrcHash hasher;
		for (uint32_t rt = 0; rt < DrawCallState::MaxRenderTargets; rt++)
		{
			if(state.RenderTargets[rt].Texture != nullptr) {
				hasher.Add(state.RenderTargets[rt]);
			}
		}
		hasher.Add(state.DepthTarget);
		hasher.Add(state.DepthIndex);
		hasher.Add(state.DepthMipSlice);
		uint32_t hash = hasher.Get();

		auto it = m_CachedFrameBuffers.find(hash);
		if (it != m_CachedFrameBuffers.end())
			return it->second;

		auto framebuffer = std::make_shared<FrameBuffer>();

		glGenFramebuffers(1, &framebuffer->Handle);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->Handle);

		for (uint32_t rt = 0; rt < DrawCallState::MaxRenderTargets; rt++)
		{
			const auto& targetBinding = state.RenderTargets[rt];

			if(targetBinding.Texture == nullptr) {
				continue;
			}

			auto glTex = GetTexture(state.RenderTargets[rt].Texture);

			framebuffer->RenderTargets[rt] = state.RenderTargets[rt].Texture.get();
			glTex->m_UsedInFrameBuffers = true;

			if (targetBinding.Index == ~0u || glTex->GetDesc().DepthOrArraySize == 0) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + rt, glTex->BindTarget(), (glTex->EnabledBindSRGB && glTex->GetDesc().ImageFormat == GraphicsFormat::SRGBA8_UNORM) ? glTex->SRGBHandle() : glTex->Handle(), GLint(targetBinding.MipSlice));
			} else {
				glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + rt, GL_TEXTURE_CUBE_MAP_POSITIVE_X + targetBinding.Index, glTex->Handle(), GLint(targetBinding.MipSlice));
			}

			GLint encoding=-1;
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + rt, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &encoding);
			if (encoding == GL_LINEAR)
				AU_LOG_INFO("Framebuffer attachment ", targetBinding.Texture->GetDesc().DebugName, " (", rt, ") color encoding is linear.");
			if (encoding == GL_SRGB)
				AU_LOG_INFO("Framebuffer attachment ", targetBinding.Texture->GetDesc().DebugName, " (", rt, ") color encoding is sRGB.");

			//glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + rt, renderState.targets[rt]->handle, renderState.targetMipSlices[rt], renderState.targetIndicies[rt]);

			framebuffer->DrawBuffers[(framebuffer->NumBuffers)++] = GL_COLOR_ATTACHMENT0 + rt;
		}

		if (state.DepthTarget)
		{
			auto glDepthTex = GetTexture(state.DepthTarget);

			framebuffer->DepthTarget = glDepthTex;
			glDepthTex->m_UsedInFrameBuffers = true;

			GLenum attachment;

			if (state.DepthTarget->GetDesc().ImageFormat == GraphicsFormat::D24S8)
				attachment = GL_DEPTH_STENCIL_ATTACHMENT;
			else
				attachment = GL_DEPTH_ATTACHMENT;

			if (state.DepthIndex == ~0u || state.DepthTarget->GetDesc().DepthOrArraySize == 0) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, glDepthTex->BindTarget(), glDepthTex->Handle(), GLint(state.DepthMipSlice));
			} else {
				glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, glDepthTex->Handle(), GLint(state.DepthMipSlice), GLint(state.DepthIndex));
			}
		}

		CHECK_GL_ERROR();

		uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			AU_LOG_ERROR("Incomplete framebuffer!");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		m_CachedFrameBuffers[hash] = framebuffer;

		return framebuffer;
	}

	void GLRenderDevice::NotifyTextureDestroy(GLTexture* texture)
	{
		std::vector<uint32_t> frameBuffersToRemove;

		for(const auto& it : m_CachedFrameBuffers) {
			for(const auto& rt : it.second->RenderTargets) {
				if(texture != rt) {
					continue;
				}

				if(m_CurrentFrameBuffer == it.second) {
					m_CurrentFrameBuffer = nullptr;
				}

				frameBuffersToRemove.push_back(it.first);
				break;
			}
		}

		for(auto rt_id : frameBuffersToRemove) {
			m_CachedFrameBuffers.erase(rt_id);
		}
	}

	void GLRenderDevice::SetBlendState(const DrawCallState &state)
	{
		// TODO: Blend state
	}

	void GLRenderDevice::SetRasterState(const FRasterState& rasterState)
	{
		// TODO: Call gl commands only on change of values !

		switch (rasterState.FillMode)
		{
			case EFillMode::Line:
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				break;
			case EFillMode::Solid:
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				break;

			default:
				AU_LOG_WARNING("Unknown fill mode specified");
				break;
		}

		switch (rasterState.CullMode)
		{
			case ECullMode::Back:
				glCullFace(GL_BACK);
				glEnable(GL_CULL_FACE);
				break;
			case ECullMode::Front:
				glCullFace(GL_FRONT);
				glEnable(GL_CULL_FACE);
				break;
			case ECullMode::None:
				glDisable(GL_CULL_FACE);
				break;
			default:
				AU_LOG_WARNING("Unknown cullMode");
		}

		glFrontFace(rasterState.FrontCounterClockwise ? GL_CCW : GL_CW);

		if (rasterState.DepthClipEnable)
		{
			glEnable(GL_DEPTH_CLAMP);
		}

		if (rasterState.ScissorEnable)
		{
			glEnable(GL_SCISSOR_TEST);
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
		}

		if (rasterState.DepthBias != 0 || rasterState.SlopeScaledDepthBias != 0.f)
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(rasterState.SlopeScaledDepthBias, float(rasterState.DepthBias));
		}

		if (rasterState.MultisampleEnable)
		{
			glEnable(GL_MULTISAMPLE);
			glSampleMaski(0, ~0u);
			CHECK_GL_ERROR();
		}
		else
		{
			glDisable(GL_MULTISAMPLE);
		}
	}

	void GLRenderDevice::ClearRenderTargets(const DrawCallState &renderState)
	{
		uint32_t nClearBitField = 0;
		if (renderState.ClearColorTarget)
		{
			nClearBitField = GL_COLOR_BUFFER_BIT;
			glClearColor(renderState.ClearColor.r, renderState.ClearColor.g, renderState.ClearColor.b, renderState.ClearColor.a);
		}

		if (renderState.ClearDepthTarget)
		{
			nClearBitField |= GL_DEPTH_BUFFER_BIT;
			glClearDepthf(renderState.ClearDepth);
		}

		if (renderState.ClearStencilTarget)
		{
			nClearBitField |= GL_STENCIL_BUFFER_BIT;
			glClearStencil(renderState.ClearStencil);
		}

		if (nClearBitField)
		{
			glClear(nClearBitField);
		}
	}

	void GLRenderDevice::SetDepthStencilState(FDepthStencilState depthState)
	{
		if (depthState.DepthEnable)
		{
			glEnable(GL_DEPTH_TEST);
			glDepthMask((depthState.DepthWriteMask == EDepthWriteMask::All) ? GL_TRUE : GL_FALSE);
			glDepthFunc(ConvertComparisonFunc(depthState.DepthFunc));
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}

		if (depthState.StencilEnable)
		{
			glEnable(GL_STENCIL_TEST);

			glStencilFuncSeparate(GL_FRONT, ConvertComparisonFunc(depthState.FrontFace.StencilFunc), depthState.StencilRefValue, depthState.StencilReadMask);
			glStencilOpSeparate(GL_FRONT, ConvertStencilOp(depthState.FrontFace.StencilFailOp),
								ConvertStencilOp(depthState.FrontFace.StencilDepthFailOp),
								ConvertStencilOp(depthState.FrontFace.StencilPassOp));

			glStencilFuncSeparate(GL_BACK, ConvertComparisonFunc(depthState.BackFace.StencilFunc), depthState.StencilRefValue, depthState.StencilReadMask);
			glStencilOpSeparate(GL_BACK, ConvertStencilOp(depthState.BackFace.StencilFailOp),
								ConvertStencilOp(depthState.BackFace.StencilDepthFailOp),
								ConvertStencilOp(depthState.BackFace.StencilPassOp));

			glStencilMask(depthState.StencilWriteMask);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
	}
}