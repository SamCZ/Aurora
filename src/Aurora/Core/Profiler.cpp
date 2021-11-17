#include "Profiler.hpp"

namespace Aurora
{
	std::stack<LocalProfileScope::LocalTiming> LocalProfileScope::m_TimingStack;
	LocalProfileScope::LocalTiming LocalProfileScope::m_LastFrameTiming = {"_Uninitialized"};
}