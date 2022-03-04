#pragma once

#include "Aurora/Core/Object.hpp"
#include "Aurora/Core/Common.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Logger/Logger.hpp"
#include "Aurora/Memory/Aum.hpp"
#include "Aurora/Tools/robin_hood.h"
#include "ActorComponent.hpp"

namespace Aurora
{
	template<typename T>
	class ComponentIterator
	{
	private:
		T** m_Current;
	public:
		explicit ComponentIterator(T** current) : m_Current(current) {}

		T* operator*()
		{
			return *m_Current;
		}

		ComponentIterator& operator++()
		{
			m_Current++;
			return *this;
		}

		bool operator==(const ComponentIterator<T>& other) const
		{
			return m_Current == other.m_Current;
		}

		bool operator!=(const ComponentIterator<T>& other) const
		{
			return m_Current != other.m_Current;
		}
	};

	template<typename T>
	class ComponentView
	{
	private:
		std::vector<std::uintptr_t>* m_DataVector;
	public:
		ComponentView() : m_DataVector(nullptr) {}

		explicit ComponentView(std::vector<std::uintptr_t>* data) : m_DataVector(data) { }

		ComponentIterator<T> begin()
		{
			if(!m_DataVector)
			{
				return ComponentIterator<T>(nullptr);
			}

			return ComponentIterator<T>(reinterpret_cast<T**>(&(*m_DataVector)[0]));
		}

		ComponentIterator<T> end()
		{
			if(!m_DataVector)
			{
				return ComponentIterator<T>(nullptr);
			}

			return ComponentIterator<T>(reinterpret_cast<T**>(&(*m_DataVector)[m_DataVector->size()]));
		}
	};

	class ComponentStorage
	{
	private:
		robin_hood::unordered_map<TTypeID, Aum*> m_ComponentMemory;
		robin_hood::unordered_map<TTypeID, std::vector<std::uintptr_t>> m_ComponentPointers;
		std::vector<std::uintptr_t> m_FindComponentCache;
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
				AU_LOG_INFO("New allocator for component ", T::TypeName(), " with size of ", FormatBytes(componentSize), " aligned ", FormatBytes(componentSizeAligned));
			}

			MemPtr componentMemory = allocator->Alloc(componentSizeAligned);
			ActorComponent* component = new(componentMemory) T(std::forward<Args>(args)...);
			component->SetName(name);

			m_ComponentPointers[componentID].push_back((std::uintptr_t)component);

			return (T*) component;
		}

		template<typename T, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		void DestroyComponent(T* component)
		{
			TTypeID componentID = component->GetTypeID();

			if(!m_ComponentMemory.contains(componentID))
			{
				AU_LOG_WARNING("Component ", component->GetTypeName(), " does not exists in Scene !");
				return;
			}

			VectorRemove<std::uintptr_t>(m_ComponentPointers[componentID], (std::uintptr_t)component);
			m_ComponentMemory[componentID]->DeAllocAndUnload<T>(component);
		}

		// Not multi-thread friendly currently
		template<typename T, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		ComponentView<T> GetComponents()
		{
			m_FindComponentCache.clear();

			for(auto& it : m_ComponentPointers)
			{
				if(it.second.empty()) continue;

				auto* typeBase = reinterpret_cast<ObjectBase*>(it.second[0]);

				if(typeBase->HasType(it.first))
				{
					m_FindComponentCache.insert(m_FindComponentCache.end(), it.second.begin(), it.second.end());
				}
			}

			return ComponentView<T>(&m_FindComponentCache);
		}
	};
}