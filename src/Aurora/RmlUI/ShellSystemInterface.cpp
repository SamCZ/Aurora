#include "ShellSystemInterface.hpp"

#include "Aurora/AuroraEngine.hpp"

namespace Aurora
{

	double ShellSystemInterface::GetElapsedTime()
	{
		return glfwGetTime();
	}

	void ShellSystemInterface::SetMouseCursor(const Rml::String &cursor_name)
	{
		AU_LOG_WARNING(cursor_name, " - is not implemented !");
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