#pragma once

#include <type_traits>
#include <functional>

#include "Log.hpp"
#include "Container.hpp"
#include "String.hpp"

namespace Aurora
{
	template<class UserClass, typename ReturnType, typename... ArgsTypes>
	struct MethodAction
	{
		typedef ReturnType(UserClass::*Type)(ArgsTypes...);
	};

	template<typename ReturnType, typename... ArgsTypes>
	class IDelegate
	{
	public:
		virtual ReturnType Invoke(ArgsTypes... params) = 0;
	};

	template<class UserClass, typename ReturnType, typename... ArgsTypes>
	class Delegate : public IDelegate<ReturnType, ArgsTypes...>
	{
	private:
		UserClass* m_Class;
	public:
		typedef typename MethodAction<UserClass, ReturnType, ArgsTypes...>::Type MethodPtr;
	private:
		MethodPtr m_Method;
	public:

		explicit Delegate(UserClass* object, MethodPtr method) : m_Class(object), m_Method(method) {}

		ReturnType Invoke(ArgsTypes... params) override
		{
			return std::invoke(m_Method, *m_Class, params...);
		}
	};

    template<typename ReturnType, typename... ArgsTypes>
    class LambdaDelegate : public IDelegate<ReturnType, ArgsTypes...>
    {
    private:
        typedef Function<ReturnType(ArgsTypes...)> FunctionPtr;
        FunctionPtr m_Function;
    public:

        explicit LambdaDelegate(FunctionPtr function) : m_Function(function) {}

        ReturnType Invoke(ArgsTypes... params) override
        {
            return m_Function(params...);
        }
    };

	template<typename ReturnType, typename... ArgsType>
	struct NamedDelegate
	{
		String Name;
		IDelegate<ReturnType, ArgsType...>* Delegate;
	};

	template<typename ReturnType, typename... ArgsType>
	class DelegateEvent
	{
	private:
		Map<String, IDelegate<ReturnType, ArgsType...>*> m_Events;
	public:
		~DelegateEvent()
		{
			for (auto& it : m_Events)
			{
				delete it.second;
			}
		}

		DelegateEvent& operator+=(const NamedDelegate<ReturnType, ArgsType...>& d)
		{
			if (m_Events.find(d.Name) != m_Events.end())
			{
				LogError("DelegateEvent +=", d.Name, "Cannot add ! Same event already exists.");
				delete d.Delegate;

				return *this;
			}

            m_Events[d.Name] = d.Delegate;

			return *this;
		}

		DelegateEvent& operator-=(const String evtName)
		{
			if (_Events.find(evtName) != _Events.end())
			{
				delete m_Events[evtName];

                m_Events.erase(evtName);
			}

			return *this;
		}

		void Invoke(ArgsType... args)
		{
			for (auto& it : m_Events)
			{
				it.second->Invoke(args...);
			}
		}
	};

#define EVENT_NAME(ClassName, MethodName) (String(#ClassName) + ":" + String(#MethodName))
#define EVENT(ClassName, MethodName) NamedDelegate<void> { EVENT_NAME(ClassName, MethodName), new Delegate<ClassName, void>(this, &ClassName::MethodName) }
#define EVENT_ARGS(ClassName, MethodName, ...) NamedDelegate<void, __VA_ARGS__> { EVENT_NAME(ClassName, MethodName), new Delegate<ClassName, void, __VA_ARGS__>(this, &ClassName::MethodName) }
#define OEVENT_ARGS(ClassName, MethodName, Instance, ...) NamedDelegate<void, __VA_ARGS__> { EVENT_NAME(ClassName, MethodName), new Delegate<ClassName, void, __VA_ARGS__>(Instance, &ClassName::MethodName) }
#define LEVENT_ARGS(ClassName, MethodName, Lambda, ...) NamedDelegate<void, __VA_ARGS__> { EVENT_NAME(ClassName, MethodName), new LambdaDelegate<ClassName, void, __VA_ARGS__>(Lambda) }

#define FUNC_POINTER(Name, ReturnType, ...) ReturnType* (*Name)(__VA_ARGS__)
}