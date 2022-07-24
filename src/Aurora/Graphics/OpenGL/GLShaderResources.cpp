#include "GLShaderResources.hpp"
#include <cstring>
#include <iostream>

#include "Aurora/Core/assert.hpp"
#include "GLShaderProgram.hpp"

#define GL_RESOURCE_LOG 0

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

	void GLShaderResources::LoadUniforms(GLuint program)
	{
		std::unordered_set<std::string> NamesPool;
		std::map<GLuint, BasicUniform> BasicUniforms;

		CHECK_GL_ERROR_AND_THROW("Unknown error");

		GLint lastUsedProgramID = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &lastUsedProgramID);
		CHECK_GL_ERROR_AND_THROW("Unable to get current program");

		glUseProgram(program);
		CHECK_GL_ERROR_AND_THROW("Unable to use program");

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



			au_assert(dataType > 0);

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

					if(std::string(Name.data()).find("_bindless") != std::string::npos)
					{
#if GL_RESOURCE_LOG
						AU_LOG_INFO("BINDLESS ", Name.data(), " - ", UniformLocation);
#endif
						continue;
					}

					// clang-format off
					const auto ResourceType =
							dataType == GL_SAMPLER_BUFFER     ||
							dataType == GL_INT_SAMPLER_BUFFER ||
							dataType == GL_UNSIGNED_INT_SAMPLER_BUFFER ?
							ShaderResourceType::BufferSRV :
							ShaderResourceType::TextureSRV;
					// clang-format on

					RemoveArrayBrackets(Name.data());

					for (GLint arr_ind = 0; arr_ind < size; ++arr_ind)
					{
						std::string strName = Name.data();

						if(size > 1)
						{
							strName += "[" + std::to_string(arr_ind) + "]";
						}

						m_Samplers.push_back(
							{*NamesPool.emplace(strName).first,
							 ResourceType,
							 m_SamplerBinding,
							 static_cast<uint32_t>(size),
							 UniformLocation,
							 dataType}
						);

						// glProgramUniform1i is not available in GLES3.0
						glUniform1i(UniformLocation + arr_ind, m_SamplerBinding);
						CHECK_GL_ERROR_ARG("Failed to set binding point for sampler uniform '", Name.data(), '\'');
						m_SamplerBinding++;
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
#if GL_RESOURCE_LOG
					std::cout << "Image: " << Name.data() << std::endl;
#endif
					m_Images.push_back(
							{
									*NamesPool.emplace(Name.data()).first,
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
					GLType glType = GetGLType(dataType);
					size_t varSize = glType.Size;

					GLint uniformLocation = glGetUniformLocation(program, namec);

					BasicUniform basicUniform = {};
					basicUniform.Name = Name.data();
					basicUniform.Size = varSize;
					basicUniform.ArraySize = size;

					std::string::size_type arrPos = basicUniform.Name.find('[');
					if (arrPos != std::string::npos)
					{
						basicUniform.Name = basicUniform.Name.substr(0, arrPos);
					}

					BasicUniforms[uniformIndex] = basicUniform;

					if (uniformLocation >= 0)
					{
						m_Uniforms[Hash_djb2(basicUniform.Name.c_str())] = {basicUniform.Name, ShaderResourceType::Uniform, 0, uint32_t(size), uniformLocation, glType.ComponentCount, glType.Size, GetGLVarType(dataType)};
					}

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
/*#ifdef DEBUG
						for (const auto& ub : UniformBlocks)
                        VERIFY(strcmp(ub.Name, Name.data()) != 0, "Uniform block with the name '", ub.Name, "' has already been enumerated");
#endif*/
					}
				}
			}

			if (IsNewBlock)
			{
				GLint dataSize;
				glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);
				GLint bindingLocal;
				glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_BINDING, &bindingLocal);
				GLint blockUniformCount;
				glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &blockUniformCount);
				std::vector<GLint> BlockUniformIndices(blockUniformCount);
				glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, BlockUniformIndices.data());
				std::vector<GLint> BlockUniformOffsets(blockUniformCount);
				glGetActiveUniformsiv(program, blockUniformCount, (GLuint*)BlockUniformIndices.data(), GL_UNIFORM_OFFSET, BlockUniformOffsets.data());

				static GLuint shaderReferences[6] = {
						GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER,
						GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER,
						GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER,
						GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER,
						GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER,
						GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER
				};

				static EShaderType eShaderTypes[6] = {
						EShaderType::Vertex,
						EShaderType::Geometry,
						EShaderType::Hull,
						EShaderType::Domain,
						EShaderType::Pixel,
						EShaderType::Compute
				};

				EShaderType shaderType = EShaderType::Unknown;

				for (int j = 0; j < 6; ++j) {
					GLint isReferenced = 0;
					glGetActiveUniformBlockiv(program, i, shaderReferences[j], &isReferenced);

					if(isReferenced) shaderType = eShaderTypes[j];
				}

				size_t blockSizeFromVars = 0;

				for (GLint j = 0; j < blockUniformCount; ++j) {
					const BasicUniform& uniform = BasicUniforms[BlockUniformIndices[j]];
					GLint offset = BlockUniformOffsets[j];
					blockSizeFromVars += uniform.Size * uniform.ArraySize;
				}
#if GL_RESOURCE_LOG
				std::cout << Name.data() << " : " << dataSize << "(" << blockSizeFromVars << ")" << std::endl;
#endif
				std::vector<ShaderVariable> shaderVariables;

				for (GLint j = 0; j < blockUniformCount; ++j) {
					const BasicUniform& uniform = BasicUniforms[BlockUniformIndices[j]];
					GLint offset = BlockUniformOffsets[j];

					ShaderVariable variable = {};
					variable.Name = uniform.Name;

					auto arrCharPos = variable.Name.find('[');
					if(arrCharPos != std::string::npos) {
						variable.Name = variable.Name.substr(0, arrCharPos);
					}

					auto dotCharPos = variable.Name.find('.');
					if(dotCharPos != std::string::npos) {
						variable.Name = variable.Name.substr(dotCharPos + 1);
					}

					bool foundDuplicate = false;
					for (auto& storedVar : shaderVariables)
					{
						if(storedVar.Name == variable.Name) {
							storedVar.Size += uniform.Size;
							storedVar.Offset = std::min<size_t>(storedVar.Offset, offset);
							foundDuplicate = true;
							break;
						}
					}

					if(!foundDuplicate) {
						variable.Size = uniform.Size * uniform.ArraySize;
						variable.Offset = offset;
						shaderVariables.emplace_back(variable);
					}
				}
#if GL_RESOURCE_LOG
				for (const auto& storedVar : shaderVariables)
				{
					std::cout << " - " << storedVar.Name << " - size " << storedVar.Size << " - offset " << storedVar.Offset << std::endl;
				}
#endif
				m_UniformBlocks.push_back({
												*NamesPool.emplace(Name.data()).first,
												ShaderResourceType::ConstantBuffer,
												m_UniformBufferBinding,
												static_cast<uint32_t>(ArraySize),
												UniformBlockIndex,
												shaderType,
												static_cast<size_t>(dataSize), // Dont use blockSizeFromVars, its going to cause weird crashed at destructing data vector
												shaderVariables
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
#if GL_RESOURCE_LOG
			std::cout << "Storage block: " << Name.data() << std::endl;
#endif
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
/*#    ifdef DEBUG
						for (const auto& sb : StorageBlocks)
                        VERIFY(strcmp(sb.Name, Name.data()) != 0, "Storage block with the name \"", sb.Name, "\" has already been enumerated");
#    endif*/
					}
				}
			}

			if (IsNewBlock)
			{
				m_StorageBlocks.push_back({
												*NamesPool.emplace(Name.data()).first,
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
