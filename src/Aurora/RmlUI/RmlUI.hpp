#pragma once

#include <string>
#include <utility>

#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>

namespace Aurora
{
	class RmlContext;

	class RmlUI
	{
		friend class RmlContext;
		friend class RmlPlugin;
	private:
		RmlContext* m_RmlContext;
		bool m_DebuggerInitialized;
		bool m_Enabled;
	public:
		explicit RmlUI(const std::string& name);
		~RmlUI();
	public:
		Rml::ElementDocument* LoadDocument(const std::string& path);
		void SetDebuggerVisible(bool visible);
		bool LoadFont(const std::string& resourceName, bool fallback = false);
		Rml::Context* GetRmlContext() const;
		void SetScale(float scale);

		inline void SetEnabled(bool enabled) { m_Enabled = enabled; }
		inline bool IsEnabled() const { return m_Enabled; }

		bool IsInputCaptured() const;

		void Update();
		void Render();

		Rml::ElementDocument* ReloadDocument(Rml::ElementDocument* document);

	private:
		Rml::Vector2i GetDesiredCanvasSize() const;
		bool IsHovered() const;
		bool IsInputCapturedInternal() const;
		void OnDocumentUnload(Rml::ElementDocument* document);
	};

	class RmlContext : public Rml::Context
	{
	public:
		explicit RmlContext(const std::string& name) : Rml::Context(name) { }
		void SetOwnerSubsystem(RmlUI* ui) { m_ContextSystem = ui; }
		[[nodiscard]] RmlUI* GetOwnerSubsystem() const { return m_ContextSystem; }
	private:
		RmlUI* m_ContextSystem;
	};
}