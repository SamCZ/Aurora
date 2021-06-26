#pragma once

#include <cstdint>
#include "GL.hpp"

#include "GLShaderProgram.hpp"
#include "GLTexture.hpp"
#include "GLSampler.hpp"
#include "GLBuffer.hpp"

namespace Aurora
{
	class GLContextState
	{
	public:
		typedef uint32_t BindIndex;
	private:
		uint32_t m_PendingMemoryBarriers = 0;

		UniqueIdentifier m_LastShaderHandle;
		BindIndex m_ActiveTexture;

		std::vector<UniqueIdentifier> m_BoundTextures;
		std::vector<UniqueIdentifier> m_BoundSamplers;
		std::vector<UniqueIdentifier> m_BoundImages;
		std::vector<UniqueIdentifier> m_BoundUniformBuffers;
		std::vector<UniqueIdentifier> m_BoundStorageBlocks;
	public:
		GLContextState();
	public:
		void Invalidate();

		void SetShader(GLShaderProgram* shader);

		void SetActiveTexture(BindIndex index);
		void BindTexture(BindIndex index, GLTexture* texture);
		void BindSampler(BindIndex index, GLSampler* sampler);
		void BindImage(BindIndex index, GLTexture* texture, GLint mipLevel, GLboolean isLayered, GLint layer, GLenum access, GLenum format);
		void BindUniformBuffer(BindIndex index, GLBuffer* buffer);
		void BindStorageBlock(BindIndex index, GLBuffer* buffer);
	};
}