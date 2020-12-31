#pragma once

#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <functional>
#include <queue>
#include <deque>

namespace Aurora
{
	template <typename A>
	using List = std::vector<A>;

	template <typename A, typename B>
	using Map = std::map<A, B>;

	template <typename A, typename B>
	using FastMap = std::unordered_map<A, B>;

	template <typename A>
	using Set = std::set<A>;

	template <typename A, typename B>
	using Pair = std::pair<A, B>;

    template <typename T>
    using Function = std::function<T>;

    template <typename T>
    using Queue = std::queue<T>;

    template <typename T>
    using Dedeque = std::deque<T>;

    #define InRange(list, index) (index < list.size())

	#define List_Remove(list, type) { list.erase(std::find(list.begin(), list.end(), type)); }
	//#define Add(type) push_back(type)

	#define ITER(map, it_name) for(decltype(map)::iterator it_name = map.begin(); it_name != map.end(); it_name++)

	template <typename T>
	static inline typename List<T>::iterator Find(List<T>& list, T value)
	{
		return std::find(list.begin(), list.end(), value);
	}

	template <typename T>
	static inline typename Set<T>::iterator Find(Set<T>& list, T value)
	{
		return std::find(list.begin(), list.end(), value);
	}

	template <typename T>
	static inline bool Exist(List<T>& list, T value)
	{
		return std::find(list.begin(), list.end(), value) != list.end();
	}
}