#include "RmlUI.hpp"

#include <atomic>
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include "RmShellSystemInterface.hpp"
#include "RmShellRenderInterfaceOpenGL.hpp"
#include "RmlUIFileInterface.hpp"

#include "Aurora/Aurora.hpp"
#include "Aurora/App/ISystemWindow.hpp"
#include "Aurora/Logger/Logger.hpp"

namespace Aurora
{
	class RmlContextInstancer : public Rml::ContextInstancer
	{
	public:
		/// Create instance of RmlContext.
		Rml::ContextPtr InstanceContext(const Rml::String& name) override
		{
			return Rml::ContextPtr(new RmlContext(name));
		}
		/// Free instance of RmlContext.
		void ReleaseContext(Rml::Context* context) override
		{
			delete static_cast<RmlContext*>(context);
		}

	protected:
		/// RmlContextInstancer is static, nothing to release.
		void Release() override { }
	};

	/// Event instancer that translates some inline events to native Urho3D events.
	class RmlEventListenerInstancer : public Rml::EventListenerInstancer
	{
	public:
		/// Create an instance of inline event listener, if applicable.
		Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element) override
		{
			/*if (auto* instancer = SoundEventListener::CreateInstancer(value, element))
				return instancer;
			if (auto* instancer = CustomEventListener::CreateInstancer(value, element))
				return instancer;*/

			return nullptr;
		}
	};

	class RmlPlugin : public Rml::Plugin
	{
	public:
		int GetEventClasses() override { return EVT_DOCUMENT; }
		void OnDocumentUnload(Rml::ElementDocument* document) override
		{
			auto* rmlContext = static_cast<RmlContext*>(document->GetContext());
			RmlUI* uiWeak = rmlContext->GetOwnerSubsystem();
			uiWeak->OnDocumentUnload(document);
		}
	};

	static std::atomic<int> rmlInstanceCounter;
	static RmlContextInstancer RmlContextInstancerInstance;
	static RmlEventListenerInstancer RmlEventListenerInstancerInstance;
	static RmlPlugin RmlPluginInstance;

	RmlUI::RmlUI(const std::string& name) : m_RmlContext(nullptr), m_DebuggerInitialized(false), m_Enabled(true)
	{
		if (rmlInstanceCounter.fetch_add(1) == 0)
		{
			Rml::SetRenderInterface(new RmShellRenderInterfaceOpenGL());
			Rml::SetSystemInterface(new RmShellSystemInterface());
			Rml::SetFileInterface(new RmlUIFileInterface());
			Rml::Initialise();
			Rml::Factory::RegisterEventListenerInstancer(&RmlEventListenerInstancerInstance);
			Rml::Factory::RegisterContextInstancer(&RmlContextInstancerInstance);
			Rml::RegisterPlugin(&RmlPluginInstance);
		}

		m_RmlContext = static_cast<RmlContext*>(Rml::CreateContext(name, GetDesiredCanvasSize()));
		m_RmlContext->SetOwnerSubsystem(this);
	}

	RmlUI::~RmlUI()
	{
		if (m_RmlContext != nullptr)
		{
			if (!Rml::RemoveContext(m_RmlContext->GetName()))
			{
				AU_LOG_FATAL("Removal of RmlUI context {} failed.", m_RmlContext->GetName());
			}
		}
		m_RmlContext = nullptr;

		if (rmlInstanceCounter.fetch_sub(1) == 1)
		{
			// Freeing last instance of RmlUI, deinitialize backend library.
			Rml::Factory::RegisterEventListenerInstancer(nullptr); // Set to a static object instance because there is no getter to delete it.
			auto* renderer = Rml::GetRenderInterface();
			auto* system = Rml::GetSystemInterface();
			auto* file = Rml::GetFileInterface();
			Rml::ReleaseTextures();
			Rml::Shutdown();
			delete renderer;
			delete system;
			delete file;
		}
	}

	Rml::ElementDocument *RmlUI::LoadDocument(const std::string &path)
	{
		return m_RmlContext->LoadDocument(path);
	}

	void RmlUI::SetDebuggerVisible(bool visible)
	{
		if (!m_DebuggerInitialized)
		{
			Rml::Debugger::Initialise(m_RmlContext);
			m_DebuggerInitialized = true;
		}
		Rml::Debugger::SetVisible(visible);
	}

	void RmlUI::ToggleDebuggerVisible()
	{
		SetDebuggerVisible(!Rml::Debugger::IsVisible());
	}

	bool RmlUI::LoadFont(const std::string &resourceName, bool fallback)
	{
		return Rml::LoadFontFace(resourceName, fallback);
	}

	Rml::Context* RmlUI::GetRmlContext() const
	{
		return m_RmlContext;
	}

	void RmlUI::SetScale(float scale)
	{
		m_RmlContext->SetDensityIndependentPixelRatio(scale);
	}

	bool RmlUI::IsInputCaptured() const
	{
		return IsInputCapturedInternal();
	}

	void RmlUI::Update()
	{
		m_RmlContext->Update();
	}

	void RmlUI::Render(const DrawCallState& drawCallState)
	{
		auto* shellRenderInterfaceOpenGl = static_cast<RmShellRenderInterfaceOpenGL*>(Rml::GetRenderInterface());

		shellRenderInterfaceOpenGl->PrepareRenderBuffer(drawCallState);
		m_RmlContext->Render();
		shellRenderInterfaceOpenGl->PresentRenderBuffer();
	}

	Rml::ElementDocument *RmlUI::ReloadDocument(Rml::ElementDocument *document)
	{
		assert(document != nullptr);
		assert(document->GetContext() == m_RmlContext);

		Rml::Vector2 pos = document->GetAbsoluteOffset(Rml::Box::BORDER);
		Rml::Vector2 size = document->GetBox().GetSize(Rml::Box::CONTENT);
		Rml::ModalFlag modal = document->IsModal() ? Rml::ModalFlag::Modal : Rml::ModalFlag::None;
		Rml::FocusFlag focus = Rml::FocusFlag::Auto;
		bool visible = document->IsVisible();
		if (Rml::Element* element = m_RmlContext->GetFocusElement())
			focus = element->GetOwnerDocument() == document ? Rml::FocusFlag::Document : focus;

		document->Close();

		Rml::ElementDocument* newDocument = m_RmlContext->LoadDocument(document->GetSourceURL());
		newDocument->SetProperty(Rml::PropertyId::Left, Rml::Property(pos.x, Rml::Property::PX));
		newDocument->SetProperty(Rml::PropertyId::Top, Rml::Property(pos.y, Rml::Property::PX));
		newDocument->SetProperty(Rml::PropertyId::Width, Rml::Property(size.x, Rml::Property::PX));
		newDocument->SetProperty(Rml::PropertyId::Height, Rml::Property(size.y, Rml::Property::PX));
		newDocument->UpdateDocument();
		newDocument->ReloadStyleSheet();

		if (visible)
			newDocument->Show(modal, focus);

		return newDocument;
	}

	Rml::Vector2i RmlUI::GetDesiredCanvasSize() const
	{
		auto windowSize = GEngine->GetWindow()->GetSize();

		return {windowSize.x, windowSize.y};
	}

	bool RmlUI::IsHovered() const
	{
		Rml::Element* hover = m_RmlContext->GetHoverElement();
		return hover != nullptr && hover != m_RmlContext->GetRootElement();
	}

	bool RmlUI::IsInputCapturedInternal() const
	{
		if (Rml::Element* element = m_RmlContext->GetFocusElement())
		{
			const std::string& tag = element->GetTagName();
			return tag == "input" || tag == "textarea" || tag == "select";
		}
		return false;
	}

	void RmlUI::OnDocumentUnload(Rml::ElementDocument *document)
	{
		// TODO: Call event
	}
}