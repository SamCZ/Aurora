#include "GLRenderDevice.hpp"

#include "GLConversions.hpp"

#include <algorithm>
#ifdef _WIN32
#include <Windows.h>
#include <wrl.h>
#endif

#include <cstdint>
#include <Aurora/Core/assert.hpp>
#include <Aurora/Core/String.hpp>
#include <Aurora/Core/Profiler.hpp>


#ifdef GLSLANG_COMPILER
#include <glslang/Public/ShaderLang.h>
#endif

namespace Aurora
{
#ifdef _WIN32
	inline bool GetSSE42Support()
	{
		int cpui[4];
		__cpuidex(cpui, 1, 0);
		return !!(cpui[2] & 0x100000);
	}

	static const bool CpuSupportsSSE42 = GetSSE42Support();
#endif

	static uint64_t CrcTable[256];

	class CrcHash
	{
	private:
		uint64_t m_crc;
	public:
		inline CrcHash() : m_crc(0)
		{
			uint64_t poly = 0xC96C5795D7870F42;

			for (int i = 0; i < 256; ++i)
			{
				uint64_t crc = i;

				for (uint32_t j = 0; j < 8; ++j)
				{
					// is current coefficient set?
					if (crc & 1)
					{
						// yes, then assume it gets zero'd (by implied x^64 coefficient of dividend)
						crc >>= 1;

						// and add rest of the divisor
						crc ^= poly;
					}
					else
					{
						// no? then move to next coefficient
						crc >>= 1;
					}
				}

				CrcTable[i] = crc;
			}
		}

		inline uint64_t Get()
		{
			return m_crc;
		}

#ifdef _WIN32_DISABLED
		template<size_t size> __forceinline void AddBytesSSE42(void* p)
		{
			static_assert(size % 4 == 0, "Size of hashable types must be multiple of 4");

			auto* data = (uint32_t*)p;

			const size_t numIterations = size / sizeof(uint32_t);
			for (size_t i = 0; i < numIterations; i++)
			{
				crc = _mm_crc32_u32(crc, data[i]);
			}
		}
#endif

		inline void AddBytes(char *p, uint64_t size)
		{
			for (uint64_t idx = 0; idx < size; idx++)
			{
				uint8_t index = p[idx] ^ m_crc;
				uint64_t lookup = CrcTable[index];

				m_crc >>= 8;
				m_crc ^= lookup;
			}
		}

		template<typename T>
		void Add(const T &value)
		{
#ifdef _WIN32_DISABLED
			if (CpuSupportsSSE42)
				AddBytesSSE42<sizeof(value)>((void*)&value);
			else
				AddBytes((char*)&value, sizeof(value));
#else
			AddBytes((char *) &value, sizeof(value));
#endif
		}
	};

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

	GLRenderDevice::GLRenderDevice()
	: IRenderDevice(),
	m_nVAO(0),
	m_nVAOEmpty(0),
	m_LastVao(0),
	m_ContextState(),
	m_LastRasterState(),
	m_LastDepthState(),
	m_LastViewPort(0, 0),
	  m_LastInputLayout(nullptr)
	{

	}

	GLRenderDevice::~GLRenderDevice()
	{
		glDeleteVertexArrays(1, &m_nVAO);
		glDeleteVertexArrays(1, &m_nVAOEmpty);
#ifdef GLSLANG_COMPILER
		glslang::FinalizeProcess();
#endif
	}

	void GLRenderDevice::Init()
	{
#ifdef GLSLANG_COMPILER
		glslang::InitializeProcess();
#endif
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

#ifdef GLSLANG_COMPILER
		AU_LOG_INFO("GLSL Version ", glslang::GetGlslVersionString());
#endif

		glDisable(GL_MULTISAMPLE);

		InvalidateState();

		SetRasterState(m_LastRasterState);
		SetDepthStencilState(m_LastDepthState);

		glFlush();
	}
#ifdef GLSLANG_COMPILER
	EShLanguage ShaderTypeToShLanguage(EShaderType ShaderType)
	{
		switch (ShaderType)
		{
			case EShaderType::Vertex:           return EShLangVertex;
			case EShaderType::Hull:             return EShLangTessControl;
			case EShaderType::Domain:           return EShLangTessEvaluation;
			case EShaderType::Geometry:         return EShLangGeometry;
			case EShaderType::Pixel:            return EShLangFragment;
			case EShaderType::Compute:          return EShLangCompute;
			case EShaderType::Amplification:    return EShLangTaskNV;
			case EShaderType::Mesh:             return EShLangMeshNV;
			case EShaderType::RayGen:          return EShLangRayGen;
			case EShaderType::RayMiss:         return EShLangMiss;
			case EShaderType::RayClosestHit:  return EShLangClosestHit;
			case EShaderType::RayAnyHit:      return EShLangAnyHit;
			case EShaderType::RayIntersection: return EShLangIntersect;
			case EShaderType::Callable:         return EShLangCallable;
			default:
				AU_LOG_FATAL("Unexpected shader type");
				return EShLangCount;
		}
	}

	TBuiltInResource InitResources()
	{
		TBuiltInResource Resources;

		Resources.maxLights                                 = 32;
		Resources.maxClipPlanes                             = 6;
		Resources.maxTextureUnits                           = 32;
		Resources.maxTextureCoords                          = 32;
		Resources.maxVertexAttribs                          = 64;
		Resources.maxVertexUniformComponents                = 4096;
		Resources.maxVaryingFloats                          = 64;
		Resources.maxVertexTextureImageUnits                = 32;
		Resources.maxCombinedTextureImageUnits              = 80;
		Resources.maxTextureImageUnits                      = 32;
		Resources.maxFragmentUniformComponents              = 4096;
		Resources.maxDrawBuffers                            = 32;
		Resources.maxVertexUniformVectors                   = 128;
		Resources.maxVaryingVectors                         = 8;
		Resources.maxFragmentUniformVectors                 = 16;
		Resources.maxVertexOutputVectors                    = 16;
		Resources.maxFragmentInputVectors                   = 15;
		Resources.minProgramTexelOffset                     = -8;
		Resources.maxProgramTexelOffset                     = 7;
		Resources.maxClipDistances                          = 8;
		Resources.maxComputeWorkGroupCountX                 = 65535;
		Resources.maxComputeWorkGroupCountY                 = 65535;
		Resources.maxComputeWorkGroupCountZ                 = 65535;
		Resources.maxComputeWorkGroupSizeX                  = 1024;
		Resources.maxComputeWorkGroupSizeY                  = 1024;
		Resources.maxComputeWorkGroupSizeZ                  = 64;
		Resources.maxComputeUniformComponents               = 1024;
		Resources.maxComputeTextureImageUnits               = 16;
		Resources.maxComputeImageUniforms                   = 8;
		Resources.maxComputeAtomicCounters                  = 8;
		Resources.maxComputeAtomicCounterBuffers            = 1;
		Resources.maxVaryingComponents                      = 60;
		Resources.maxVertexOutputComponents                 = 64;
		Resources.maxGeometryInputComponents                = 64;
		Resources.maxGeometryOutputComponents               = 128;
		Resources.maxFragmentInputComponents                = 128;
		Resources.maxImageUnits                             = 8;
		Resources.maxCombinedImageUnitsAndFragmentOutputs   = 8;
		Resources.maxCombinedShaderOutputResources          = 8;
		Resources.maxImageSamples                           = 0;
		Resources.maxVertexImageUniforms                    = 0;
		Resources.maxTessControlImageUniforms               = 0;
		Resources.maxTessEvaluationImageUniforms            = 0;
		Resources.maxGeometryImageUniforms                  = 0;
		Resources.maxFragmentImageUniforms                  = 8;
		Resources.maxCombinedImageUniforms                  = 8;
		Resources.maxGeometryTextureImageUnits              = 16;
		Resources.maxGeometryOutputVertices                 = 256;
		Resources.maxGeometryTotalOutputComponents          = 1024;
		Resources.maxGeometryUniformComponents              = 1024;
		Resources.maxGeometryVaryingComponents              = 64;
		Resources.maxTessControlInputComponents             = 128;
		Resources.maxTessControlOutputComponents            = 128;
		Resources.maxTessControlTextureImageUnits           = 16;
		Resources.maxTessControlUniformComponents           = 1024;
		Resources.maxTessControlTotalOutputComponents       = 4096;
		Resources.maxTessEvaluationInputComponents          = 128;
		Resources.maxTessEvaluationOutputComponents         = 128;
		Resources.maxTessEvaluationTextureImageUnits        = 16;
		Resources.maxTessEvaluationUniformComponents        = 1024;
		Resources.maxTessPatchComponents                    = 120;
		Resources.maxPatchVertices                          = 32;
		Resources.maxTessGenLevel                           = 64;
		Resources.maxViewports                              = 16;
		Resources.maxVertexAtomicCounters                   = 0;
		Resources.maxTessControlAtomicCounters              = 0;
		Resources.maxTessEvaluationAtomicCounters           = 0;
		Resources.maxGeometryAtomicCounters                 = 0;
		Resources.maxFragmentAtomicCounters                 = 8;
		Resources.maxCombinedAtomicCounters                 = 8;
		Resources.maxAtomicCounterBindings                  = 1;
		Resources.maxVertexAtomicCounterBuffers             = 0;
		Resources.maxTessControlAtomicCounterBuffers        = 0;
		Resources.maxTessEvaluationAtomicCounterBuffers     = 0;
		Resources.maxGeometryAtomicCounterBuffers           = 0;
		Resources.maxFragmentAtomicCounterBuffers           = 1;
		Resources.maxCombinedAtomicCounterBuffers           = 1;
		Resources.maxAtomicCounterBufferSize                = 16384;
		Resources.maxTransformFeedbackBuffers               = 4;
		Resources.maxTransformFeedbackInterleavedComponents = 64;
		Resources.maxCullDistances                          = 8;
		Resources.maxCombinedClipAndCullDistances           = 8;
		Resources.maxSamples                                = 4;
		Resources.maxMeshOutputVerticesNV                   = 256;
		Resources.maxMeshOutputPrimitivesNV                 = 512;
		Resources.maxMeshWorkGroupSizeX_NV                  = 32;
		Resources.maxMeshWorkGroupSizeY_NV                  = 1;
		Resources.maxMeshWorkGroupSizeZ_NV                  = 1;
		Resources.maxTaskWorkGroupSizeX_NV                  = 32;
		Resources.maxTaskWorkGroupSizeY_NV                  = 1;
		Resources.maxTaskWorkGroupSizeZ_NV                  = 1;
		Resources.maxMeshViewCountNV                        = 4;

		Resources.limits.nonInductiveForLoops                 = 1;
		Resources.limits.whileLoops                           = 1;
		Resources.limits.doWhileLoops                         = 1;
		Resources.limits.generalUniformIndexing               = 1;
		Resources.limits.generalAttributeMatrixVectorIndexing = 1;
		Resources.limits.generalVaryingIndexing               = 1;
		Resources.limits.generalSamplerIndexing               = 1;
		Resources.limits.generalVariableIndexing              = 1;
		Resources.limits.generalConstantMatrixVectorIndexing  = 1;

		return Resources;
	}

	class IncluderImpl : public ::glslang::TShader::Includer
	{
	public:
		IncluderImpl()
		{}

		// For the "system" or <>-style includes; search the "system" paths.
		virtual IncludeResult* includeSystem(const char* headerName,
		                                     const char* /*includerName*/,
		                                     size_t /*inclusionDepth*/)
		{
			std::cout << "Trying to include (System) " << headerName << std::endl;

			/*const char* test = "yo\nnasdsdfsdf";

			auto* pNewInclude =
					new IncludeResult{
							headerName,
							reinterpret_cast<const char*>(test),
							strlen(test),
							nullptr};

			return pNewInclude;*/
			return nullptr;
		}

		// For the "local"-only aspect of a "" include. Should not search in the
		// "system" paths, because on returning a failure, the parser will
		// call includeSystem() to look in the "system" locations.
		virtual IncludeResult* includeLocal(const char* headerName,
		                                    const char* includerName,
		                                    size_t      inclusionDepth)
		{
			std::cout << "Trying to include (Local) " << headerName << std::endl;
			return nullptr;
		}

		// Signals that the parser will no longer use the contents of the
		// specified IncludeResult.
		virtual void releaseInclude(IncludeResult* IncldRes)
		{
			delete IncldRes;
		}

	private:

	};
#endif
	Shader_ptr GLRenderDevice::CreateShaderProgram(const ShaderProgramDesc &desc)
	{
		CPU_DEBUG_SCOPE("CreateShaderProgram");

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

			std::string glslSourcePreprocessed;
			EShMessages        messages = (EShMessages)(EShMsgAST);
			{
				EShLanguage        ShLang = ShaderTypeToShLanguage(type);
				::glslang::TShader* Shader = new ::glslang::TShader(ShLang);

				Shader->setEnvInput(::glslang::EShSourceGlsl, ShLang, ::glslang::EShClientOpenGL, 450);
				Shader->setEnvClient(::glslang::EShClientOpenGL, ::glslang::EShTargetOpenGL_450);
				Shader->setEnvTarget(::glslang::EShTargetSpv, ::glslang::EShTargetSpv_1_0);
				Shader->setEntryPoint("main");
				//Shader->setSourceEntryPoint("main");


				const char* ShaderStrings[]       = {source.c_str()};
				const int   ShaderStringLengths[] = {static_cast<int>(source.length())};
				const char* Names[]               = {"yo"};
				Shader->setStringsWithLengthsAndNames(ShaderStrings, ShaderStringLengths, Names, 1);
				Shader->setAutoMapBindings(true);

				TBuiltInResource Resources = InitResources();
				IncluderImpl includer;
				if(!Shader->preprocess(&Resources, 100, ECoreProfile, false, true, messages, &glslSourcePreprocessed, includer))
				{
					AU_LOG_FATAL("Failed to preprocess shader: \n", Shader->getInfoLog(), Shader->getInfoDebugLog());
				}
			}

			std::string error;
			GLuint shaderID = CompileShaderRaw(glslSourcePreprocessed, type, &error);

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
		else if (desc.SampleCount > 1)
		{
			AU_LOG_FATAL("Multisample is not supported !");
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

		glObjectLabel(GL_TEXTURE, handle, static_cast<GLsizei>(desc.Name.size()), desc.Name.c_str());
		CHECK_GL_ERROR();

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

			glBindTexture(bindTarget, 0);
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

		if(desc.DimensionType == EDimensionType::TYPE_CubeMap)
		{
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + subresource, (GLint)mipLevel, 0, 0, (GLsizei)width, (GLsizei)height, glTexture->Format().BaseFormat, glTexture->Format().Type, data);
			CHECK_GL_ERROR();
		} else if (desc.DimensionType == EDimensionType::TYPE_2DArray || desc.DepthOrArraySize > 0)
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
		void* mappedData = nullptr;

		if(desc.Type != EBufferType::TextureBuffer) {
			glGenBuffers(1, &handle);
			glBindBuffer(bindTarget, handle);

			glObjectLabel(GL_BUFFER, handle, static_cast<GLsizei>(desc.Name.size()), desc.Name.c_str());

			//glBufferStorage(bindTarget, desc.ByteSize, data, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
			//mappedData = glMapBufferRange(bindTarget, 0, desc.ByteSize, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

			glBufferData(bindTarget, desc.ByteSize, data, usage);
			CHECK_GL_ERROR();

			glBindBuffer(bindTarget, GL_NONE);
		} else {
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
		CPU_DEBUG_SCOPE("GLRenderDevice::WriteBuffer")
		if(buffer == nullptr) {
			return;
		}

		auto* glBuffer = static_cast<GLBuffer*>(buffer.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

		glBindBuffer(glBuffer->BindTarget(), glBuffer->Handle());

		au_assert(glBuffer->GetDesc().ByteSize >= dataSize);

		if (dataSize > glBuffer->GetDesc().ByteSize)
			dataSize = glBuffer->GetDesc().ByteSize;

		//memcpy(glBuffer->m_MappedData, data, dataSize);
		//glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		glBufferSubData(glBuffer->BindTarget(), offset, GLsizeiptr(dataSize), data);
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

	void* GLRenderDevice::MapBuffer(const Buffer_ptr& buffer, EBufferAccess bufferAccess)
	{
		if(buffer == nullptr) {
			return nullptr;
		}
		auto* glBuffer = static_cast<GLBuffer*>(buffer.get()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
		glBindBuffer(glBuffer->BindTarget(), glBuffer->Handle());
		return glMapBuffer(glBuffer->BindTarget(), ConvertBufferAccess(bufferAccess));
		//GLvoid* pMappedData = glMapBufferRange(glBuffer->BindTarget(), 0, glBuffer->GetDesc().ByteSize, GL_MAP_WRITE_BIT);//GL_MAP_UNSYNCHRONIZED_BIT
		//return glBuffer->m_MappedData;
	}

	void GLRenderDevice::UnmapBuffer(const Buffer_ptr& buffer)
	{
		if(buffer == nullptr) {
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

	void GLRenderDevice::Draw(const DrawCallState &state, const std::vector<DrawArguments>& args)
	{
		if(state.Shader == nullptr) {
			AU_LOG_ERROR("Cannot draw without shader !");
			throw;
			return;
		}

		glBindVertexArray(m_nVAOEmpty); // FIXME: idk why, but when frustum clips all geometry and nothing renders,then this call happens, it will throw error in non bound Array (maybe it does NanoVG?)

		ApplyDrawCallState(state);

		GLenum primitiveType = ConvertPrimType(state.PrimitiveType);

		for(const auto& drawArg : args) {
			glDrawArraysInstanced(primitiveType, GLint(drawArg.StartVertexLocation), GLint(drawArg.VertexCount), GLint(drawArg.InstanceCount));
			CHECK_GL_ERROR();
		}
	}

	#define BUFFER_OFFSET(i) ((char *)NULL + (i))

	void GLRenderDevice::DrawIndexed(const DrawCallState &state, const std::vector<DrawArguments> &args)
	{
		if(state.IndexBuffer.Buffer == nullptr || state.Shader == nullptr) {
			AU_LOG_ERROR("Cannot draw with these arguments !");
			throw;
			return;
		}

		//ApplyDrawCallState(state);

		CHECK_GL_ERROR();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GetBuffer(state.IndexBuffer.Buffer)->Handle());

		GLenum primitiveType = ConvertPrimType(state.PrimitiveType);
		GLenum ibFormat = ConvertIndexBufferFormat(state.IndexBuffer.Format);

		for(const auto& drawArg : args) {
			if(drawArg.InstanceCount > 1) {
				//glDrawElementsInstancedBaseVertex(primitiveType, GLsizei(drawArg.VertexCount), ibFormat, (const void*)size_t(drawArg.StartIndexLocation), GLsizei(drawArg.InstanceCount), GLint(drawArg.StartVertexLocation));
				glDrawElementsInstanced(primitiveType, GLsizei(drawArg.VertexCount), ibFormat, BUFFER_OFFSET(drawArg.StartIndexLocation), GLsizei(drawArg.InstanceCount));
			} else {
				//glDrawElementsBaseVertex(primitiveType, GLsizei(drawArg.VertexCount), ibFormat, (const void*)size_t(drawArg.StartIndexLocation), GLint(drawArg.StartVertexLocation));
				glDrawElements(primitiveType, GLsizei(drawArg.VertexCount), ibFormat, BUFFER_OFFSET(drawArg.StartIndexLocation));
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

		if(state.InputLayoutHandle == m_LastInputLayout) return;
		m_LastInputLayout = state.InputLayoutHandle;

		for(const auto& var : inputVars) {
			uint8_t location = var.first;
			const ShaderInputVariable& inputVariable = var.second;
			VertexAttributeDesc layoutAttribute;

			/*if(!state.InputLayoutHandle->GetDescriptorBySemanticID(location, layoutAttribute)) {
				AU_LOG_FATAL("Input layout from DrawState is not supported by that in the shader !");
				continue;
			}*/

			if(!state.InputLayoutHandle->GetDescriptorByName(inputVariable.Name, layoutAttribute)) {
				AU_LOG_FATAL("Input layout from DrawState is not supported by that in the shader (" , inputVariable.Name, ") !");
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

			if(formatMapping.Type == GL_INT || formatMapping.Type == GL_UNSIGNED_INT || formatMapping.Type == GL_UNSIGNED_SHORT || formatMapping.Type == GL_SHORT) {
				glVertexAttribIPointer(
						GLuint(location),
						GLint(formatMapping.Components),
						formatMapping.Type,
						GLsizei(layoutAttribute.Stride),
						(const void*)size_t(layoutAttribute.Offset));
			} else {
				glVertexAttribPointer(
						GLuint(location),
						GLint(formatMapping.Components),
						formatMapping.Type,
						layoutAttribute.Normalized ? GL_TRUE : GL_FALSE,
						GLsizei(layoutAttribute.Stride),
						(const void*)size_t(layoutAttribute.Offset));
			}

			//glVertexAttribDivisor(GLuint(location), layoutAttribute.IsInstanced ? 1 : 0);
		}

		//glBindBuffer(GL_ARRAY_BUFFER, GL_NONE); Do not do this, android render will fail !

		if(m_LastInputLayout != nullptr)
		{
			if(inputVars.size() == m_LastInputLayout->GetDescriptors().size())
			{
				return;
			}
		}

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
				GLint layer = 0;

				switch (targetTextureBinding->Access) {
					case TextureBinding::EAccess::Read: access = GL_READ_ONLY; break;
					case TextureBinding::EAccess::Write: access = GL_WRITE_ONLY; break;
					case TextureBinding::EAccess::ReadAndWrite: access = GL_READ_WRITE; break;
				}

				m_ContextState.BindImage(imageResource.Binding, glTexture, targetTextureBinding->MipLevel, layered, layer, access, format);
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

			UniformBufferBinding uniformBinding;

			if(uniformBufferIt != state.BoundUniformBuffers.end()) {
				uniformBinding = uniformBufferIt->second;
			}

			GLBuffer* glBuffer = nullptr;

			if(uniformBinding.Size == 0 && uniformBinding.Buffer != nullptr)
			{
				uniformBinding.Size = uniformBinding.Buffer->GetDesc().ByteSize;
			}

			if(uniformBinding.Buffer != nullptr)
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

		assert(state.ViewPort.x > 0 && state.ViewPort.y > 0);

		if(state.ViewPort != m_LastViewPort)
		{
			m_LastViewPort = state.ViewPort;
			glViewport(0, 0, state.ViewPort.x, state.ViewPort.y);
		}
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

		std::string fbName = "Cached framebuffer " + std::to_string(hash);
		glObjectLabel(GL_FRAMEBUFFER, framebuffer->Handle, static_cast<GLsizei>(fbName.size()), fbName.c_str());

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
				AU_LOG_INFO("Framebuffer attachment ", targetBinding.Texture->GetDesc().Name, " (", rt, ") color encoding is linear.");
			if (encoding == GL_SRGB)
				AU_LOG_INFO("Framebuffer attachment ", targetBinding.Texture->GetDesc().Name, " (", rt, ") color encoding is sRGB.");

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

			if (state.DepthTarget->GetDesc().DepthOrArraySize == 0) {
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

		if(m_LastRasterState.FillMode != rasterState.FillMode)
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

		if(m_LastRasterState.CullMode != rasterState.CullMode)
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

		if(m_LastRasterState.FrontCounterClockwise != rasterState.FrontCounterClockwise)
		{
			glFrontFace(rasterState.FrontCounterClockwise ? GL_CCW : GL_CW);
			m_LastRasterState.FrontCounterClockwise = rasterState.FrontCounterClockwise;
		}

		if(m_LastRasterState.DepthClipEnable)
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

		if(m_LastRasterState.ScissorEnable != rasterState.ScissorEnable)
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

		if(m_LastRasterState.DepthBias != rasterState.DepthBias || m_LastRasterState.SlopeScaledDepthBias != rasterState.SlopeScaledDepthBias)
		{
			if (rasterState.DepthBias != 0 || rasterState.SlopeScaledDepthBias != 0.f)
			{
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(rasterState.SlopeScaledDepthBias, float(rasterState.DepthBias));
			}

			m_LastRasterState.DepthBias = rasterState.DepthBias;
			m_LastRasterState.SlopeScaledDepthBias = rasterState.SlopeScaledDepthBias;
		}

		if(rasterState.MultisampleEnable != m_LastRasterState.MultisampleEnable)
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
		if(m_LastDepthState.DepthEnable != depthState.DepthEnable || m_LastDepthState.DepthWriteMask != depthState.DepthWriteMask || m_LastDepthState.DepthFunc != depthState.DepthFunc)
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
		}

		if(m_LastDepthState.StencilEnable != depthState.StencilEnable)
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
}