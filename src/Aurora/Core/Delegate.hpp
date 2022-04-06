#pragma once

#include <cstdint>
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
		virtual ~IDelegate() = default;
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
			return (*m_Instance.*m_Method)(std::forward<ArgsTypes&&>(args)...);
			//return std::invoke(m_Method, *m_Instance, args...);
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
			return m_Function(std::forward<ArgsTypes&&>(args)...);
		}
	};

#if INTPTR_MAX == INT64_MAX
	typedef uint64_t EventID;
#else
	typedef uint32_t EventID;
#endif

	class EventEmitterBase
	{
	public:
		virtual ~EventEmitterBase() = default;
		virtual bool Unbind(EventID eventId) = 0;
	};

	class UniqueEvent
	{
	private:
		mutable EventEmitterBase* m_EmitterBase;
		EventID m_EventID;
	public:
		UniqueEvent() : m_EmitterBase(nullptr), m_EventID(0) {}
		UniqueEvent(EventEmitterBase* emitterBase, EventID eventId) : m_EmitterBase(emitterBase), m_EventID(eventId) { }

		UniqueEvent(const UniqueEvent& other) = delete;
		UniqueEvent& operator=(const UniqueEvent& other)
		{
			if(m_EmitterBase == other.m_EmitterBase && m_EventID == other.m_EventID)
			{
				return *this;
			}

			// Unbind current event
			if(m_EmitterBase)
				m_EmitterBase->Unbind(m_EventID);

			// Copy data from other to current
			m_EmitterBase = other.m_EmitterBase;
			m_EventID = other.m_EventID;

			// Set other emitter to null to prevent deleting now current event
			other.m_EmitterBase = nullptr;

			return *this;
		}

		~UniqueEvent()
		{
			if(m_EmitterBase)
				m_EmitterBase->Unbind(m_EventID);
		}
	};

	template<typename... ArgsTypes>
	class EventEmitter : public EventEmitterBase
	{
	public:
		typedef IDelegate<void, ArgsTypes...>* Delegate;
	private:
		std::vector<Delegate> m_Delegates;
	public:
		~EventEmitter() override
		{
			for(Delegate delegate : m_Delegates)
			{
				delete delegate;
			}
		}

		EventID Bind(Delegate delegate)
		{
			m_Delegates.push_back(delegate);
			return reinterpret_cast<EventID>(delegate);
		}

		UniqueEvent BindUnique(Delegate delegate)
		{
			m_Delegates.push_back(delegate);
			return UniqueEvent(this, reinterpret_cast<EventID>(delegate));
		}

		EventID Bind(typename FunctionAction<void, ArgsTypes...>::Type function)
		{
			return Bind(new FunctionDelegate<void, ArgsTypes...>(function));
		}

		UniqueEvent BindUnique(typename FunctionAction<void, ArgsTypes...>::Type function)
		{
			return Bind(new FunctionDelegate<void, ArgsTypes...>(function));
		}

		template<class UserClass>
		EventID Bind(UserClass* instance, typename MethodAction<UserClass, void, ArgsTypes...>::Type method)
		{
			return Bind(new MethodDelegate<UserClass, void, ArgsTypes...>(instance, method));
		}

		template<class UserClass>
		UniqueEvent BindUnique(UserClass* instance, typename MethodAction<UserClass, void, ArgsTypes...>::Type method)
		{
			return BindUnique(new MethodDelegate<UserClass, void, ArgsTypes...>(instance, method));
		}

		bool Unbind(EventID eventId) override
		{
			auto it = std::find(m_Delegates.begin(), m_Delegates.end(), reinterpret_cast<Delegate>(eventId));

			if(it == m_Delegates.end())
				return false;

			delete *it;
			m_Delegates.erase(it);

			return true;
		}

		void Invoke(ArgsTypes&& ...args) const
		{
			for(const Delegate& delegate : m_Delegates)
			{
				delegate->Invoke(std::forward<ArgsTypes>(args)...);
			}
		}
	};
}