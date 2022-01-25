#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace Aurora
{
	template<class UserClass, typename ReturnType, typename... ArgsTypes>
	struct MethodAction
	{
		typedef ReturnType(UserClass::*Type)(ArgsTypes...);
	};

	template<typename ReturnType, typename... ArgsTypes>
	struct FunctionAction
	{
		typedef ReturnType(*Type)(ArgsTypes...);
	};

	template<typename ReturnType = void, typename... ArgsTypes>
	class IDelegate
	{
	public:
		virtual ReturnType Invoke(ArgsTypes&& ...args) = 0;
	};

	template<class UserClass, typename ReturnType, typename... ArgsTypes>
	class MethodDelegate : public IDelegate<ReturnType, ArgsTypes...>
	{
	public:
		typedef typename MethodAction<UserClass, ReturnType, ArgsTypes...>::Type MethodPtr;
	private:
		UserClass* m_Instance;
		MethodPtr m_Method;
	public:
		inline MethodDelegate(UserClass* instance, MethodPtr method) : m_Instance(instance), m_Method(method) {}

		ReturnType Invoke(ArgsTypes&& ...args) override
		{
			return std::invoke(m_Method, *m_Instance, args...);
		}
	};

	template<typename ReturnType, typename... ArgsTypes>
	class FunctionDelegate : public IDelegate<ReturnType, ArgsTypes...>
	{
	public:
		typedef typename FunctionAction<ReturnType, ArgsTypes...>::Type FunctionPtr;
	private:
		FunctionPtr m_Function;
	public:
		inline explicit FunctionDelegate(FunctionPtr function) : m_Function(function) {}

		ReturnType Invoke(ArgsTypes&& ...args) override
		{
			return std::invoke(m_Function, args...);
		}
	};

#if INTPTR_MAX == INT64_MAX
	typedef uint64_t EventID;
#else
	typedef uint32_t EventID;
#endif

	template<typename... ArgsTypes>
	class EventEmitter
	{
	public:
		typedef IDelegate<void, ArgsTypes...>* Delegate;
	private:
		std::vector<Delegate> m_Delegates;
	public:
		EventID Bind(Delegate delegate)
		{
			m_Delegates.push_back(delegate);
			return reinterpret_cast<EventID>(delegate);
		}


		EventID Bind(typename FunctionAction<void, ArgsTypes...>::Type function)
		{
			return Bind(new FunctionDelegate<void, ArgsTypes...>(function));
		}

		template<class UserClass>
		EventID Bind(UserClass* instance, typename MethodAction<UserClass, void, ArgsTypes...>::Type method)
		{
			return Bind(new MethodDelegate<UserClass, void, ArgsTypes...>(instance, method));
		}

		bool Unbind(EventID eventId)
		{
			auto it = std::find(m_Delegates.begin(), m_Delegates.end(), reinterpret_cast<Delegate>(eventId));

			if(it == m_Delegates.end())
				return false;

			m_Delegates.erase(it);

			return true;
		}

		void Invoke(ArgsTypes&& ...args)
		{
			for(Delegate delegate : m_Delegates)
			{
				delegate->Invoke(std::forward<ArgsTypes>(args)...);
			}
		}
	};
}