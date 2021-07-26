#pragma once

#include <string>
#include <utility>

#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Context.h>

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/Common.hpp"

namespace Aurora
{
	class RmlContext;

	AU_CLASS(RmlUI) : public SharedFromThis<RmlUI>
	{
		friend class RmlContext;
		friend class RmlPlugin;

	private:
		RmlContext* m_RmlContext;
		bool m_DebuggerInitialized;
		bool m_Enabled;
	public:
		explicit RmlUI(const std::string& name);
		~RmlUI() override;
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
		void SetOwnerSubsystem(std::weak_ptr<RmlUI> ui) { m_ContextSystem = std::move(ui); }
		[[nodiscard]] std::weak_ptr<RmlUI> GetOwnerSubsystem() const { return m_ContextSystem; }
	private:
		std::weak_ptr<RmlUI> m_ContextSystem;
	};
}