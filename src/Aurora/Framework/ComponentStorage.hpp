#pragma once

#include "Aurora/Core/Object.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Logger/Logger.hpp"
#include "Aurora/Memory/Aum.hpp"
#include "Aurora/Tools/robin_hood.h"
#include "ActorComponent.hpp"

namespace Aurora
{
	class ComponentStorage
	{
	private:
		robin_hood::unordered_map<TTypeID, Aum*> m_ComponentMemory;
	public:
		~ComponentStorage()
		{
			for(auto& it : m_ComponentMemory)
			{
				delete it.second;
			}
		}

		template<typename T, typename... Args, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		T* CreateComponent(const String& name, Args&& ... args)
		{
			TTypeID componentID = T::TypeID();

			Aum* allocator = nullptr;

			size_t componentSize = sizeof(T);
			size_t componentSizeAligned = Align(componentSize, 16u);

			if(m_ComponentMemory.contains(componentID))
			{
				allocator = m_ComponentMemory[componentID];
			}
			else
			{
				allocator = new Aum();
				m_ComponentMemory.emplace(componentID, allocator);
				AU_LOG_INFO("New allocator for component ", T::TypeName(), " with size of ", FormatBytes(componentSize), " aligned ", componentSizeAligned);
			}

			MemPtr componentMemory = allocator->Alloc(componentSizeAligned);
			ActorComponent* component = new(componentMemory) T(std::forward<Args>(args)...);
			component->SetName(name);
			return (T*) component;
		}

		template<typename T, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		void DestroyComponent(T* component)
		{
			TTypeID componentID = T::TypeID();

			if(!m_ComponentMemory.contains(componentID))
			{
				AU_LOG_WARNING("Component ", T::TypeName(), " does not exists in Scene !");
				return;
			}

			m_ComponentMemory[componentID]->DeAllocAndUnload<T>(component);
		}
	};
}