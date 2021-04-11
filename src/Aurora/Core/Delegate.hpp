#pragma once

#include <functional>
#include <vector>
#include <map>
#include <memory>

namespace Aurora
{
	template<typename... ArgsTypes>
	class EventList
	{
	public:
		typedef typename std::function<void(ArgsTypes...)> Fnc;
	private:
		std::map<int, std::weak_ptr<Fnc>> m_Events;
		int m_NextId;
	public:
		inline EventList() : m_Events() {}
	public:

		inline int Add(const std::weak_ptr<Fnc> lambda)
		{
			int id = m_NextId++;
			m_Events[id] = lambda;
			return id;
		}

		inline EventList& Remove(int id)
		{
			m_Events.erase(id);
			return *this;
		}

		inline void Invoke(ArgsTypes... args)
		{
			auto iter = m_Events.begin();
			auto endIter = m_Events.end();

			for(; iter != endIter; ) {
				if (iter->second.lock() == nullptr) {
					iter = m_Events.erase(iter);
				} else {
					++iter;
				}
			}

			for(const auto& eventPtr : m_Events) {
				if(auto event = eventPtr.second.lock()) {
					(*event)(std::forward<ArgsTypes>(args)...);
				}
			}
		}

		inline void Clear() { m_Events.clear(); }
	};
}

#define EVENT_LIST(name, fnc)
