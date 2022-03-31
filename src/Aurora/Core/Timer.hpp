#pragma once

#include "Time.hpp"

namespace Aurora
{
	class TickTimer
	{
	private:
		double m_LastUpdate;
		double m_Delay;
	public:
		explicit TickTimer(double delayInSeconds) : m_LastUpdate(-1), m_Delay(delayInSeconds) {}

		operator bool()
		{
			double currentTime = GetTimeInSeconds();

			if(currentTime >= m_LastUpdate + m_Delay || m_LastUpdate < 0)
			{
				m_LastUpdate = currentTime;
				return true;
			}

			return false;
		}
	};
}