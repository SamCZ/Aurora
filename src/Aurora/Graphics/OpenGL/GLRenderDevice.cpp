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

	GLRenderDevice::GLRenderDevice() : IRenderDevice(), m_PipelineState(), m_nVAO(0)
	{

	}

	GLRenderDevice::~GLRenderDevice()
	{
		glDeleteVertexArrays(1, &m_nVAO);
	}

	void GLRenderDevice::Init()
	{
		glGenVertexArrays(1,&m_nVAO);
		glBindVertexArray(m_nVAO);
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

		glBindBuffer(GL_COPY_READ_BUFFER, glBuffer->Handle());

		void* pMappedData = glMapBufferRange(GL_COPY_READ_BUFFER, 0, nBytesToRead, GL_MAP_READ_BIT);
		if (pMappedData)
		{
			memcpy(data, pMappedData, nBytesToRead);
			*dataSize = nBytesToRead;
		}
		else
		{
			*dataSize = 0;
		}

		glUnmapBuffer(GL_COPY_READ_BUFFER);
		glBindBuffer(GL_COPY_READ_BUFFER, GL_NONE);
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

		return nullptr;
	}

	void GLRenderDevice::Draw(const DrawCallState &state)
	{

	}

	void GLRenderDevice::DrawIndexed(const DrawCallState &state, const std::vector<DrawArguments> &args)
	{

	}

	void GLRenderDevice::DrawIndirect(const DrawCallState &state, const Buffer_ptr &indirectParams, uint32_t offsetBytes)
	{

	}

	void GLRenderDevice::Dispatch(const DispatchState &state, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ)
	{
		ApplyDispatchState(state);

		glDispatchCompute(GLuint(groupsX), GLuint(groupsY), GLuint(groupsZ));

		//GL_SHADER_IMAGE_ACCESS_BARRIER_BIT, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT, GL_BUFFER_UPDATE_BARRIER_BIT
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
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

	void GLRenderDevice::ApplyDispatchState(const DispatchState &state)
	{
		ApplyShader(state.Shader);
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
				// TODO: Set placeholder texture
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

				switch (targetTextureBinding->Access) {
					case TextureBinding::EAccess::Read: access = GL_READ_ONLY; break;
					case TextureBinding::EAccess::Write: access = GL_WRITE_ONLY; break;
					case TextureBinding::EAccess::ReadAndWrite: access = GL_READ_WRITE; break;
				}

				glBindImageTexture(imageResource.Binding, glTexture->Handle(), static_cast<GLint>(textureDesc.MipLevels), GL_TRUE, 0, access, format);
				CHECK_GL_ERROR();
				//m_vecBoundImages.push_back(imageResource.Binding);
			} else {
				glActiveTexture(GL_TEXTURE0 + imageResource.Binding);

				if(glTexture->Format().AbstractFormat == GraphicsFormat::SRGBA8_UNORM)
					glBindTexture(glTexture->BindTarget(), glTexture->SRGBView());
				else
					glBindTexture(glTexture->BindTarget(), glTexture->Handle());

				CHECK_GL_ERROR();

				//m_vecBoundTextures.emplace_back(imageResource.Binding, targetTexture->bindTarget);
			}
		}

		for(const auto& imageResource : shader->GetGLResource().GetImages()) {
			auto boundTextureIt = state.BoundTextures.find(imageResource.Name);

			const TextureBinding* targetTextureBinding = nullptr;

			if(boundTextureIt == state.BoundTextures.end()) {
				// TODO: Set placeholder texture
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

				glBindImageTexture(imageResource.Binding, glTexture->Handle(), level, layered ? GL_TRUE : GL_FALSE, layer, access, format);
				CHECK_GL_ERROR();
				//m_vecBoundImages.push_back(imageResource.Binding);
			} else {
				glActiveTexture(GL_TEXTURE0 + imageResource.Binding);

				if(glTexture->Format().AbstractFormat == GraphicsFormat::SRGBA8_UNORM)
					glBindTexture(glTexture->BindTarget(), glTexture->SRGBView());
				else
					glBindTexture(glTexture->BindTarget(), glTexture->Handle());

				CHECK_GL_ERROR();

				//m_vecBoundTextures.emplace_back(imageResource.Binding, targetTexture->bindTarget);
			}
		}

		// binding samplers
		// TODO: Samplers
		for(const auto& samplerResource : shader->GetGLResource().GetSamplers()) {
			auto boundSamplerIt = state.BoundSamplers.find(samplerResource.Name);

			Sampler_ptr targetSampler = nullptr;

			if(boundSamplerIt == state.BoundSamplers.end()) {
				// TODO: Set placeholder sampler
				continue;
			}

			targetSampler = boundSamplerIt->second;

			if(targetSampler == nullptr) {
				// TODO: Set placeholder sampler or throw error
				continue;
			}

			glBindSampler(samplerResource.Binding, GetSampler(targetSampler)->Handle());
			//m_vecBoundSamplers.push_back(samplerResource.Binding);
		}

		// binding constant buffers
		for(const auto& uniformResource : shader->GetGLResource().GetUniformBlocks()) {
			auto uniformBufferIt = state.BoundUniformBuffers.find(uniformResource.Name);

			Buffer_ptr uniformBufferHandle = nullptr;

			if(uniformBufferIt == state.BoundUniformBuffers.end()) {
				// TODO: Set empty buffer
				continue;
			}

			uniformBufferHandle = uniformBufferIt->second;

			if(uniformBufferHandle == nullptr) {
				// TODO: Set empty buffer or throw error
				continue;
			}

			glBindBufferBase(GL_UNIFORM_BUFFER, uniformResource.Binding, GetBuffer(uniformBufferHandle)->Handle());
			//m_vecBoundConstantBuffers.push_back(uniformResource.Binding);
		}


		// binding ssbo`s
		// TODO: Complete SSBO
		/*for(const auto& uniformResource : shader->GetGLResource().GetStorageBlocks()) {

		}
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
}