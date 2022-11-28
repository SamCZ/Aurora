#include "GLShaderProgram.hpp"

#include <iostream>

namespace Aurora
{
	GLShaderProgram::GLShaderProgram(GLuint handle, ShaderProgramDesc desc)
	: m_Desc(std::move(desc)), m_Handle(handle), m_Resources(), m_HasInputLayout(false), m_InputVariables(), m_ConstantBufferDescriptorCacheInitialized(false), m_SamplerDescriptorCacheInitialized(false)
	{
		//std::cout << "Loading uniforms for " << m_Desc.GetName() << std::endl;
		m_Resources.LoadUniforms(m_Handle);

		if(m_Desc.HasShader(EShaderType::Vertex)) { // Get input attributes
			GLint attributeCount;
			glGetProgramiv(m_Handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

			GLint activeAttributesMaxLength = 0;
			glGetProgramiv(m_Handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &activeAttributesMaxLength);

			GLint size = 0;
			GLenum type = 0;
			GLint nameLen = 0;
			GLint location = 0;
			std::vector<GLchar> Name(activeAttributesMaxLength + 1);

			ShaderInputVariables_t inputVariables;

			for (int i = 0; i < attributeCount; ++i) {
				glGetActiveAttrib(m_Handle, i, activeAttributesMaxLength, &nameLen, &size, &type, Name.data());
				location = glGetAttribLocation(m_Handle, Name.data());

				if(location < 0) continue;

				std::string nameStr = Name.data();

				ShaderInputVariable inputVariable = {};
				inputVariable.Name = nameStr;
				inputVariable.Size = GetOpenGLDataTypeSize(type);
				inputVariable.Instanced = nameStr.find("_INSTANCED") != std::string::npos;
				inputVariable.SemanticIndex = location;

				switch (type) {
					case GL_FLOAT_VEC2:
						inputVariable.Format = GraphicsFormat::RG32_FLOAT;
						break;
					case GL_FLOAT_VEC3:
						inputVariable.Format = GraphicsFormat::RGB32_FLOAT;
						break;
					case GL_FLOAT_VEC4:
						inputVariable.Format = GraphicsFormat::RGBA32_FLOAT;
						break;
					case GL_INT_VEC4:
						inputVariable.Format = GraphicsFormat::RGBA32_UINT;
						break;
					case GL_UNSIGNED_INT:
						inputVariable.Format = GraphicsFormat::R32_UINT;
						break;
					case GL_FLOAT:
						inputVariable.Format = GraphicsFormat::R32_FLOAT;
						break;
					case GL_FLOAT_MAT4:
					{
						auto vec4Size = GetOpenGLDataTypeSize(GL_FLOAT_VEC4);
						assert(vec4Size == 16);
						inputVariables[location]     = {nameStr, vec4Size, GraphicsFormat::RGBA32_FLOAT, true, location + 0};
						inputVariables[location + 1] = {nameStr, vec4Size, GraphicsFormat::RGBA32_FLOAT, true, location + 1};
						inputVariables[location + 2] = {nameStr, vec4Size, GraphicsFormat::RGBA32_FLOAT, true, location + 2};
						inputVariables[location + 3] = {nameStr, vec4Size, GraphicsFormat::RGBA32_FLOAT, true, location + 3};
						continue;
					}
					default: {
						AU_LOG_ERROR("Unknown input format ", nameStr, " in program ", m_Desc.GetName());
						continue;
					}
				}

				inputVariables[location] = inputVariable;
			}

			if(!inputVariables.empty()) {
				m_HasInputLayout = true;
				m_InputVariables = inputVariables;
			}
		}
	}

	GLShaderProgram::~GLShaderProgram()
	{
		if(m_Handle) {
			glDeleteProgram(m_Handle);
		}
	}

	std::vector<ShaderResourceDesc> GLShaderProgram::GetResources(const ShaderResourceType& resourceType)
	{
		switch (resourceType)
		{
			case ShaderResourceType::Unknown:
				break;
			case ShaderResourceType::ConstantBuffer:
			{
				if(m_ConstantBufferDescriptorCacheInitialized)
				{
					return m_ConstantBufferDescriptorCache;
				}

				for(const auto& ub : m_Resources.GetUniformBlocks()) {
					ShaderResourceDesc resourceDesc = {};
					resourceDesc.Name = ub.Name;
					resourceDesc.Size = ub.Size;
					resourceDesc.Type = resourceType;
					resourceDesc.ArraySize = ub.ArraySize;
					resourceDesc.ShadersIn = ub.ShadersIn;
					resourceDesc.Variables = ub.Variables;

					m_ConstantBufferDescriptorCache.emplace_back(resourceDesc);
				}

				m_ConstantBufferDescriptorCacheInitialized = true;

				return m_ConstantBufferDescriptorCache;
			}
			case ShaderResourceType::Sampler:
			case ShaderResourceType::TextureSRV:
			{
				if(m_SamplerDescriptorCacheInitialized)
				{
					return m_SamplerDescriptorCache;
				}

				for(const auto& ub : m_Resources.GetSamplers()) {
					ShaderResourceDesc resourceDesc = {};
					resourceDesc.Name = ub.Name;
					resourceDesc.Size = 0;
					resourceDesc.Type = resourceType;
					resourceDesc.ArraySize = ub.ArraySize;
					resourceDesc.ShadersIn = EShaderType::Unknown;

					m_SamplerDescriptorCache.emplace_back(resourceDesc);
				}

				m_SamplerDescriptorCacheInitialized = true;

				return m_SamplerDescriptorCache;
			}
			case ShaderResourceType::BufferSRV:
				break;
			case ShaderResourceType::TextureUAV:
				break;
			case ShaderResourceType::BufferUAV:
				break;
			case ShaderResourceType::InputAttachment:
				break;
			case ShaderResourceType::AccelStruct:
				break;
		}

		return {};
	}

	GLint GLShaderProgram::GetUniformLocation(StrHashID nameID) const
	{
		auto it = m_Resources.GetUniforms().find(nameID);

		if (it == m_Resources.GetUniforms().end())
		{
			return -1;
		}

		return it->second.Location;
	}
}