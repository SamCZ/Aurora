#pragma once

#include <cstdint>

namespace Aurora
{
	typedef uint8_t UIID_t;

	class UserInterface
	{
	protected:
		UIID_t m_ID;
		bool m_Enabled;
	public:
		explicit UserInterface(UIID_t id) : m_ID(id), m_Enabled(true) {}
		virtual ~UserInterface() = default;

		virtual void BeginPlay() = 0;
		virtual void BeginDestroy() = 0;
		virtual void Tick(double delta) = 0;

		[[nodiscard]] inline UIID_t GetID() const { return m_ID; }

		[[nodiscard]] inline bool IsEnabled() const { return m_Enabled; }
		inline void SetEnabled(bool enabled) { m_Enabled = enabled; }
	};
}