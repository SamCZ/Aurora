#include "RmShellSystemInterface.hpp"

#include "Aurora/Aurora.hpp"
#include "Aurora/Core/Time.hpp"
#include "Aurora/Logger/Logger.hpp"
#include "Aurora/App/IWindow.hpp"

namespace Aurora
{

	RmShellSystemInterface::RmShellSystemInterface()
	{

	}

	RmShellSystemInterface::~RmShellSystemInterface()
	{

	}

	double RmShellSystemInterface::GetElapsedTime()
	{
		return GetTimeInSeconds();
	}

	void RmShellSystemInterface::SetMouseCursor(const Rml::String &cursor_name)
	{
		// TODO: Set mouse cursor
	}

	void RmShellSystemInterface::SetClipboardText(const Rml::String &text)
	{
		GetEngine()->GetWindow()->SetClipboardString(text);
	}

	void RmShellSystemInterface::GetClipboardText(Rml::String &text)
	{
		text = GetEngine()->GetWindow()->GetClipboardString();
	}

	bool RmShellSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String &message)
	{
		AU_LOG_INFO(message);
		return true;
	}
}