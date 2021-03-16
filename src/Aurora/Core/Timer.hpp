#pragma once

#include <chrono>

namespace Aurora
{
	class Timer
	{
	public:
		Timer();
		void   Restart();
		[[nodiscard]] double GetElapsedTime() const;
		[[nodiscard]] float  GetElapsedTimef() const;

		static double GetTimeNow();

	private:
		std::chrono::high_resolution_clock::time_point m_StartTime;
	};
}