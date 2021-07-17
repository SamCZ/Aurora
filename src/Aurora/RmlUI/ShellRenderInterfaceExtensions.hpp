#pragma once

namespace Aurora
{
	class ShellRenderInterfaceExtensions
	{
	public:
		/**
		 * @param[in] width width of viewport
		 * @param[in] height height of viewport
		 */
		virtual void SetViewport(int width, int height) = 0;

		/// Attach the internal window buffer to a native window
		/// @param[in] nativeWindow A handle to the OS specific native window handle
		virtual bool AttachToNative(void *nativeWindow) = 0;

		/// Detach and cleanup the internal window buffer from a native window
		virtual void DetachFromNative(void) = 0;

		/// Prepares the render buffer for drawing, in OpenGL, this would call glClear();
		virtual void PrepareRenderBuffer(void) = 0;

		/// Presents the rendered framebuffer to the screen, in OpenGL this would cal glSwapBuffers();
		virtual void PresentRenderBuffer(void) = 0;
	};
}