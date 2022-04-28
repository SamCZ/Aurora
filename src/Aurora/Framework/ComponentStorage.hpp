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
		size_t m_Index;
	public:
		explicit ComponentIterator(T** current, size_t index) : m_Current(current), m_Index(index) {}

		T* operator*()
		{
			return *m_Current;
		}

		ComponentIterator& operator++()
		{
			m_Current++;
			m_Index++;

			return *this;
		}

		bool operator==(const ComponentIterator<T>& other) const
		{
			return m_Index == other.m_Index;
		}

		bool operator!=(const ComponentIterator<T>& other) const
		{
			return m_Index != other.m_Index;
		}
	};

	template<typename T>
	class ComponentView
	{
	private:
		std::vector<std::uintptr_t> m_DataVector;
	public:
		ComponentView() : m_DataVector() {}

		explicit ComponentView(std::vector<std::uintptr_t> data) : m_DataVector(std::move(data))
		{
		}

		ComponentIterator<T> begin()
		{
			if(m_DataVector.empty())
			{
				return ComponentIterator<T>(nullptr, 0);
			}

			return ComponentIterator<T>(reinterpret_cast<T**>(&m_DataVector[0]), 0);
		}

		ComponentIterator<T> end()
		{
			if(m_DataVector.empty())
			{
				return ComponentIterator<T>(nullptr, 0);
			}

			return ComponentIterator<T>(nullptr, m_DataVector.size());
		}
	};

	class AU_API ComponentStorage
	{
	private:
		robin_hood::unordered_map<TTypeID, Aum*> m_ComponentMemory;
		robin_hood::unordered_map<TTypeID, std::vector<std::uintptr_t>> m_ComponentPointers;
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

			if (not m_ComponentMemory[componentID]->CheckMemory(component))
			{
				//__debugbreak();
				AU_LOG_FATAL("Memory corrupted!");
			}

			m_ComponentMemory[componentID]->DeAllocAndUnload<T>(component);
		}

		// Not multi-thread friendly currently
		template<typename T, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		ComponentView<T> GetComponents()
		{
			std::vector<std::uintptr_t> foundComponents;

			for(auto& it : m_ComponentPointers)
			{
				if(it.second.empty()) continue;

				auto* typeBase = reinterpret_cast<ObjectBase*>(it.second[0]);

				if(typeBase->HasType(T::TypeID()))
				{
					foundComponents.insert(foundComponents.end(), it.second.begin(), it.second.end());
				}
			}

			return ComponentView<T>(std::forward<std::vector<std::uintptr_t>>(foundComponents));
		}

		template<typename T, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		T* FindFirstComponent()
		{
			for(auto& it : m_ComponentPointers)
			{
				if(it.second.empty()) continue;

				auto* typeBase = reinterpret_cast<ObjectBase*>(it.second[0]);

				if(typeBase->HasType(T::TypeID()))
				{
					return reinterpret_cast<T*>(*it.second.begin());
				}
			}

			return nullptr;
		}
	};
}