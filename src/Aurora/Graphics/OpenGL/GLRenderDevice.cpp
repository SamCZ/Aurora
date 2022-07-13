#include "GLRenderDevice.hpp"

#include "GLConversions.hpp"

#include <algorithm>

#include <cstdint>
#include <Aurora/Core/assert.hpp>
#include <Aurora/Core/String.hpp>
#include <Aurora/Core/Profiler.hpp>


static const char* g_BlitVS = R"(
const vec4 Tri[3] = {
	vec4(-1, 3, 0, 1),
	vec4(-1, -1, 0, 1),
	vec4(3, -1, 0, 1)
};

const vec2 Uvs[3] = {
	vec2(0, 2),
	vec2(0, 0),
	vec2(2, 0)
};

out vec2 TexCoords;

void main()
{
	gl_Position = Tri[gl_VertexID];
	TexCoords = Uvs[gl_VertexID];
}

)";

static const char* g_BlitPS = R"(
layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform sampler2D Source;

in vec2 TexCoords;

void main()
{
	//FragColor = texelFetch(Source, ivec2(gl_FragCoord.xy), 0);
	FragColor = texture(Source, TexCoords);
}
)";

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

	GLTexture* GetTexture(ITexture* texture)
	{
		return static_cast<GLTexture*>(texture); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
	}

	GLSampler* GetSampler(const Sampler_ptr& sampler)
	{
		return static_cast<GLSampler*>(sampler.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
	}

	GLShaderProgram* GetShader(const Shader_ptr& shader)
	{
		return static_cast<GLShaderProgram*>(shader.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
	}

	GLRenderDevice::GLRenderDevice()
	: IRenderDevice(),
	m_nVAO(0),
	m_nVAOEmpty(0),
	m_LastVao(0),
	m_ContextState(),
	m_LastRasterState(),
	m_LastDepthState(),
	m_LastViewPort(0, 0),
	m_LastInputLayout(nullptr),
	m_GpuVendor(EGpuVendor::Unknown)
	{

	}

	GLRenderDevice::~GLRenderDevice()
	{
		glDeleteVertexArrays(1, &m_nVAO);
		glDeleteVertexArrays(1, &m_nVAOEmpty);
	}

	void GLRenderDevice::Init()
	{
		CHECK_GL_ERROR_AND_THROW("Error before GLRenderDevice::Init");

		glGenVertexArrays(1, &m_nVAO);
		CHECK_GL_ERROR_AND_THROW("Cannot gen default vao");
		glGenVertexArrays(1, &m_nVAOEmpty);
		CHECK_GL_ERROR_AND_THROW("Cannot gen empty vao");
		glBindVertexArray(m_nVAO);
		CHECK_GL_ERROR_AND_THROW("Cannot bind empty vao");


		glDepthRangef(0.0f, 1.0f);
		CHECK_GL_ERROR_AND_THROW("Cannot set glDepthRangef");
		// Enable depth remapping to [0, 1] interval
		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		CHECK_GL_ERROR_AND_THROW("Cannot set glClipControl");

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		CHECK_GL_ERROR_AND_THROW("Cannot enable GL_TEXTURE_CUBE_MAP_SEAMLESS");

        const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
        const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model

        AU_LOG_INFO("GPU Vendor: ", vendor);
        AU_LOG_INFO("GPU Renderer: ", renderer);

		std::stringstream ssVendor;
		ssVendor << vendor;
		std::string vendorName = ssVendor.str();

		// TODO: Finish other GPU vendor types
		if (vendorName.find("NVIDIA") != std::string::npos)
		{
			m_GpuVendor = EGpuVendor::Nvidia;
		}
		else
		{
			m_GpuVendor = EGpuVendor::AMD;
		}

		{
			GLint size;
			glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &size);
			AU_LOG_INFO("GL_MAX_SHADER_STORAGE_BLOCK_SIZE is ", size, " bytes", "(", FormatBytes(size), ")");
		}

		{
			GLint size;
			glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &size);
			AU_LOG_INFO("GL_MAX_VERTEX_UNIFORM_COMPONENTS is ", size);
		}

		{
			GLint size;
			glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &size);
			AU_LOG_INFO("GL_MAX_UNIFORM_BLOCK_SIZE is ", size, " bytes", "(", FormatBytes(size), ")");
			AU_LOG_INFO("Max instances: ", (size / sizeof(Matrix4)));
		}

		{
			GLint size;
			glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &size);
			AU_LOG_INFO("GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT is ", size);
		}

		{
			GLint size;
			glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &size);
			AU_LOG_INFO("GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT is ", size);
		}

		glDisable(GL_MULTISAMPLE);
		CHECK_GL_ERROR_AND_THROW("Cannot disable multisampling");

		InvalidateState();
		CHECK_GL_ERROR_AND_THROW("Cannot invalidate state");

		SetRasterState(m_LastRasterState);
		CHECK_GL_ERROR_AND_THROW("Cannot set raster state");
		SetDepthStencilState(m_LastDepthState);
		CHECK_GL_ERROR_AND_THROW("Cannot set depth state");

		{ // Init blit shader
			ShaderProgramDesc desc("_Blit_Embedded");
			desc.AddShader(EShaderType::Vertex, g_BlitVS);
			desc.AddShader(EShaderType::Pixel, g_BlitPS);
			m_BlitShader = CreateShaderProgram(desc);
		}

		glFlush();
	}

	Shader_ptr GLRenderDevice::CreateShaderProgram(const ShaderProgramDesc &desc)
	{
		CHECK_GL_ERROR_AND_THROW("error before CreateShaderProgram", desc.GetName());
		CPU_DEBUG_SCOPE("CreateShaderProgram");

		const auto& shaderDescriptions = desc.GetShaderDescriptions();

		if (shaderDescriptions.empty())
		{
			AU_LOG_ERROR("No shaders provided !");
			return nullptr;
		}

		if (shaderDescriptions.contains(EShaderType::Compute) && shaderDescriptions.size() > 1)
		{
			AU_LOG_ERROR("You cannot mix compute shader with basic types of shaders in program ", desc.GetName(), " !");
			return nullptr;
		}

		/*if(!shaderDescriptions.contains(EShaderType::Vertex) || !shaderDescriptions.contains(EShaderType::Pixel) || !shaderDescriptions.contains(EShaderType::Compute)) {
			AU_LOG_ERROR("You must always provide Vertex and Pixel or Compute shader !");
			return nullptr;
		}*/

		std::vector<GLuint> compiledShaders;
		for (const auto& it : shaderDescriptions)
		{
			const auto& shaderDesc = it.second;
			EShaderType type = shaderDesc.Type;

			std::stringstream ss;

			ss << "#version 430 core\n";
			ss << "layout(std140) uniform;\n";

			switch (type)
			{
				case EShaderType::Vertex:
					ss << "#define SHADER_VERTEX\n";
					break;
				case EShaderType::Hull:
					ss << "#define SHADER_HULL\n";
					break;
				case EShaderType::Domain:
					ss << "#define SHADER_DOMAIN\n";
					break;
				case EShaderType::Geometry:
					ss << "#define SHADER_GEOMETRY\n";
					break;
				case EShaderType::Pixel:
					ss << "#define SHADER_PIXEL\n";
					break;
				case EShaderType::Compute:
					ss << "#define SHADER_COMPUTE\n";
					break;
				default:
					break;
			}

			for (const auto& macrosIt : shaderDesc.Macros)
			{
				ss << "#define " << macrosIt.first << " " << macrosIt.second << "\n";
			}

			ss << shaderDesc.Source;

			String glslSourcePreprocessed = ss.str();

			if (type == EShaderType::Pixel && shaderDesc.EnableBindless)
			{
				String ext;
				ext += "#extension GL_ARB_bindless_texture : enable\n";
				ext += "#extension GL_ARB_gpu_shader_int64 : enable\n";
				glslSourcePreprocessed.insert(18, ext);
			}

			std::string error;
			GLuint shaderID = CompileShaderRaw(glslSourcePreprocessed, type, &error);

			if (shaderID == 0)
			{
				AU_LOG_ERROR("Cannot compile shader ", ShaderType_ToString(type), " in program ", desc.GetName(), "!\n", error);
				return nullptr;
			}

			compiledShaders.push_back(shaderID);
		}

		GLuint programID = glCreateProgram();
		CHECK_GL_ERROR_AND_THROW("Could not create program ", desc.GetName());

		// Attach shaders

		for (auto shaderID : compiledShaders)
		{
			glAttachShader(programID, shaderID);
			CHECK_GL_ERROR_AND_THROW("Could not attach shader ", desc.GetName());
		}

		// Link program

		glLinkProgram(programID);
		CHECK_GL_ERROR_AND_THROW("Could not link program ", desc.GetName());
		GLint linkStatus = 0;
		glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus);
		CHECK_GL_ERROR_AND_THROW("Could not get link status ", desc.GetName());

		if (linkStatus == GL_FALSE)
		{
			GLchar error[1024] = { 0 };
			GLsizei logLength = 0;
			glGetProgramInfoLog(programID, sizeof(error), &logLength, error);
			CHECK_GL_ERROR_AND_THROW("Could not get error log ", desc.GetName());

			if (desc.HasSetErrorOutput())
			{
				desc.GetErrorOutput()(error);
			}
			else
			{
				AU_LOG_ERROR("Cannot link shader program ", desc.GetName(), ": ", error);
			}

			return nullptr;
		}

		// Validate program

		glValidateProgram(programID);
		CHECK_GL_ERROR_AND_THROW("Could not validate program ", desc.GetName());
		GLint validateStatus = 0;
		glGetProgramiv(programID, GL_VALIDATE_STATUS, &validateStatus);
		CHECK_GL_ERROR_AND_THROW("Could get validation status", desc.GetName());

		if (validateStatus == GL_FALSE)
		{
			GLchar error[1024] = { 0 };
			GLsizei logLength = 0;
			glGetProgramInfoLog(programID, sizeof(error), &logLength, error);
			CHECK_GL_ERROR_AND_THROW("Could not get error log");

			if (desc.HasSetErrorOutput())
			{
				desc.GetErrorOutput()(error);
			}
			else
			{
				AU_LOG_ERROR("Cannot validate shader program ", desc.GetName(), ": ", error);
			}

			return nullptr;
		}

		auto shaderProgram = std::make_shared<GLShaderProgram>(programID, desc);

		// Cleanup before returning shader program object
		for (auto shaderID : compiledShaders)
		{
			glDetachShader(programID, shaderID);
			glDeleteShader(shaderID);
		}


		glUseProgram(programID);
		glObjectLabel(GL_PROGRAM, programID, static_cast<GLsizei>(desc.GetName().size()), desc.GetName().c_str());
		glUseProgram(0);

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
		CHECK_GL_ERROR_AND_THROW("Cannot create shader", sourceString);

		const GLchar* source = sourceString.data();
		auto length = static_cast<GLint>(sourceString.length());
		glShaderSource(shaderID, 1, &source, &length);
		glCompileShader(shaderID);
		CHECK_GL_ERROR_AND_THROW("Cannot compile shader", sourceString);

		GLint compileStatus = 0;
		GLchar error[1024] = { 0 };
		GLsizei logLength = 0;

		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileStatus);
		CHECK_GL_ERROR_AND_THROW("Cannot get compile status", sourceString);
		if (compileStatus == GL_FALSE)
		{
			glGetShaderInfoLog(shaderID, sizeof(error), &logLength, error);
			if (errorOutput)
			{
				*errorOutput = error;
			}
			else
			{
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

		if (desc.DimensionType == EDimensionType::TYPE_CubeMap)
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
		else if (desc.DimensionType == EDimensionType::TYPE_2DArray)
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
		else if (desc.DimensionType == EDimensionType::TYPE_3D)
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
		else if (desc.SampleCount > 1 && !desc.UseAsBindless)
		{
			AU_LOG_FATAL("Multisample is not supported !");
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

			//glTexParameteri(bindTarget, GL_TEXTURE_SPARSE_ARB, GL_TRUE);

			glTexStorage2D(
					bindTarget,
					(GLsizei)desc.MipLevels,
					formatMapping.InternalFormat,
					(GLsizei)desc.Width,
					(GLsizei)desc.Height);

			CHECK_GL_ERROR();
		}

		if (desc.SampleCount == 1 && !desc.UseAsBindless)
		{
			glTexParameteri(bindTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(bindTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			CHECK_GL_ERROR();
		}

		glObjectLabel(GL_TEXTURE, handle, static_cast<GLsizei>(desc.Name.size()), desc.Name.c_str());
		CHECK_GL_ERROR();

		GLuint64 bHandle = 0;
		GLuint64 bHandleSrgb = 0;

		if (desc.UseAsBindless)
		{
			bHandle = glGetTextureHandleARB(handle);
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

			glBindTexture(bindTarget, srgbView);

			std::string srgbName = desc.Name + "_SRGB";
			glObjectLabel(GL_TEXTURE, srgbView, static_cast<GLsizei>(srgbName.size()), srgbName.c_str());

			if (desc.SampleCount == 1)
			{

				glTexParameteri(bindTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(bindTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				CHECK_GL_ERROR();
			}

			if (desc.UseAsBindless)
			{
				bHandleSrgb = glGetTextureHandleARB(srgbView);
				CHECK_GL_ERROR();
			}

			glBindTexture(bindTarget, 0);
		}

		// TODO: Write @textureData

		return std::make_shared<GLTexture>(desc, formatMapping, handle, srgbView, bindTarget, bHandle, bHandleSrgb);
	}

	void GLRenderDevice::WriteTexture(const Texture_ptr &texture, uint32_t mipLevel, uint32_t subresource, const void *data)
	{
		auto* glTexture = static_cast<GLTexture*>(texture.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		glBindTexture(glTexture->BindTarget(), glTexture->Handle());

		const auto& desc = texture->GetDesc();

		uint32_t width = std::max<uint32_t>(1, desc.Width >> mipLevel);
		uint32_t height = std::max<uint32_t>(1, desc.Height >> mipLevel);

		if  (desc.DimensionType == EDimensionType::TYPE_CubeMap)
		{
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + subresource, (GLint)mipLevel, 0, 0, (GLsizei)width, (GLsizei)height, glTexture->Format().BaseFormat, glTexture->Format().Type, data);
			CHECK_GL_ERROR();
		}
		else if (desc.DimensionType == EDimensionType::TYPE_2DArray || desc.DepthOrArraySize > 0)
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

	void GLRenderDevice::ClearTextureFloat(const Texture_ptr &texture, float val)
	{
		auto* glTexture = static_cast<GLTexture*>(texture.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		for (GLint nMipLevel = 0; nMipLevel < glTexture->GetDesc().MipLevels; ++nMipLevel)
		{
			glClearTexImage(glTexture->Handle(), nMipLevel, glTexture->Format().BaseFormat, GL_FLOAT, &val);
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

	void* GLRenderDevice::GetTextureHandleForBindless(const Texture_ptr& texture, bool srgb)
	{
		if (texture == nullptr) return nullptr;

		GLTexture* glTexture = GetTexture(texture);

		if (texture->GetDesc().ImageFormat == GraphicsFormat::SRGBA8_UNORM && srgb)
		{
			return &glTexture->m_BindlessSrgbHandle;
		}

		return &glTexture->m_BindlessHandle;
	}

	bool GLRenderDevice::MakeTextureHandleResident(const Texture_ptr& texture, bool enabled)
	{
		if (texture == nullptr)
			return false;

		GLTexture* glTexture = GetTexture(texture);

		if (!glTexture->GetDesc().UseAsBindless)
			return false;

		if(enabled)
		{
			glMakeTextureHandleResidentARB(glTexture->m_BindlessHandle);
			if (glTexture->m_BindlessSrgbHandle != 0)
			{
				glMakeTextureHandleResidentARB(glTexture->m_BindlessSrgbHandle);
			}
		}
		else
		{
			glMakeTextureHandleNonResidentARB(glTexture->m_BindlessHandle);
			if (glTexture->m_BindlessSrgbHandle != 0)
			{
				glMakeTextureHandleNonResidentARB(glTexture->m_BindlessSrgbHandle);
			}
		}

		return true;
	}

	Buffer_ptr GLRenderDevice::CreateBuffer(const BufferDesc &desc, const void *data)
	{
		GLuint handle = 0;
		GLenum bindTarget = ConvertBufferType(desc.Type);
		GLenum usage = ConvertUsage(desc.Usage);
		uint8_t* mappedData = nullptr;

		if (desc.Type != EBufferType::TextureBuffer)
		{
			glGenBuffers(1, &handle);
			glBindBuffer(bindTarget, handle);

			glObjectLabel(GL_BUFFER, handle, static_cast<GLsizei>(desc.Name.size()), desc.Name.c_str());

			if (desc.IsDMA)
			{
				glBufferStorage(bindTarget, desc.ByteSize, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_DYNAMIC_STORAGE_BIT);
				mappedData = (uint8_t*)glMapBufferRange(bindTarget, 0, desc.ByteSize, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
				if(data) memcpy(mappedData, data, desc.ByteSize);
			}
			else
			{
				glBufferData(bindTarget, desc.ByteSize, data, usage);
			}
			CHECK_GL_ERROR();

			glBindBuffer(bindTarget, GL_NONE);
		}
		else
		{
			glGenTextures(1, &handle);
			glBindTexture(GL_TEXTURE_BUFFER, handle);

			glObjectLabel(GL_TEXTURE, handle, static_cast<GLsizei>(desc.Name.size()), desc.Name.c_str());

			glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, handle);
			CHECK_GL_ERROR();

			glBindTexture(GL_TEXTURE_BUFFER, GL_NONE);
		}

		auto buffer = std::make_shared<GLBuffer>(desc, handle, bindTarget, usage);
		buffer->m_MappedData = mappedData;
		return buffer;
	}

	void GLRenderDevice::WriteBuffer(const Buffer_ptr &buffer, const void *data, size_t dataSize, size_t offset)
	{
		CPU_DEBUG_SCOPE("GLRenderDevice::WriteBuffer");
		if (buffer == nullptr)
		{
			return;
		}

		auto* glBuffer = static_cast<GLBuffer*>(buffer.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		if (!(glBuffer->GetDesc().Usage == EBufferUsage::DynamicDraw || glBuffer->GetDesc().Usage == EBufferUsage::DynamicRead || glBuffer->GetDesc().Usage == EBufferUsage::DynamicCopy))
		{
			//AU_LOG_ERROR("Cannot write to static buffer !");
		}

		au_assert(glBuffer->GetDesc().ByteSize >= offset + dataSize);

		if (glBuffer->GetDesc().IsDMA)
		{
			memcpy(glBuffer->m_MappedData + offset, data, dataSize);
			m_FrameRenderStatistics.BufferWrites++;
			return;
		}

		glBindBuffer(glBuffer->BindTarget(), glBuffer->Handle());

		if (dataSize > glBuffer->GetDesc().ByteSize)
			dataSize = glBuffer->GetDesc().ByteSize;

		//memcpy(glBuffer->m_MappedData, data, dataSize);
		//glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		glBufferSubData(glBuffer->BindTarget(), offset, GLsizeiptr(dataSize), data);
		CHECK_GL_ERROR();

		//glBindBuffer(glBuffer->BindTarget(), GL_NONE);

		m_FrameRenderStatistics.BufferWrites++;
	}

	void GLRenderDevice::ClearBufferUInt(const Buffer_ptr &buffer, uint32_t clearValue)
	{
		if (buffer == nullptr)
		{
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
		if (dest == nullptr)
		{
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

	uint8_t* GLRenderDevice::MapBuffer(const Buffer_ptr& buffer, EBufferAccess bufferAccess)
	{
		if (buffer == nullptr)
		{
			return nullptr;
		}
		m_FrameRenderStatistics.BufferMaps++;

		auto* glBuffer = static_cast<GLBuffer*>(buffer.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		if (buffer->GetDesc().IsDMA)
		{
			return glBuffer->m_MappedData;
		}

		glBindBuffer(glBuffer->BindTarget(), glBuffer->Handle());
		return reinterpret_cast<uint8_t*>(glMapBuffer(glBuffer->BindTarget(), ConvertBufferAccess(bufferAccess)));
		//GLvoid* pMappedData = glMapBufferRange(glBuffer->BindTarget(), 0, glBuffer->GetDesc().ByteSize, GL_MAP_WRITE_BIT);//GL_MAP_UNSYNCHRONIZED_BIT
		//return glBuffer->m_MappedData;
	}

	void GLRenderDevice::UnmapBuffer(const Buffer_ptr& buffer)
	{
		if (buffer == nullptr || buffer->GetDesc().IsDMA)
		{
			return;
		}
		auto* glBuffer = static_cast<GLBuffer*>(buffer.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
		glUnmapBuffer(glBuffer->BindTarget());
		glBindBuffer(glBuffer->BindTarget(), GL_NONE);
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

	void GLRenderDevice::Draw(const DrawCallState &state, const std::vector<DrawArguments>& args, bool bindState)
	{
		CPU_DEBUG_SCOPE("Draw");
		if (bindState)
		{
			if (state.Shader == nullptr)
			{
				AU_LOG_ERROR("Cannot draw without shader !");
				throw;
				return;
			}

			glBindVertexArray(m_nVAOEmpty); // FIXME: idk why, but when frustum clips all geometry and nothing renders,then this call happens, it will throw error in non bound Array (maybe it does NanoVG?)
			m_LastVao = m_nVAOEmpty;
		}

		if(bindState)
			ApplyDrawCallState(state);

		GLenum primitiveType = ConvertPrimType(state.PrimitiveType);

		for (const auto& drawArg : args)
		{
			glDrawArraysInstanced(primitiveType, GLint(drawArg.StartVertexLocation), GLint(drawArg.VertexCount), GLint(drawArg.InstanceCount));
			CHECK_GL_ERROR();
			m_FrameRenderStatistics.VertexCount += drawArg.VertexCount * drawArg.InstanceCount;
		}

		m_FrameRenderStatistics.DrawCalls++;
	}

	#define BUFFER_OFFSET(i) ((char *)NULL + (i))

	GLuint lastUsedElementBuffer = 0;

	void GLRenderDevice::DrawIndexed(const DrawCallState &state, const std::vector<DrawArguments> &args, bool bindState)
	{
		CPU_DEBUG_SCOPE("DrawIndexed");
		if (state.IndexBuffer.Buffer == nullptr || state.Shader == nullptr)
		{
			AU_LOG_ERROR("Cannot draw with these arguments !");
			throw;
			return;
		}

		if (bindState)
			ApplyDrawCallState(state);

		CHECK_GL_ERROR();

		GLBuffer* ib = GetBuffer(state.IndexBuffer.Buffer);

		if (lastUsedElementBuffer != ib->Handle())
		{
			lastUsedElementBuffer = ib->Handle();
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->Handle());

		GLenum primitiveType = ConvertPrimType(state.PrimitiveType);
		GLenum ibFormat = ConvertIndexBufferFormat(state.IndexBuffer.Format);

		if(args.size() == 1)
		{
			if (args[0].InstanceCount > 1)
			{
				//glDrawElementsInstancedBaseVertex(primitiveType, GLsizei(drawArg.VertexCount), ibFormat, (const void*)size_t(drawArg.StartIndexLocation), GLsizei(drawArg.InstanceCount), GLint(drawArg.StartVertexLocation));
				glDrawElementsInstanced(primitiveType, GLsizei(args[0].VertexCount), ibFormat, BUFFER_OFFSET(args[0].StartIndexLocation), GLsizei(args[0].InstanceCount));
			}
			else
			{
				glDrawElementsBaseVertex(primitiveType, GLsizei(args[0].VertexCount), ibFormat, (const void*)size_t(args[0].StartIndexLocation), GLint(args[0].StartVertexLocation));
			}

			m_FrameRenderStatistics.DrawCalls++;
			return;
		}

		GLsizei* count = nullptr;
		uintptr_t* indices = nullptr;
		uint16_t multiDrawCount = 0;

		for (const auto& drawArg : args)
		{
			if (drawArg.InstanceCount > 1)
			{
				//glDrawElementsInstancedBaseVertex(primitiveType, GLsizei(drawArg.VertexCount), ibFormat, (const void*)size_t(drawArg.StartIndexLocation), GLsizei(drawArg.InstanceCount), GLint(drawArg.StartVertexLocation));
				glDrawElementsInstanced(primitiveType, GLsizei(drawArg.VertexCount), ibFormat, BUFFER_OFFSET(drawArg.StartIndexLocation), GLsizei(drawArg.InstanceCount));
			}
			else
			{
				//glDrawElementsBaseVertex(primitiveType, GLsizei(drawArg.VertexCount), ibFormat, (const void*)size_t(drawArg.StartIndexLocation), GLint(drawArg.StartVertexLocation));
				//glDrawElements(primitiveType, GLsizei(drawArg.VertexCount), ibFormat, BUFFER_OFFSET(drawArg.StartIndexLocation));

				if (!count)
					count = (GLsizei*)alloca( sizeof(GLsizei) * args.size());
				if (!indices)
					indices = (uintptr_t*)alloca( sizeof(uintptr_t) * args.size());

				count[multiDrawCount] = GLsizei(drawArg.VertexCount);
				indices[multiDrawCount] = drawArg.StartIndexLocation;
				multiDrawCount++;
			}
			m_FrameRenderStatistics.VertexCount += drawArg.VertexCount * 3 * drawArg.InstanceCount;
		}

		if (multiDrawCount > 0)
			glMultiDrawElements(primitiveType, count, ibFormat, (const void* const*)indices, multiDrawCount);

		/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);
		glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);*/

		m_FrameRenderStatistics.DrawCalls++;
	}

	void GLRenderDevice::DrawIndirect(const DrawCallState &state, const Buffer_ptr &indirectParams, uint32_t offsetBytes)
	{
		CPU_DEBUG_SCOPE("DrawIndirect");

		GLenum primitiveType = ConvertPrimType(state.PrimitiveType);
		GLenum ibFormat = ConvertIndexBufferFormat(state.IndexBuffer.Format);

		GLBuffer* indexbuff = GetBuffer(state.IndexBuffer.Buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuff->Handle());

		GLBuffer* ib = GetBuffer(indirectParams);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ib->Handle());
		//glDrawElementsIndirect(primitiveType, ibFormat, nullptr);

		glMultiDrawElementsIndirect(primitiveType, ibFormat, (const void*)0, offsetBytes, sizeof(DrawElementsIndirectCommand));
	}

	void GLRenderDevice::ApplyDrawCallState(const DrawCallState &state)
	{
		SetShader(state.Shader);

		BindShaderInputs(state, false);
		BindShaderResources(state);

		BindRenderTargets(state);

		SetBlendState(state.BlendState);
		SetRasterState(state.RasterState);

		SetDepthStencilState(state.DepthStencilState);

		ClearRenderTargets(state);
	}

	void GLRenderDevice::BindShaderInputsCached(const DrawCallState &state)
	{
		auto glShader = GetShader(state.Shader);

		if (
				(glShader == nullptr && m_LastVao != m_nVAOEmpty) ||
				(glShader != nullptr && glShader->GetInputVariables().empty()) ||
				(state.InputLayoutHandle == nullptr))
		{
			m_LastVao = m_nVAOEmpty;
			glBindVertexArray(m_nVAOEmpty);
			return;
		}

		VaoKey key = {
			.Shader = state.Shader,
			.Buffers = state.VertexBuffers
		};

		auto it = m_CachedVaos.find(key);

		// Find existing
		if (it != m_CachedVaos.end())
		{
			GLuint vao = it->second;

			if (vao != m_LastVao)
			{
				m_LastVao = vao;
				glBindVertexArray(vao);
			}
			return;
		}

		const auto& inputVars = glShader->GetInputVariables();

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		for (const auto& [location, inputVariable] : inputVars)
		{
			VertexAttributeDesc layoutAttribute;

			/*if (!state.InputLayoutHandle->GetDescriptorBySemanticID(location, layoutAttribute))
			{
				AU_LOG_FATAL("Input layout from DrawState is not supported by that in the shader !");
				continue;
			}*/

			if (!state.InputLayoutHandle->GetDescriptorByName(inputVariable.Name, layoutAttribute))
			{
				//AU_LOG_FATAL("Input layout from DrawState is not supported by that in the shader (" , inputVariable.Name, ") !");
				continue;
			}

			if (inputVariable.Format != layoutAttribute.Format)
			{
				AU_LOG_FATAL("Input descriptor format is not the same !");
			};

			const FormatMapping& formatMapping = GetFormatMapping(layoutAttribute.Format);

			auto vertexBufferIt = state.VertexBuffers.find(layoutAttribute.BufferIndex);
			auto glBuffer = GetBuffer(vertexBufferIt->second);
			glBindBuffer(GL_ARRAY_BUFFER, glBuffer->Handle());

			glEnableVertexAttribArray(location);

			if (formatMapping.Type == GL_INT || formatMapping.Type == GL_UNSIGNED_INT || formatMapping.Type == GL_UNSIGNED_SHORT || formatMapping.Type == GL_SHORT)
			{
				glVertexAttribIPointer(
					location,
					GLint(formatMapping.Components),
					formatMapping.Type,
					GLsizei(layoutAttribute.Stride),
					(const void*)size_t(layoutAttribute.Offset));
			}
			else
			{
				glVertexAttribPointer(
					location,
					GLint(formatMapping.Components),
					formatMapping.Type,
					layoutAttribute.Normalized ? GL_TRUE : GL_FALSE,
					GLsizei(layoutAttribute.Stride),
					(const void*)size_t(layoutAttribute.Offset));
			}

			m_FrameRenderStatistics.GPUMemoryUsage += formatMapping.Components * sizeof(float);

			glVertexAttribDivisor(location, layoutAttribute.IsInstanced ? 1 : 0);
		}

		m_CachedVaos[key] = vao;
	}

	void GLRenderDevice::BindShaderInputs(const DrawCallState &state, bool force)
	{
		/*if (true)
		{
		    // FIXME: Sometime the screen in black wher rotating camera
			BindShaderInputsCached(state);
			return;
		}*/

		auto glShader = GetShader(state.Shader);
		const auto& inputVars = glShader->GetInputVariables();

		if (inputVars.empty() || state.InputLayoutHandle == nullptr)
		{
			if (m_LastVao != m_nVAOEmpty)
			{
				m_LastVao = m_nVAOEmpty;
				glBindVertexArray(m_nVAOEmpty);
			}
			return;
		}

		if (m_LastVao != m_nVAO)
		{
			m_LastVao = m_nVAO;
			glBindVertexArray(m_nVAO);
		}

		if (state.InputLayoutHandle == m_LastInputLayout && !force)
			return;
		m_LastInputLayout = state.InputLayoutHandle;

		for (const auto& [location, inputVariable] : inputVars)
		{
			VertexAttributeDesc layoutAttribute;

			/*if (!state.InputLayoutHandle->GetDescriptorBySemanticID(location, layoutAttribute))
			{
				AU_LOG_FATAL("Input layout from DrawState is not supported by that in the shader !");
				continue;
			}*/

			if (!state.InputLayoutHandle->GetDescriptorByName(inputVariable.Name, layoutAttribute))
			{
				//AU_LOG_FATAL("Input layout from DrawState is not supported by that in the shader (" , inputVariable.Name, ") !");
				continue;
			}

			if (inputVariable.Format != layoutAttribute.Format)
			{
				AU_LOG_FATAL("Input descriptor format is not the same !");
			};

			const FormatMapping& formatMapping = GetFormatMapping(layoutAttribute.Format);

			auto vertexBufferIt = state.VertexBuffers.find(layoutAttribute.BufferIndex);
			auto glBuffer = GetBuffer(vertexBufferIt->second);
			glBindBuffer(GL_ARRAY_BUFFER, glBuffer->Handle());

			glEnableVertexAttribArray(location);

			if (formatMapping.Type == GL_INT || formatMapping.Type == GL_UNSIGNED_INT || formatMapping.Type == GL_UNSIGNED_SHORT || formatMapping.Type == GL_SHORT)
			{
				glVertexAttribIPointer(
						location,
						GLint(formatMapping.Components),
						formatMapping.Type,
						GLsizei(layoutAttribute.Stride),
						(const void*)size_t(layoutAttribute.Offset));
			}
			else
			{
				glVertexAttribPointer(
						location,
						GLint(formatMapping.Components),
						formatMapping.Type,
						layoutAttribute.Normalized ? GL_TRUE : GL_FALSE,
						GLsizei(layoutAttribute.Stride),
						(const void*)size_t(layoutAttribute.Offset));
			}

			m_FrameRenderStatistics.GPUMemoryUsage += formatMapping.Components * sizeof(float);

			glVertexAttribDivisor(location, layoutAttribute.IsInstanced ? 1 : 0);
		}

		//glBindBuffer(GL_ARRAY_BUFFER, GL_NONE); Do not do this, android render will fail !

		/*if (m_LastInputLayout != nullptr)
		{
			if (inputVars.size() == m_LastInputLayout->GetDescriptors().size())
			{
				return;
			}
		}

		static GLint nMaxVertexAttrs = 0;
		if (nMaxVertexAttrs == 0)
			glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nMaxVertexAttrs);

		for (auto semanticIndex = 0; semanticIndex < nMaxVertexAttrs; semanticIndex++)
		{
			bool foundSemantic = false;
			for (const auto& var : inputVars)
			{
				if (semanticIndex == var.first)
				{
					foundSemantic = true;
					break;
				}
			}
			if (!foundSemantic)
			{
				glDisableVertexAttribArray(GLuint(semanticIndex));
			}
		}*/
	}

	void GLRenderDevice::Dispatch(const DispatchState &state, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ)
	{
		CPU_DEBUG_SCOPE("Dispatch");
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

		m_LastRasterState = FRasterState();
		m_LastDepthState = FDepthStencilState();

		m_LastRasterState.FillMode = EFillMode::Solid;
		m_LastRasterState.CullMode = ECullMode::Front;
		m_LastRasterState.DepthClipEnable = false;

		m_LastDepthState.DepthEnable = false;
	}

	void GLRenderDevice::ApplyDispatchState(const DispatchState &state)
	{
		SetShader(state.Shader);
		BindShaderResources(state);
	}

	void GLRenderDevice::BindShaderResources(const BaseState& state)
	{
		CHECK_GL_ERROR();

		if (state.Shader == nullptr) return;

		auto shader = static_cast<GLShaderProgram*>(state.Shader.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		for (const auto& imageResource : shader->GetGLResource().GetSamplers())
		{
			auto boundTextureIt = state.BoundTextures.find(imageResource.Name);

			const TextureBinding* targetTextureBinding = nullptr;

			if (boundTextureIt == state.BoundTextures.end())
			{
				m_ContextState.BindTexture(imageResource.Binding, nullptr);
				continue;
			}

			targetTextureBinding = &boundTextureIt->second;

			if (targetTextureBinding == nullptr)
			{
				m_ContextState.BindTexture(imageResource.Binding, nullptr);
				// TODO: Set placeholder texture or throw error
				continue;
			}

			auto* glTexture = GetTexture(targetTextureBinding->Texture);

			if (glTexture == nullptr)
			{
				m_ContextState.BindTexture(imageResource.Binding, nullptr);
				// TODO: Set placeholder texture or throw error
				continue;
			}

			const TextureDesc& textureDesc = glTexture->GetDesc();

			m_ContextState.BindTexture(imageResource.Binding, glTexture);
		}

		for (const auto& imageResource : shader->GetGLResource().GetImages())
		{
			auto boundTextureIt = state.BoundTextures.find(imageResource.Name);

			const TextureBinding* targetTextureBinding = nullptr;

			if (boundTextureIt == state.BoundTextures.end())
			{
				m_ContextState.BindImage(imageResource.Binding, nullptr, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB16F);
				continue;
			}

			targetTextureBinding = &boundTextureIt->second;

			if (targetTextureBinding == nullptr)
			{
				// TODO: Set placeholder texture or throw error
				continue;
			}

			auto* glTexture = GetTexture(targetTextureBinding->Texture);

			const TextureDesc& textureDesc = glTexture->GetDesc();

			if (textureDesc.IsUAV && targetTextureBinding->IsUAV)
			{
				GLenum format = glTexture->Format().InternalFormat;
				GLenum access = GL_WRITE_ONLY;
				bool layered = textureDesc.DepthOrArraySize > 0;
				GLint layer = 0;

				switch (targetTextureBinding->Access)
				{
					case TextureBinding::EAccess::Read: access = GL_READ_ONLY; break;
					case TextureBinding::EAccess::Write: access = GL_WRITE_ONLY; break;
					case TextureBinding::EAccess::ReadAndWrite: access = GL_READ_WRITE; break;
				}

				m_ContextState.BindImage(imageResource.Binding, glTexture, targetTextureBinding->MipLevel, layered, layer, access, format);
			}
			else
			{
				AU_LOG_WARNING("Trying to bind image as UAV but somewhere the texture is not marked as UAV");
			}
		}

		// binding samplers
		// TODO: Samplers
		for(const auto& samplerResource : shader->GetGLResource().GetSamplers())
		{
			auto boundSamplerIt = state.BoundSamplers.find(samplerResource.Name);

			Sampler_ptr targetSampler = nullptr;

			if (boundSamplerIt != state.BoundSamplers.end()) {
				targetSampler = boundSamplerIt->second;
			}

			m_ContextState.BindSampler(samplerResource.Binding, GetSampler(targetSampler));
		}

		// binding constant buffers
		for(const auto& uniformResource : shader->GetGLResource().GetUniformBlocks())
		{
			auto uniformBufferIt = state.BoundUniformBuffers.find(uniformResource.Name);

			BufferBinding uniformBinding;

			if (uniformBufferIt != state.BoundUniformBuffers.end()) {
				uniformBinding = uniformBufferIt->second;
			}

			GLBuffer* glBuffer = nullptr;

			if (uniformBinding.Size == 0 && uniformBinding.Buffer != nullptr)
			{
				uniformBinding.Size = uniformBinding.Buffer->GetDesc().ByteSize;
			}

			if (uniformBinding.Buffer != nullptr)
			{
				glBuffer = GetBuffer(uniformBinding.Buffer);
			}
			else
			{
				continue;
			}

			m_ContextState.BindUniformBuffer(uniformResource.Binding, glBuffer, uniformBinding.Offset, uniformBinding.Size);
		}


		// binding ssbo`s
		for (const auto& uniformResource : shader->GetGLResource().GetStorageBlocks())
		{
			auto ssboIt = state.SSBOBuffers.find(uniformResource.Name);

			BufferBinding ssboBinding;

			if (ssboIt != state.SSBOBuffers.end()) {
				ssboBinding = ssboIt->second;
			}

			GLBuffer* glBuffer = nullptr;

			if (ssboBinding.Size == 0 && ssboBinding.Buffer != nullptr)
			{
				ssboBinding.Size = ssboBinding.Buffer->GetDesc().ByteSize;
			}

			if (ssboBinding.Buffer != nullptr)
			{
				glBuffer = GetBuffer(ssboBinding.Buffer);
			}
			else
			{
				continue;
			}

			m_ContextState.BindStorageBlock(uniformResource.Binding, glBuffer, ssboBinding.Offset, ssboBinding.Size);
		}

		ApplyShaderUniformResources(state.Shader, state.Uniforms);
	}

	void GLRenderDevice::ApplyShaderUniformResources(const Shader_ptr& shader, const UniformResources& resources)
	{
		CPU_DEBUG_SCOPE("ApplyShaderUniformResources");

		if (shader == nullptr) return;

		auto glShader = static_cast<GLShaderProgram*>(shader.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		const auto& uniforms = glShader->GetGLResource().GetUniforms();

		for (const auto& [typeID, uniform]: resources.Uniforms)
		{
			auto uniformTypeIt = uniforms.find(typeID);

			if (uniformTypeIt == uniforms.end())
				continue;

			const UniformInfo& uniformInfo = uniformTypeIt->second;

			if (uniformInfo.Type != uniform.Type)
			{
				AU_LOG_WARNING("Uniform ", uniformInfo.Name, " ", VarType_Strings[(int)uniformInfo.Type], " has wrong type ", VarType_Strings[(int)uniform.Type],"!");
				continue;
			}

			// TODO: Complete types

			switch (uniformInfo.Type)
			{
				case VarType::Float:
					glUniform1f(uniformInfo.Location, uniform.TypeData.Float);
					break;
				case VarType::Int:
					glUniform1i(uniformInfo.Location, uniform.TypeData.Int);
					break;
				case VarType::UnsignedInt:
					glUniform1ui(uniformInfo.Location, uniform.TypeData.UInt);
					break;
				case VarType::Bool:
					glUniform1i(uniformInfo.Location, uniform.TypeData.Bool ? 1 : 0);
					break;
				case VarType::Vec2:
					glUniform2f(uniformInfo.Location, uniform.TypeData.Vec2.x, uniform.TypeData.Vec2.y);
					break;
				case VarType::Vec3:
					glUniform3f(uniformInfo.Location, uniform.TypeData.Vec3.x, uniform.TypeData.Vec3.y, uniform.TypeData.Vec3.z);
					break;
				case VarType::Vec4:
					glUniform4f(uniformInfo.Location, uniform.TypeData.Vec4.x, uniform.TypeData.Vec4.y, uniform.TypeData.Vec4.z, uniform.TypeData.Vec4.w);
					break;
				case VarType::IVec2:
					glUniform2i(uniformInfo.Location, uniform.TypeData.IVec2.x, uniform.TypeData.IVec2.y);
					break;
				case VarType::IVec3:
					glUniform3i(uniformInfo.Location, uniform.TypeData.IVec3.x, uniform.TypeData.IVec3.y, uniform.TypeData.IVec3.z);
					break;
				case VarType::IVec4:
					glUniform4i(uniformInfo.Location, uniform.TypeData.IVec4.x, uniform.TypeData.IVec4.y, uniform.TypeData.IVec4.z, uniform.TypeData.IVec4.w);
					break;
				case VarType::UIVec2:
					glUniform2ui(uniformInfo.Location, uniform.TypeData.UIVec2.x, uniform.TypeData.UIVec2.y);
					break;
				case VarType::UIVec3:
					glUniform3ui(uniformInfo.Location, uniform.TypeData.UIVec3.x, uniform.TypeData.UIVec3.y, uniform.TypeData.UIVec3.z);
					break;
				case VarType::UIVec4:
					glUniform4ui(uniformInfo.Location, uniform.TypeData.UIVec4.x, uniform.TypeData.UIVec4.y, uniform.TypeData.UIVec4.z, uniform.TypeData.UIVec4.w);
					break;

				/*case VarType::BoolVec2:
					break;
				case VarType::BoolVec3:
					break;
				case VarType::BoolVec4:
					break;*/

				case VarType::Mat4x4:
					glUniformMatrix4fv(uniformInfo.Location, 1, false, glm::value_ptr(uniform.TypeData.Mat4x4));
					break;
				case VarType::Mat3x3:
					glUniformMatrix3fv(uniformInfo.Location, 1, false, glm::value_ptr(uniform.TypeData.Mat3x3));
					break;

				/*case VarType::Mat2x3:
					break;
				case VarType::Mat2x4:
					break;
				case VarType::Mat3x2:
					break;
				case VarType::Mat3x4:
					break;
				case VarType::Mat4x2:
					break;
				case VarType::Mat4x3:
					break;*/

				case VarType::Unknown:
					break;
				default:
					break;
			}
		}
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

		SetViewPort(state.ViewPort);
	}

	FrameBuffer_ptr GLRenderDevice::GetCachedFrameBuffer(const DrawCallState &state)
	{
		if (!state.HasAnyRenderTarget) {
			return nullptr;
		}

		FrameBufferKey key = {
			state.RenderTargets, state.DepthTarget, state.DepthIndex, state.DepthMipSlice
		};

		auto it = m_CachedFrameBuffers.find(key);

		if (it != m_CachedFrameBuffers.end())
		{
			return it->second;
		}

		auto framebuffer = std::make_shared<FrameBuffer>();

		glGenFramebuffers(1, &framebuffer->Handle);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->Handle);

		std::string fbName = "FB(A=";

		for (uint32_t rt = 0; rt < DrawCallState::MaxRenderTargets; rt++)
		{
			const auto& targetBinding = state.RenderTargets[rt];

			if (targetBinding.Texture == nullptr) {
				continue;
			}

			auto glTex = GetTexture(state.RenderTargets[rt].Texture);

			framebuffer->RenderTargets[rt] = state.RenderTargets[rt].Texture;
			glTex->m_UsedInFrameBuffers = true;

			fbName.append(glTex->GetDesc().Name + ",");

			if (targetBinding.Index == ~0u || glTex->GetDesc().DepthOrArraySize == 0)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + rt, glTex->BindTarget(), (glTex->EnabledBindSRGB && glTex->GetDesc().ImageFormat == GraphicsFormat::SRGBA8_UNORM) ? glTex->SRGBHandle() : glTex->Handle(), GLint(targetBinding.MipSlice));
			}
			else
			{
				glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + rt, GL_TEXTURE_CUBE_MAP_POSITIVE_X + targetBinding.Index, glTex->Handle(), GLint(targetBinding.MipSlice));
			}

			GLint encoding=-1;
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + rt, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &encoding);
			if (encoding == GL_LINEAR)
				AU_LOG_INFO("Framebuffer attachment ", targetBinding.Texture->GetDesc().Name, " (", rt, ") color encoding is linear.");
			if (encoding == GL_SRGB)
				AU_LOG_INFO("Framebuffer attachment ", targetBinding.Texture->GetDesc().Name, " (", rt, ") color encoding is sRGB.");

			//glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + rt, renderState.targets[rt]->handle, renderState.targetMipSlices[rt], renderState.targetIndicies[rt]);

			framebuffer->DrawBuffers[(framebuffer->NumBuffers)++] = GL_COLOR_ATTACHMENT0 + rt;
		}

		if (state.DepthTarget)
		{
			auto glDepthTex = GetTexture(state.DepthTarget);

			fbName.append("D=" + glDepthTex->GetDesc().Name);

			framebuffer->DepthTarget = glDepthTex;
			glDepthTex->m_UsedInFrameBuffers = true;

			GLenum attachment;

			if (state.DepthTarget->GetDesc().ImageFormat == GraphicsFormat::D24S8)
				attachment = GL_DEPTH_STENCIL_ATTACHMENT;
			else
				attachment = GL_DEPTH_ATTACHMENT;

			if (state.DepthTarget->GetDesc().DepthOrArraySize == 0)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, glDepthTex->BindTarget(), glDepthTex->Handle(), GLint(state.DepthMipSlice));
			}
			else
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, glDepthTex->Handle(), GLint(state.DepthMipSlice), GLint(state.DepthIndex));
			}
		}
		else
		{
			fbName = fbName.substr(0, fbName.length() - 1);
		}

		fbName.append(")");

		CHECK_GL_ERROR();

		framebuffer->Name = fbName;

		AU_LOG_INFO("New FB(", framebuffer->Handle, "): ", fbName);
		glObjectLabel(GL_FRAMEBUFFER, framebuffer->Handle, static_cast<GLsizei>(fbName.size()), fbName.c_str());

		uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			AU_LOG_ERROR("Incomplete framebuffer!");
		}


		glBindFramebuffer(GL_FRAMEBUFFER, m_CurrentFrameBuffer != nullptr ? m_CurrentFrameBuffer->Handle : 0);

		m_CachedFrameBuffers[key] = framebuffer;

		return framebuffer;
	}

	void GLRenderDevice::NotifyTextureDestroy(GLTexture* texture)
	{
		std::erase_if(m_CachedFrameBuffers, [this, texture](auto& kv) -> bool
		{
			const FrameBuffer_ptr& fb = kv.second;

			if (fb->DepthTarget == texture)
			{
				if (m_CurrentFrameBuffer == fb) {
					m_CurrentFrameBuffer = nullptr;
				}

				AU_LOG_INFO("Destroyed FB: ", fb->Name);
				return true;
			}

			for (const auto& rt : fb->RenderTargets)
			{
				if (texture != rt) {
					continue;
				}

				if (m_CurrentFrameBuffer == fb) {
					m_CurrentFrameBuffer = nullptr;
				}

				AU_LOG_INFO("Destroyed FB: ", fb->Name);
				return true;
			}

			return false;
		});
	}

	void GLRenderDevice::NotifyBufferDestroy(class GLBuffer* buffer)
	{
		std::vector<VaoKey> vaosToRemove;

		for (const auto& [key, vao]: m_CachedVaos)
		{
			for (const auto& [location, cachedBuffer]: key.Buffers)
			{
				if (buffer == cachedBuffer.get())
				{
					vaosToRemove.push_back(key);
					continue;
				}
			}
		}

		for (const auto& item: vaosToRemove)
		{
			m_CachedVaos.erase(item);
		}
	}

	void GLRenderDevice::SetBlendState(const FBlendState& state)
	{
		if (state.AlphaToCoverage)
		{
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		}

		// TODO: Finish proper blending

		if (state.Enabled)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			glDisable(GL_BLEND);
		}

		/*for (uint32_t i = 0; i < targetCount; ++i)
		{
			if (blendState.blendEnable[i])
				glEnablei(GL_BLEND, i);

			uint32_t BlendOpRGB = convertBlendOp(blendState.blendOp[i]);
			uint32_t BlendOpAlpha = convertBlendOp(blendState.blendOpAlpha[i]);
			glBlendEquationSeparatei(i, BlendOpRGB, BlendOpAlpha);

			uint32_t SrcBlendRGB = convertBlendValue(blendState.srcBlend[i]);
			uint32_t DstBlendRGB = convertBlendValue(blendState.destBlend[i]);
			uint32_t SrcBlendAlpha = convertBlendValue(blendState.srcBlendAlpha[i]);
			uint32_t DstBlendAlpha = convertBlendValue(blendState.destBlendAlpha[i]);
			glBlendFuncSeparatei(i, SrcBlendRGB, DstBlendRGB, SrcBlendAlpha, DstBlendAlpha);

			glColorMaski(i,
				(blendState.colorWriteEnable[i] & BlendState::COLOR_MASK_RED) != 0,
				(blendState.colorWriteEnable[i] & BlendState::COLOR_MASK_GREEN) != 0,
				(blendState.colorWriteEnable[i] & BlendState::COLOR_MASK_BLUE) != 0,
				(blendState.colorWriteEnable[i] & BlendState::COLOR_MASK_ALPHA) != 0);
		}*/
	}

	void GLRenderDevice::SetRasterState(const FRasterState& rasterState)
	{
		// FIXME: This do not works if some gl call from ImGui set some state, find a fix here!

		if (m_LastRasterState.FillMode != rasterState.FillMode || true)
		{
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
			m_LastRasterState.FillMode = rasterState.FillMode;
		}

		if (m_LastRasterState.CullMode != rasterState.CullMode)
		{
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
			m_LastRasterState.CullMode = rasterState.CullMode;
		}

		if (m_LastRasterState.FrontCounterClockwise != rasterState.FrontCounterClockwise || true)
		{
			glFrontFace(rasterState.FrontCounterClockwise ? GL_CCW : GL_CW);
			m_LastRasterState.FrontCounterClockwise = rasterState.FrontCounterClockwise;
		}

		if (m_LastRasterState.DepthClipEnable)
		{
			if (rasterState.DepthClipEnable)
			{
				glEnable(GL_DEPTH_CLAMP);
			}
			else
			{
				glDisable(GL_DEPTH_CLAMP);
			}
			m_LastRasterState.DepthClipEnable = rasterState.DepthClipEnable;
		}

		if (m_LastRasterState.ScissorEnable != rasterState.ScissorEnable)
		{
			if (rasterState.ScissorEnable)
			{
				glEnable(GL_SCISSOR_TEST);
			}
			else
			{
				glDisable(GL_SCISSOR_TEST);
			}
			m_LastRasterState.ScissorEnable = rasterState.ScissorEnable;
		}

		if (m_LastRasterState.DepthBias != rasterState.DepthBias || m_LastRasterState.SlopeScaledDepthBias != rasterState.SlopeScaledDepthBias)
		{
			if (rasterState.DepthBias != 0 || rasterState.SlopeScaledDepthBias != 0.f)
			{
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(rasterState.SlopeScaledDepthBias, float(rasterState.DepthBias));
			}

			m_LastRasterState.DepthBias = rasterState.DepthBias;
			m_LastRasterState.SlopeScaledDepthBias = rasterState.SlopeScaledDepthBias;
		}

		if (rasterState.MultisampleEnable != m_LastRasterState.MultisampleEnable)
		{
			m_LastRasterState.MultisampleEnable = rasterState.MultisampleEnable;

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

		if (rasterState.LineWidth != m_LastRasterState.LineWidth)
		{
			m_LastRasterState.LineWidth = rasterState.LineWidth;
			glLineWidth(rasterState.LineWidth);
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

		/*if (m_LastDepthState.DepthEnable != depthState.DepthEnable || m_LastDepthState.DepthWriteMask != depthState.DepthWriteMask || m_LastDepthState.DepthFunc != depthState.DepthFunc)
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

			m_LastDepthState.DepthEnable = depthState.DepthEnable;
			m_LastDepthState.DepthWriteMask = depthState.DepthWriteMask;
			m_LastDepthState.DepthFunc = depthState.DepthFunc;
		}*/

		if (m_LastDepthState.StencilEnable != depthState.StencilEnable)
		{
			//TODO: Other props needs to be checked too, but I think that we will never user stencil

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
			m_LastDepthState.StencilEnable = depthState.StencilEnable;
		}
	}

	void GLRenderDevice::Blit(const Texture_ptr &src, const Texture_ptr &dest)
	{
		GPU_DEBUG_SCOPE(dest != nullptr ? "Blit" : "BlitToBackBuffer");

		au_assert(src != nullptr);
		au_assert(src != dest);

		if(src->GetDesc().IsRenderTarget)
		{
			DrawCallState srcState;
			srcState.BindTarget(0, src);
			FrameBuffer_ptr srcFramebuffer = GetCachedFrameBuffer(srcState);

			if (m_CurrentFrameBuffer != srcFramebuffer)
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFramebuffer->Handle);
				m_CurrentFrameBuffer = srcFramebuffer;
			}
			//glDrawBuffers(GLsizei(srcFramebuffer->NumBuffers), srcFramebuffer->DrawBuffers);

			if(dest)
			{
				DrawCallState dstState;
				dstState.BindTarget(0, dest);
				FrameBuffer_ptr dstFramebuffer = GetCachedFrameBuffer(dstState);

				if (m_CurrentFrameBuffer != dstFramebuffer)
				{
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFramebuffer->Handle);
					m_CurrentFrameBuffer = dstFramebuffer;
				}
			}
			else if (m_CurrentFrameBuffer != nullptr)
			{
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				m_CurrentFrameBuffer = nullptr;
			}

			glBlitFramebuffer(0, 0, src->GetDesc().Width, src->GetDesc().Height, 0, 0, src->GetDesc().Width, src->GetDesc().Height,  GL_COLOR_BUFFER_BIT, GL_NEAREST);

			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//m_CurrentFrameBuffer = nullptr;

			return;
		}

		// This not works on: Intel(R) UHD Graphics 630
		/*SetShader(m_BlitShader);

		FRasterState rasterState = m_LastRasterState;
		rasterState.CullMode = ECullMode::None;
		SetRasterState(rasterState);

		FDepthStencilState depthStencilState = m_LastDepthState;
		depthStencilState.DepthEnable = false;
		SetDepthStencilState(depthStencilState);

		DrawCallState drawCallState;
		if (dest != nullptr)
		{
			au_assert(dest->GetDesc().IsRenderTarget == true);

			drawCallState.ViewPort = dest->GetDesc().GetSize();
			drawCallState.BindTarget(0, dest);
		}
		else
		{
			drawCallState.ViewPort = src->GetDesc().GetSize();
		}

		BindRenderTargets(drawCallState);

		GLTexture* glSrc = GetTexture(src);
		m_ContextState.BindTexture(0, glSrc);
		m_ContextState.BindSampler(0, nullptr);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		//glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 1);
		CHECK_GL_ERROR();*/
	}

	void GLRenderDevice::SetViewPort(const FViewPort &wp)
	{
		au_assert(wp.Width > 0);
		au_assert(wp.Height > 0);

		if (wp != m_LastViewPort)
		{
			m_LastViewPort = wp;
			glViewport(wp.X, wp.Y, wp.Width, wp.Height);
			glScissor(wp.X, wp.Y, wp.Width, wp.Height);
		}
	}

	const FViewPort &GLRenderDevice::GetCurrentViewPort() const
	{
		return m_LastViewPort;
	}

	size_t GLRenderDevice::GetUsedGPUMemory()
	{
		if (m_GpuVendor == EGpuVendor::Nvidia)
		{
			GLint totalAvailableMemory;
			glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalAvailableMemory);

			GLint currentAvailableMemory;
			glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &currentAvailableMemory);
			return totalAvailableMemory - currentAvailableMemory;
		}

		return 0;
	}
}