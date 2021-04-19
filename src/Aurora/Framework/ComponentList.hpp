#pragma once

#include "Aurora/Core/Common.hpp"
#include "ComponentConcept.hpp"
#include "ISystem.hpp"

namespace Aurora
{
	template<ComponentType T>
	class ComponentList
	{
	private:
		std::vector<T*> m_Components;
		std::vector<ISystem<T>*> m_Systems;
	public:
		~ComponentList()
		{
			for(auto* system : m_Systems) {
				delete system;
			}
		}
	public:
		inline void Add(T* component)
		{
			m_Components.push_back(component);
		}

		inline void Remove(T* component)
		{
			VectorRemove(m_Components, component);
		}

		inline void Clear()
		{
			m_Components.clear();
		}

		[[nodiscard]] inline size_t Count() const
		{
			return m_Components.size();
		}

		inline std::vector<T*> GetComponents()
		{
			return m_Components;
		}

		inline const std::vector<T*>& GetComponents() const
		{
			return m_Components;
		}

		inline explicit operator std::vector<T*>&()
		{
			return m_Components;
		}

		inline explicit operator const std::vector<T*>&() const
		{
			return m_Components;
		}

		inline typename std::vector<T*>::iterator begin() { return m_Components.begin(); }
		inline typename std::vector<T*>::iterator end() { return m_Components.end(); }
	public:
		template<typename System, typename... Args, BASE_OF(System, ISystem<T>)>
		inline System* AddSystem(Args&& ... args)
		{
			System* system = new System(std::forward<Args>(args)...);
			m_Systems.push_back(system);
			return system;
		}

		inline void RemoveSystem(ISystem<T>* system)
		{
			VectorRemove(m_Systems, system);
		}

		inline size_t SystemCount()
		{
			return m_Systems.size();
		}
	public:
		void PreUpdateSystems(double delta)
		{
			for(ISystem<T>* system : m_Systems) {
				system->PreUpdate(m_Components, delta);
			}
		}

		void UpdateComponents(double delta)
		{
			/*for(auto* component : m_Components) {
				if(!component->IsActive()) continue;
				component->Tick(delta);
			}*/

			for(int i = (int)m_Components.size() - 1; i >= 0; i--) {
				auto* component = m_Components[i];
				component->Tick(delta);
			}
		}

		void UpdateSystems(double delta)
		{
			for(ISystem<T>* system : m_Systems) {
				system->Update(m_Components, delta);
			}
		}

		void PostUpdateSystems(double delta)
		{
			for(ISystem<T>* system : m_Systems) {
				system->PostUpdate(m_Components, delta);
			}
		}
	};
}