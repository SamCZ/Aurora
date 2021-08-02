#pragma once

#include <RmlUi/Core/SystemInterface.h>

namespace Aurora
{
    class ShellSystemInterface : public Rml::SystemInterface
    {
    public:
		ShellSystemInterface();
    	~ShellSystemInterface();
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