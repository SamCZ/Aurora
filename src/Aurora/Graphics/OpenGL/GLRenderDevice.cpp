#include "GLRenderDevice.hpp"
#include "Aurora/Logger/Logger.hpp"
#include "GLShaderProgram.hpp"

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

}