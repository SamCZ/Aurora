#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <utility>

#include <common/TracySystem.hpp>

class WorkerThread
{
private:
	std::string             m_ThreadName;
	std::atomic_bool        m_ReadyState;
	std::atomic_bool        m_ProcessedState;
	std::atomic_bool        m_DestroyState;
	std::mutex              m_Mutex;
	std::condition_variable m_ConditionVariable;

	std::thread m_Thread;

	std::function<void()> m_ThreadFunction;
public:
	inline explicit WorkerThread(std::string threadName = "") : m_ThreadName(std::move(threadName)), m_Thread(), m_ReadyState(false), m_ProcessedState(false), m_DestroyState(false), m_Mutex(), m_ConditionVariable()
	{

	}

	inline explicit WorkerThread(std::function<void()> threadFunc, std::string threadName = "")
			:
			m_ThreadName(std::move(threadName)),
			m_ThreadFunction(std::move(threadFunc)),
			m_Thread([this] { Run(); }),
			m_ReadyState(false),
			m_ProcessedState(false),
			m_DestroyState(false),
			m_Mutex(),
			m_ConditionVariable()
	{

	}
public:
	inline bool Destroy()
	{
		MarkForDestroy();
		Trigger();

		if(m_Thread.joinable()) {
			m_Thread.join();
			return true;
		}

		return false;
	}

	inline void WaitFor()
	{
		std::unique_lock<std::mutex> lk(m_Mutex);
		m_ConditionVariable.wait(lk, [this]{return (bool)m_ProcessedState;});
		m_ProcessedState = false;
	}

	inline void Trigger()
	{
		std::lock_guard<std::mutex> lk(m_Mutex);
		m_ReadyState = true;
		m_ConditionVariable.notify_one();
	}

	inline void MarkForDestroy()
	{
		m_DestroyState = true;
	}
private:
	inline void Run()
	{
		if(m_ThreadName.length() > 0)
		{
			tracy::SetThreadName(m_ThreadName.c_str());
		}

		while(true)
		{
			std::unique_lock<std::mutex> lk(m_Mutex);
			m_ConditionVariable.wait(lk, [this]{ return (bool)m_ReadyState;});

			if(m_DestroyState) {
				break;
			}

			m_ThreadFunction();

			m_ProcessedState = true;
			m_ReadyState = false;

			lk.unlock();
			m_ConditionVariable.notify_one();
		}
	}
};