#pragma once

#include "Aurora/Tools/robin_hood.h"
#include "Aurora/Logger/Logger.hpp"
#include "UserInterface.hpp"

namespace Aurora
{
	class AU_API GameModeBase
	{
	private:
		robin_hood::unordered_map<UIID_t, UserInterface*> m_UserInterfaces;
	public:
		virtual ~GameModeBase()
		{
			for (const auto &item : m_UserInterfaces)
			{
				DestroyUI(item.second);
			}
		}

		virtual void BeginPlay() = 0;
		virtual void BeginDestroy() = 0;
		virtual void Tick(double delta) {}

		template<class T, typename... Args, typename std::enable_if<std::is_base_of<UserInterface, T>::value>::type* = nullptr>
		UserInterface* AddUserInterface(UIID_t id, Args... args)
		{
			if(m_UserInterfaces.contains(id))
			{
				AU_LOG_WARNING("User interface with id ", id, " already exists !");
				return nullptr;
			}

			UserInterface* ui = new T(id, std::forward<Args>(args)...);
			ui->BeginPlay();

			m_UserInterfaces[id] = ui;

			return ui;
		}

		bool GetUserInterface(UIID_t id, UserInterface*& ui)
		{
			auto it = m_UserInterfaces.find(id);

			if(it == m_UserInterfaces.end())
			{
				return false;
			}

			ui = it->second;
			return true;
		}

		void RemoveUserInterface(UIID_t id)
		{
			auto it = m_UserInterfaces.find(id);

			if(it == m_UserInterfaces.end())
			{
				AU_LOG_WARNING("User interface with id ", id, " does not exists !");
				return;
			}

			DestroyUI(it->second);
			m_UserInterfaces.erase(it);
		}

		void SetUserInterfaceEnabled(UIID_t id, bool enabled, bool disableOthers = false)
		{
			for (auto &item : m_UserInterfaces)
			{
				if(item.first == id)
				{
					item.second->SetEnabled(enabled);
					if(!disableOthers)
					{
						return;
					}
				} else if(disableOthers)
				{
					item.second->SetEnabled(false);
				}
			}
		}

		void TickUI(double delta)
		{
			for (auto &item : m_UserInterfaces)
			{
				item.second->Tick(delta);
			}
		}
	private:
		inline static void DestroyUI(UserInterface* ui)
		{
			if(ui)
				ui->Destroy();
			delete ui;
		}
	};
}