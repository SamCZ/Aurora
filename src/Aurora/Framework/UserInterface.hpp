#pragma once

#include <cstdint>
#include <vector>
#include <Aurora/Engine.hpp>
#include <Aurora/Core/Common.hpp>
#include <Aurora/Core/String.hpp>
#include <Aurora/Core/Vector.hpp>
#include <Aurora/Core/Library.hpp>
#include <Aurora/RmlUI/RmlUI.hpp>
#include <RmlUi/Core/DataModelHandle.h>

namespace Aurora
{
	typedef uint8_t UIID_t;

	class AU_API UserInterface
	{
	protected:
		UIID_t m_ID;
		bool m_Enabled;
		std::vector<Rml::ElementDocument*> m_Documents;
	public:
		explicit UserInterface(UIID_t id) : m_ID(id), m_Enabled(true) {}
		virtual ~UserInterface() = default;

		virtual void Destroy();

		virtual void BeginPlay() = 0;
		virtual void BeginDestroy() {}
		virtual void Tick(double delta) {}

		Rml::ElementDocument* LoadAndRegisterDocument(const String& path, bool defaultVisible = true);

		Rml::ElementDocument* RegisterDocument(Rml::ElementDocument* document, bool defaultVisible = true)
		{
			m_Documents.push_back(document);

			if (document->IsVisible() && !m_Enabled)
			{
				document->Hide();
			}
			else if(!document->IsVisible() && m_Enabled && defaultVisible)
			{
				document->Show();
			}

			return document;
		}

		void RemoveAndCloseDocument(Rml::ElementDocument*& document)
		{
			VectorRemove(m_Documents, document);
			document->Close();
			document = nullptr;
		}

		void ReloadDocument(Rml::ElementDocument*& document)
		{
			VectorRemove(m_Documents, document);
			document = RegisterDocument(GEngine->GetRmlUI()->ReloadDocument(document));
		}

		inline static Rml::DataModelConstructor CreateDataModel(const String& name)
		{
			return GEngine->GetRmlUI()->GetRmlContext()->CreateDataModel(name);
		}

		inline static bool RemoveDataModel(const String& name)
		{
			return GEngine->GetRmlUI()->GetRmlContext()->RemoveDataModel(name);
		}

		[[nodiscard]] inline UIID_t GetID() const { return m_ID; }

		[[nodiscard]] inline bool IsEnabled() const { return m_Enabled; }
		inline void SetEnabled(bool enabled) { m_Enabled = enabled; }
	};
}