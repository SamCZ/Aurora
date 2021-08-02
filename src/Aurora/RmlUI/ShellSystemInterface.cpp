#include "ShellSystemInterface.hpp"

#include "Aurora/AuroraEngine.hpp"
#include "Aurora/App/GLFWWindow.hpp"

namespace Aurora
{
	GLFWcursor* arrowCursor;
	GLFWcursor* ibeamCursor;
	GLFWcursor* crosshairCursor;
	GLFWcursor* handCursor;
	GLFWcursor* hresizeCursor;
	GLFWcursor* vResizeCursor;
	GLFWcursor* allResizeCursor;

	GLFWcursor* cursor;

	ShellSystemInterface::ShellSystemInterface()
	{
		arrowCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		ibeamCursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		crosshairCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
		handCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
		hresizeCursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		vResizeCursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		allResizeCursor = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);

		/*unsigned char pixels[16 * 16 * 4];
		memset(pixels, 0xff, sizeof(pixels));

		GLFWimage image;
		image.width = 16;
		image.height = 16;
		image.pixels = pixels;

		cursor = glfwCreateCursor(&image, 0, 0);*/
	}

	ShellSystemInterface::~ShellSystemInterface()
	{
		glfwDestroyCursor(arrowCursor);
		glfwDestroyCursor(ibeamCursor);
		glfwDestroyCursor(crosshairCursor);
		glfwDestroyCursor(handCursor);
		glfwDestroyCursor(hresizeCursor);
		glfwDestroyCursor(vResizeCursor);
	}

	double ShellSystemInterface::GetElapsedTime()
	{
		return glfwGetTime();
	}

	void ShellSystemInterface::SetMouseCursor(const Rml::String &cursor_name)
	{
		//AU_LOG_WARNING(cursor_name, " - is not implemented !");

		auto context = AuroraEngine::GetCurrentThreadContext();

		if(context == nullptr) return;

		GLFWwindow* window = static_cast<GLFWWindow*>(context->GetWindow().get())->GetHandle();

		if(cursor_name == "move") {
			glfwSetCursor(window, allResizeCursor);
			return;
		}

		glfwSetCursor(window, nullptr);
	}

	void ShellSystemInterface::SetClipboardText(const Rml::String &text)
	{
		Aurora::AuroraEngine::GetCurrentThreadContext()->GetWindow()->SetClipboardString(text);
	}

	void ShellSystemInterface::GetClipboardText(Rml::String &text)
	{
		text = Aurora::AuroraEngine::GetCurrentThreadContext()->GetWindow()->GetClipboardString();
	}

	bool ShellSystemInterface::LogMessage(Rml::Log::Type type, const String &message)
	{
		AU_LOG_INFO(message);
		return true;
	}
}