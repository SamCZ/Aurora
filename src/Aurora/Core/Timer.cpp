#include "Timer.hpp"

using namespace std::chrono;

namespace Aurora
{
	Timer::Timer()
	{
		Restart();
	}

	void Timer::Restart()
	{
		m_StartTime = std::chrono::high_resolution_clock::now();
	}

	template <typename T>
	T GetElapsedTime(high_resolution_clock::time_point StartTime)
	{
		auto CurrTime  = high_resolution_clock::now();
		auto time_span = duration_cast<duration<T>>(CurrTime - StartTime);
		return time_span.count();
	}

	double Timer::GetElapsedTime() const
	{
		return Aurora::GetElapsedTime<double>(m_StartTime);
	}

	float Timer::GetElapsedTimef() const
	{

		return Aurora::GetElapsedTime<float>(m_StartTime);
	}

	double Timer::GetTimeNow()
	{
		auto CurrTime  = std::chrono::high_resolution_clock::now();
		auto time_span = duration_cast<std::chrono::duration<double>>(CurrTime.time_since_epoch());
		return time_span.count();
	}
}