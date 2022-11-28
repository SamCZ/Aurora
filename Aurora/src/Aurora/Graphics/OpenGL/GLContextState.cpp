#include "GLContextState.hpp"
#include "GLUtils.hpp"

namespace Aurora
{
	GLContextState::GLContextState() : m_ActiveTexture(-1), m_LastShaderHandle(0)
	{

	}

	void GLContextState::Invalidate()
	{
#if !PLATFORM_ANDROID
		// On Android this results in OpenGL error, so we will not
		// clear the barriers. All the required barriers will be
		// executed next frame when needed
		if (m_PendingMemoryBarriers != 0) {
			//EnsureMemoryBarrier(m_PendingMemoryBarriers);
		}
		m_PendingMemoryBarriers = 0;
#endif

		glUseProgram(0);
		//glBindProgramPipeline(0);
		//glBindVertexArray(0);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		//glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		m_ActiveTexture = -1;
		m_LastShaderHandle = -1;

		m_BoundTextures.clear();
		m_BoundSamplers.clear();
		m_BoundImages.clear();
		m_BoundUniformBuffers.clear();
		m_BoundStorageBlocks.clear();
	}

	template <typename ObjectType>
	bool UpdateBoundObject(UniqueIdentifier& CurrentObjectID, ObjectType* NewObject, GLuint& NewGLHandle)
	{
		if(NewObject == nullptr) {
			bool state = CurrentObjectID != -1;
			CurrentObjectID = -1;
			NewGLHandle = 0;
			return state;
		}

		NewGLHandle = NewObject->Handle();
		// Only ask for the ID if the object handle is non-zero
		// to avoid ID generation for null objects
		UniqueIdentifier NewObjectID = (NewGLHandle != 0) ? NewObject->GetUniqueID() : 0;

		// It is unsafe to use GL handle to keep track of bound textures
		// When a texture is released, GL is free to reuse its handle for
		// the new created textures
		if (CurrentObjectID != NewObjectID)
		{
			CurrentObjectID = NewObjectID;
			return true;
		}
		return false;
	}

	template <>
	bool UpdateBoundObject(UniqueIdentifier& CurrentObjectID, GLTexture* NewObject, GLuint& NewGLHandle)
	{
		if(NewObject == nullptr) {
			bool state = CurrentObjectID != -1;
			CurrentObjectID = -1;
			return state;
		}

		NewGLHandle = NewObject->Handle();

		if(NewObject->Format().AbstractFormat == GraphicsFormat::SRGBA8_UNORM && NewObject->EnabledBindSRGB) {
			NewGLHandle = NewObject->SRGBHandle();
		}

		// Only ask for the ID if the object handle is non-zero
		// to avoid ID generation for null objects
		UniqueIdentifier NewObjectID = (NewGLHandle != 0) ? NewObject->GetUniqueID() : 0;

		// It is unsafe to use GL handle to keep track of bound textures
		// When a texture is released, GL is free to reuse its handle for
		// the new created textures
		if (CurrentObjectID != NewObjectID)
		{
			CurrentObjectID = NewObjectID;
			return true;
		}
		return false;
	}

	template <class ObjectType>
	bool UpdateBoundObjectsArr(std::vector<UniqueIdentifier>& BoundObjectIDs, uint32_t Index, const ObjectType& NewObject, GLuint& NewGLHandle)
	{
		if (Index >= BoundObjectIDs.size())
			BoundObjectIDs.resize(Index + 1, -1);

		return UpdateBoundObject(BoundObjectIDs[Index], NewObject, NewGLHandle);
	}

	void GLContextState::SetShader(GLShaderProgram* shader)
	{
		GLuint GLProgHandle = 0;
		if (UpdateBoundObject(m_LastShaderHandle, shader, GLProgHandle) || true)
		{
			glUseProgram(GLProgHandle);
			CHECK_GL_ERROR("Failed to set GL program");
		}
	}

	void GLContextState::SetActiveTexture(GLContextState::BindIndex index)
	{
		if(m_ActiveTexture != index) {
			glActiveTexture(GL_TEXTURE0 + index);
			CHECK_GL_ERROR("Failed to activate texture slot ", Index);
			m_ActiveTexture = index;
		}
	}

	void GLContextState::BindTexture(GLContextState::BindIndex index, GLTexture* texture)
	{
		SetActiveTexture(index);

		GLuint GLTexHandle = 0;

		if (UpdateBoundObjectsArr(m_BoundTextures, index, texture, GLTexHandle) || true) // FIXME: Temporal RT should not be used here
		{
			if(texture != nullptr) {
				glBindTexture(texture->BindTarget(), GLTexHandle);
			} else {
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			CHECK_GL_ERROR("Failed to bind texture to slot ", index);
		}
	}

	void GLContextState::BindSampler(GLContextState::BindIndex index, GLSampler* sampler)
	{
		GLuint GLSamplerHandle = 0;
		if (UpdateBoundObjectsArr(m_BoundSamplers, index, sampler, GLSamplerHandle))
		{
			glBindSampler(index, GLSamplerHandle);
			CHECK_GL_ERROR("Failed to bind sampler to slot ", index);
		}
	}

	void GLContextState::BindImage(GLContextState::BindIndex index, GLTexture *texture, GLint mipLevel, GLboolean isLayered, GLint layer, GLenum access, GLenum format)
	{
#if GL_ARB_shader_image_load_store
		GLuint GLTexHandle = 0;
		if (UpdateBoundObjectsArr(m_BoundImages, index, texture, GLTexHandle) || true) // TODO: Fix this hotfix for compute shaders not updating texture after changing mip write level
		{
			glBindImageTexture(index, GLTexHandle, mipLevel, isLayered, layer, access, format);
			CHECK_GL_ERROR("glBindImageTexture() failed");
		}
#else
		AU_LOG_ERROR("GL_ARB_shader_image_load_store is not supported");
#endif
	}

	void GLContextState::BindUniformBuffer(GLContextState::BindIndex index, GLBuffer *buffer, uint32_t offset, uint32_t size)
	{
		GLuint GLBufferHandle = 0;
		if (UpdateBoundObjectsArr(m_BoundUniformBuffers, index, buffer, GLBufferHandle) || (buffer != nullptr)) // TODO: Fix this for buffer cache, when only offset and size is changed
		{
			if(buffer == nullptr || size == buffer->GetDesc().ByteSize)
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, index, GLBufferHandle);
			}
			else
			{
				glBindBufferRange(GL_UNIFORM_BUFFER, index, GLBufferHandle, offset, size);
			}
			CHECK_GL_ERROR("Failed to bind uniform buffer to slot ", index);
		}
	}

	void GLContextState::BindStorageBlock(GLContextState::BindIndex index, GLBuffer *buffer, uint32_t offset, uint32_t size)
	{
		GLuint GLBufferHandle = 0;
		if (UpdateBoundObjectsArr(m_BoundStorageBlocks, index, buffer, GLBufferHandle))
		{
			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, GLBufferHandle);
			glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, GLBufferHandle, offset, size);
			CHECK_GL_ERROR("Failed to bind shader storage block to slot ", index);
		}
	}


}