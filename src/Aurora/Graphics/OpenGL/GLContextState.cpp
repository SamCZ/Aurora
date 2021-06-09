#include "GLContextState.hpp"

namespace Aurora
{

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
		glBindVertexArray(0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);


	}


}