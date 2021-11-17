#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <stack>
#include "Aurora/Graphics/OpenGL/GL.hpp"
#include "../Logger/Logger.hpp"

#if AU_TRACY_ENABLED
#include <TracyOpenGL.hpp>
#endif

#define CAT_(a, b) a ## b
#define CAT(a, b) CAT_(a, b)

#if defined(AURORA_OPENGL) && AU_GPU_PROFILE == 1
#include "Aurora/Graphics/OpenGL/GLRenderGroupScope.hpp"
#define GPU_DEBUG_SCOPE(name) ::Aurora::GLRenderGroupScope CAT(_GPU_Debug_Scope_, __LINE__)(name)
#else
#define GPU_DEBUG_SCOPE(name)
#endif

namespace Aurora
{
	class LocalProfileScope
	{
	public:
		struct LocalTiming
		{
			std::string Name;
			std::chrono::high_resolution_clock::time_point StartTime;
			std::chrono::high_resolution_clock::time_point EndTime;
			std::vector<LocalTiming> Childs;
			bool IsRoot;

			[[nodiscard]] inline int64_t GetElapsedTimeInNanoSeconds() const
			{
				return (EndTime - StartTime).count();
			}

			[[nodiscard]] inline double GetElapsedTimeInMicroseconds() const
			{
				return static_cast<double>(GetElapsedTimeInNanoSeconds()) / 1000.0;
			}

			[[nodiscard]] inline double GetElapsedTimeInMilliseconds() const
			{
				return static_cast<double>(GetElapsedTimeInMicroseconds()) / 1000.0;
			}

			[[nodiscard]] inline double GetElapsedTimeInSeconds() const
			{
				return GetElapsedTimeInMilliseconds() / 1000;
			}
		};
	private:
		static std::stack<LocalTiming> m_TimingStack;
		static LocalTiming m_LastFrameTiming;
	public:
		explicit LocalProfileScope(const std::string& name)
		{
			LocalTiming timing = {};
			timing.Name = name;
			timing.StartTime = std::chrono::high_resolution_clock::now();
			timing.IsRoot = false;
			m_TimingStack.emplace(timing);
		}

		~LocalProfileScope()
		{
			auto now = std::chrono::high_resolution_clock::now();
			LocalTiming timing = m_TimingStack.top();
			timing.EndTime = now;
			m_TimingStack.pop();

			LocalTiming& parentTiming = m_TimingStack.top();
			parentTiming.Childs.emplace_back(timing);
		}

		static void Reset(const std::string& rootName)
		{
			if(m_TimingStack.size() != 1)
			{
				AU_LOG_WARNING("Profiler timing stack is at reset is not 1");
				while(!m_TimingStack.empty()) m_TimingStack.pop();
			}
			else
			{
				m_LastFrameTiming = m_TimingStack.top();
				m_LastFrameTiming.EndTime = std::chrono::high_resolution_clock::now();
				m_TimingStack.pop();
			}

			LocalTiming rootTiming = {};
			rootTiming.Name = rootName;
			rootTiming.IsRoot = true;
			rootTiming.StartTime = std::chrono::high_resolution_clock::now();
			m_TimingStack.emplace(rootTiming);
		}

		static const LocalTiming& GetLastFrameTimings() { return m_LastFrameTiming; }
	};
}

#define AU_CPU_DEBUG_SCOPE(name) ::Aurora::LocalProfileScope CAT(AU_GPU_Debug_Scope_, __LINE__)(name)

#if AU_TRACY_ENABLED
#define CPU_DEBUG_SCOPE(name) ZoneNamedN(CAT(_GPU_Debug_Scope_, __LINE__), name, true)
#else
#define CPU_DEBUG_SCOPE(name) AU_CPU_DEBUG_SCOPE(name)
#endif