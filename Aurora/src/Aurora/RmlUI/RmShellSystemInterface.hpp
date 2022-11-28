#pragma once

#include <RmlUi/Core/SystemInterface.h>
#include "Aurora/Core/Library.hpp"

namespace Aurora
{
	class AU_API RmShellSystemInterface : public Rml::SystemInterface
	{
	public:
		RmShellSystemInterface();
		~RmShellSystemInterface() override;

		/// Get the number of seconds elapsed since the start of the application
		/// @returns Seconds elapsed
		double GetElapsedTime() override;

		/// Set mouse cursor.
		/// @param[in] cursor_name Cursor name to activate.
		void SetMouseCursor(const Rml::String& cursor_name) override;

		/// Set clipboard text.
		/// @param[in] text Text to apply to clipboard.
		void SetClipboardText(const Rml::String& text) override;

		/// Get clipboard text.
		/// @param[out] text Retrieved text from clipboard.
		void GetClipboardText(Rml::String& text) override;

		bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;
	};
}
