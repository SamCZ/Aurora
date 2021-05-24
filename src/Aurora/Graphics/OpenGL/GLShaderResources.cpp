#include "GLShaderResources.hpp"
#include <cstring>

#include "OpenGLShader.hpp"

namespace Aurora
{
	struct BasicUniform
	{
		GLuint Index{};
		std::string Name;
		GLenum Type{};
		GLint ArraySize{};
		size_t Size{};
	};

	GLShaderResources::GLShaderResources()
	: m_UniformBufferBinding(0),
	  m_SamplerBinding(0),
	  m_ImageBinding(0),
	  m_StorageBufferBinding(0),

	  m_UniformBlocks(),
	  m_Samplers(),
	  m_Images(),
	  m_StorageBlocks()
	{

	}

	void GLShaderResources::LoadUniforms(const ShaderType &shaderType, OpenGLShader* shaderHandle)
	{
		std::unordered_set<std::string> NamesPool;
		std::map<GLuint, BasicUniform> BasicUniforms;

		auto program = shaderHandle->Handle();

		GLint lastUsedProgramID = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &lastUsedProgramID);

		glUseProgram(program);

		GLint numActiveUniforms = 0;
		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
		CHECK_GL_ERROR_AND_THROW("Unable to get the number of active uniforms");

		// Query the maximum name length of the active uniform (including null terminator)
		GLint activeUniformMaxLength = 0;
		glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &activeUniformMaxLength);
		CHECK_GL_ERROR_AND_THROW("Unable to get the maximum uniform name length");

		GLint numActiveUniformBlocks = 0;
		glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &numActiveUniformBlocks);
		CHECK_GL_ERROR_AND_THROW("Unable to get the number of active uniform blocks");

		//
		// #### This parameter is currently unsupported by Intel OGL drivers.
		//
		// Query the maximum name length of the active uniform block (including null terminator)
		GLint activeUniformBlockMaxLength = 0;
		// On Intel driver, this call might fail:
		glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &activeUniformBlockMaxLength);
		//CHECK_GL_ERROR_AND_THROW("Unable to get the maximum uniform block name length\n");
		if (glGetError() != GL_NO_ERROR)
		{
			AU_LOG_WARNING("Unable to get the maximum uniform block name length. Using 1024 as a workaround");
			activeUniformBlockMaxLength = 1024;
		}

		auto MaxNameLength = std::max(activeUniformMaxLength, activeUniformBlockMaxLength);

#if GL_ARB_program_interface_query
		GLint numActiveShaderStorageBlocks = 0;
		if (glGetProgramInterfaceiv)
		{
			glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &numActiveShaderStorageBlocks);
			CHECK_GL_ERROR_AND_THROW("Unable to get the number of shader storage blocks blocks\n");

			// Query the maximum name length of the active shader storage block (including null terminator)
			GLint MaxShaderStorageBlockNameLen = 0;
			glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &MaxShaderStorageBlockNameLen);
			CHECK_GL_ERROR_AND_THROW("Unable to get the maximum shader storage block name length\n");
			MaxNameLength = std::max(MaxNameLength, MaxShaderStorageBlockNameLen);
		}
#endif

		MaxNameLength = std::max(MaxNameLength, 512);
		std::vector<GLchar> Name(MaxNameLength + 1);

		for (int i = 0; i < numActiveUniforms; i++)
		{
			GLenum dataType = 0;
			GLint  size     = 0;
			GLint  NameLen  = 0;

			glGetActiveUniform(program, i, MaxNameLength, &NameLen, &size, &dataType, Name.data());
			CHECK_GL_ERROR_AND_THROW("Unable to get active uniform");

			switch (dataType)
			{
				case GL_SAMPLER_1D:
				case GL_SAMPLER_2D:
				case GL_SAMPLER_3D:
				case GL_SAMPLER_CUBE:
				case GL_SAMPLER_1D_SHADOW:
				case GL_SAMPLER_2D_SHADOW:

				case GL_SAMPLER_1D_ARRAY:
				case GL_SAMPLER_2D_ARRAY:
				case GL_SAMPLER_1D_ARRAY_SHADOW:
				case GL_SAMPLER_2D_ARRAY_SHADOW:
				case GL_SAMPLER_CUBE_SHADOW:

				case GL_INT_SAMPLER_1D:
				case GL_INT_SAMPLER_2D:
				case GL_INT_SAMPLER_3D:
				case GL_INT_SAMPLER_CUBE:
				case GL_INT_SAMPLER_1D_ARRAY:
				case GL_INT_SAMPLER_2D_ARRAY:
				case GL_UNSIGNED_INT_SAMPLER_1D:
				case GL_UNSIGNED_INT_SAMPLER_2D:
				case GL_UNSIGNED_INT_SAMPLER_3D:
				case GL_UNSIGNED_INT_SAMPLER_CUBE:
				case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
				case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:

				case GL_SAMPLER_CUBE_MAP_ARRAY:
				case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
				case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
				case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:

				case GL_SAMPLER_2D_MULTISAMPLE:
				case GL_INT_SAMPLER_2D_MULTISAMPLE:
				case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
				case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
				case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
				case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:

				case GL_SAMPLER_BUFFER:
				case GL_INT_SAMPLER_BUFFER:
				case GL_UNSIGNED_INT_SAMPLER_BUFFER:
				{
					auto UniformLocation = glGetUniformLocation(program, Name.data());
					// Note that glGetUniformLocation(program, name) is equivalent to
					// glGetProgramResourceLocation(program, GL_UNIFORM, name);
					// The latter is only available in GL 4.4 and GLES 3.1

					// clang-format off
					const auto ResourceType =
							dataType == GL_SAMPLER_BUFFER     ||
							dataType == GL_INT_SAMPLER_BUFFER ||
							dataType == GL_UNSIGNED_INT_SAMPLER_BUFFER ?
							ShaderResourceType::BufferSRV :
							ShaderResourceType::TextureSRV;
					// clang-format on

					RemoveArrayBrackets(Name.data());

					m_Samplers.push_back(
							{NamesPool.emplace(Name.data()).first->c_str(),
							 shaderType,
							 ResourceType,
							 m_SamplerBinding,
							 static_cast<uint32_t>(size),
							 UniformLocation,
							 dataType}
					);

					for (GLint arr_ind = 0; arr_ind < size; ++arr_ind)
					{
						// glProgramUniform1i is not available in GLES3.0
						glUniform1i(UniformLocation + arr_ind, m_SamplerBinding++);
						CHECK_GL_ERROR_ARG("Failed to set binding point for sampler uniform '", Name.data(), '\'');
					}

					break;
				}

#if GL_ARB_shader_image_load_store
				case GL_IMAGE_1D:
				case GL_IMAGE_2D:
				case GL_IMAGE_3D:
				case GL_IMAGE_2D_RECT:
				case GL_IMAGE_CUBE:
				case GL_IMAGE_BUFFER:
				case GL_IMAGE_1D_ARRAY:
				case GL_IMAGE_2D_ARRAY:
				case GL_IMAGE_CUBE_MAP_ARRAY:
				case GL_IMAGE_2D_MULTISAMPLE:
				case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
				case GL_INT_IMAGE_1D:
				case GL_INT_IMAGE_2D:
				case GL_INT_IMAGE_3D:
				case GL_INT_IMAGE_2D_RECT:
				case GL_INT_IMAGE_CUBE:
				case GL_INT_IMAGE_BUFFER:
				case GL_INT_IMAGE_1D_ARRAY:
				case GL_INT_IMAGE_2D_ARRAY:
				case GL_INT_IMAGE_CUBE_MAP_ARRAY:
				case GL_INT_IMAGE_2D_MULTISAMPLE:
				case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
				case GL_UNSIGNED_INT_IMAGE_1D:
				case GL_UNSIGNED_INT_IMAGE_2D:
				case GL_UNSIGNED_INT_IMAGE_3D:
				case GL_UNSIGNED_INT_IMAGE_2D_RECT:
				case GL_UNSIGNED_INT_IMAGE_CUBE:
				case GL_UNSIGNED_INT_IMAGE_BUFFER:
				case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
				case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
				case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
				case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
				case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
				{
					auto UniformLocation = glGetUniformLocation(program, Name.data());

					// clang-format off
					const auto ResourceType =
							dataType == GL_IMAGE_BUFFER     ||
							dataType == GL_INT_IMAGE_BUFFER ||
							dataType == GL_UNSIGNED_INT_IMAGE_BUFFER ?
							ShaderResourceType::BufferUAV :
							ShaderResourceType::TextureUAV;
					// clang-format on

					RemoveArrayBrackets(Name.data());

					m_Images.push_back(
							{
									NamesPool.emplace(Name.data()).first->c_str(),
									shaderType,
									ResourceType,
									m_ImageBinding,
									static_cast<uint32_t>(size),
									UniformLocation,
									dataType
							}
					);

					for (GLint arr_ind = 0; arr_ind < size; ++arr_ind)
					{
						// glUniform1i for image uniforms is not supported in at least GLES3.2.
						// glProgramUniform1i is not available in GLES3.0
						glUniform1i(UniformLocation + arr_ind, m_ImageBinding);
						if (glGetError() != GL_NO_ERROR)
						{
							if (size > 1)
							{
								AU_LOG_WARNING("Failed to set binding for image uniform '", Name.data(), "'[", arr_ind,
											   "]. Expected binding: ", m_ImageBinding,
											   ". Make sure that this binding is explicitly assigned in shader source code."
											   " Note that if the source code is converted from HLSL and if images are only used"
											   " by a single shader stage, then bindings automatically assigned by HLSL->GLSL"
											   " converter will work fine.");
							}
							else
							{
								AU_LOG_WARNING("Failed to set binding for image uniform '", Name.data(), "'."
																										 " Expected binding: ",
											   m_ImageBinding,
											   ". Make sure that this binding is explicitly assigned in shader source code."
											   " Note that if the source code is converted from HLSL and if images are only used"
											   " by a single shader stage, then bindings automatically assigned by HLSL->GLSL"
											   " converter will work fine.");
							}
						}
						++m_ImageBinding;
					}

					break;
				}
#endif
				default: {
					GLuint uniformIndex;
					const GLchar* namec = Name.data();
					glGetUniformIndices(program, 1, &namec, &uniformIndex);
					size_t varSize = GetOpenGLDataTypeSize(dataType);

					BasicUniform basicUniform = {};
					basicUniform.Name = Name.data();
					basicUniform.Size = varSize;
					basicUniform.ArraySize = size;

					BasicUniforms[uniformIndex] = basicUniform;

					break;
				}
			}
		}

		for (int i = 0; i < numActiveUniformBlocks; i++)
		{
			// In contrast to shader uniforms, every element in uniform block array is enumerated individually
			GLsizei NameLen = 0;
			glGetActiveUniformBlockName(program, i, MaxNameLength, &NameLen, Name.data());
			CHECK_GL_ERROR_AND_THROW("Unable to get active uniform block name\n");

			auto UniformBlockIndex = glGetUniformBlockIndex(program, Name.data());
			CHECK_GL_ERROR_AND_THROW("Unable to get active uniform block index\n");

			bool IsNewBlock = true;

			GLint ArraySize     = 1;
			auto* OpenBacketPtr = strchr(Name.data(), '[');
			if (OpenBacketPtr != nullptr)
			{
				auto Ind       = atoi(OpenBacketPtr + 1);
				ArraySize      = std::max(ArraySize, Ind + 1);
				*OpenBacketPtr = 0;
				if (!m_UniformBlocks.empty())
				{
					// Look at previous uniform block to check if it is the same array
					auto& LastBlock = m_UniformBlocks.back();
					if (strcmp(LastBlock.Name.c_str(), Name.data()) == 0)
					{
						ArraySize = std::max(ArraySize, static_cast<GLint>(LastBlock.ArraySize));
						//VERIFY(UniformBlockIndex == LastBlock.UBIndex + Ind, "Uniform block indices are expected to be continuous");
						LastBlock.ArraySize = ArraySize;
						IsNewBlock          = false;
					}
					else
					{
#ifdef DEBUG
						for (const auto& ub : UniformBlocks)
                        VERIFY(strcmp(ub.Name, Name.data()) != 0, "Uniform block with the name '", ub.Name, "' has already been enumerated");
#endif
					}
				}
			}

			if (IsNewBlock)
			{
				GLint dataSize;
				glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);
				GLint blockUniformCount;
				glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &blockUniformCount);
				std::vector<GLint> BlockUniformIndices(blockUniformCount);
				glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, BlockUniformIndices.data());
				std::vector<GLint> BlockUniformOffsets(blockUniformCount);
				glGetActiveUniformsiv(program, blockUniformCount, (GLuint*)BlockUniformIndices.data(), GL_UNIFORM_OFFSET, BlockUniformOffsets.data());

				for (GLint j = 0; j < blockUniformCount; ++j) {
					const BasicUniform& uniform = BasicUniforms[BlockUniformIndices[j]];
					GLint offset = BlockUniformOffsets[j];
				}

				m_UniformBlocks.push_back({
												NamesPool.emplace(Name.data()).first->c_str(),
												shaderType,
												ShaderResourceType::ConstantBuffer,
												m_UniformBufferBinding,
												static_cast<uint32_t>(ArraySize),
												UniformBlockIndex
										});
			}

			glUniformBlockBinding(program, UniformBlockIndex, m_UniformBufferBinding++);
			CHECK_GL_ERROR_ARG("glUniformBlockBinding() failed");
		}

#if GL_ARB_shader_storage_buffer_object
		for (int i = 0; i < numActiveShaderStorageBlocks; ++i)
		{
			GLsizei Length = 0;
			glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, i, MaxNameLength, &Length, Name.data());
			CHECK_GL_ERROR_AND_THROW("Unable to get shader storage block name\n");
			//VERIFY(Length < MaxNameLength && static_cast<size_t>(Length) == strlen(Name.data()), "Incorrect shader storage block name");

			auto SBIndex = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, Name.data());
			CHECK_GL_ERROR_AND_THROW("Unable to get shader storage block index\n");

			//std::cout << "Storage block: " << Name.data() << std::endl;

			bool  IsNewBlock    = true;
			int32_t ArraySize     = 1;
			auto* OpenBacketPtr = strchr(Name.data(), '[');
			if (OpenBacketPtr != nullptr)
			{
				auto Ind       = atoi(OpenBacketPtr + 1);
				ArraySize      = std::max(ArraySize, Ind + 1);
				*OpenBacketPtr = 0;
				if (!m_StorageBlocks.empty())
				{
					// Look at previous storage block to check if it is the same array
					auto& LastBlock = m_StorageBlocks.back();
					if (std::strcmp(LastBlock.Name.c_str(), Name.data()) == 0)
					{
						ArraySize = std::max(ArraySize, static_cast<GLint>(LastBlock.ArraySize));
						//VERIFY(static_cast<GLint>(SBIndex) == LastBlock.SBIndex + Ind, "Storage block indices are expected to be continuous");
						LastBlock.ArraySize = ArraySize;
						IsNewBlock          = false;
					}
					else
					{
#    ifdef DEBUG
						for (const auto& sb : StorageBlocks)
                        VERIFY(strcmp(sb.Name, Name.data()) != 0, "Storage block with the name \"", sb.Name, "\" has already been enumerated");
#    endif
					}
				}
			}

			if (IsNewBlock)
			{
				m_StorageBlocks.push_back({
												NamesPool.emplace(Name.data()).first->c_str(),
												shaderType,
												ShaderResourceType::BufferUAV,
												m_StorageBufferBinding,
												static_cast<uint32_t>(ArraySize),
												SBIndex
										});
			}

			if (glShaderStorageBlockBinding)
			{
				glShaderStorageBlockBinding(program, SBIndex, m_StorageBufferBinding);
				CHECK_GL_ERROR_ARG("glShaderStorageBlockBinding() failed");
			}
			else
			{
				AU_LOG_WARNING("glShaderStorageBlockBinding is not available on this device and "
							   "the engine is unable to automatically assign shader storage block bindindg for '",
							   Name.data(), "' variable. Expected binding: ", m_StorageBufferBinding,
							   ". Make sure that this binding is explicitly assigned in shader source code."
							   " Note that if the source code is converted from HLSL and if storage blocks are only used"
							   " by a single shader stage, then bindings automatically assigned by HLSL->GLSL"
							   " converter will work fine.");
			}
			++m_StorageBufferBinding;
		}
#endif

		glUseProgram(lastUsedProgramID);
	}
}
