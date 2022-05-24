#pragma once

#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <vector>
#include <Aurora/Core/Library.hpp>
#include <Aurora/Core/FormatString.hpp>
#include <Aurora/Core/SystemUtils.hpp>

namespace Aurora
{
	class AU_API Logger
	{
	public:
		enum class Severity : uint8_t
		{
			Info = 0,
			Warning,
			Error,
			FatalError
		};

		class Sink
		{
		public:
			virtual void Log(const Severity& severity, const std::string& severityStr, const std::string& file, const std::string& function, int line, const std::string& message) = 0;
		};

	private:
		static std::vector<std::shared_ptr<Sink>> m_Sinks;
	public:
		template<typename... ArgsType>
		static void Log(const Severity& severity, const char* function, const char* fullFilePath, int line, const ArgsType&... args)
		{
			auto message = FormatString(args...);

			std::string fileName;
			if(fullFilePath) fileName = fullFilePath;
			auto lastSlashPos = fileName.find_last_of("/\\");
			if (lastSlashPos != std::string::npos)
				fileName.erase(0, lastSlashPos + 1);

			static const char* const strSeverities[] = {"Info", "Warning", "ERROR", "CRITICAL ERROR"};
			std::string messageSeverity = strSeverities[static_cast<uint8_t>(severity)];

			std::string fncStr;
			if(function) fncStr = function;

			for(auto& sink : m_Sinks) {
				sink->Log(severity, messageSeverity, fileName, fncStr, line, message);
			}

			if (severity == Severity::FatalError)
			{
				ShowErrorTraceWindow(message);
			}
		}

		template<class Sink, typename... ArgsType>
		static std::shared_ptr<Sink> AddSink(ArgsType&&... args)
		{
			std::shared_ptr<Sink> sink = std::make_shared<Sink>(std::forward<ArgsType>(args)...);
			m_Sinks.push_back(sink);
			return sink;
		}

		template<class Sink, typename... ArgsType>
		static std::shared_ptr<Sink> AddSinkPtr(const std::shared_ptr<Sink>& sink)
		{
			m_Sinks.push_back(sink);
			return sink;
		}

		template<class Sink>
		static bool RemoveSink(const std::shared_ptr<Sink>& sink)
		{
			auto it = std::find(m_Sinks.begin(), m_Sinks.end(), sink);

			if(it == m_Sinks.end()) {
				return false;
			}

			m_Sinks.erase(it);

			return true;
		}
	};
}

#define AU_LOG(_severity, ...) ::Aurora::Logger::Log(::Aurora::Logger::Severity::_severity, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#define AU_LOG_INFO(...) AU_LOG(Info, __VA_ARGS__)
#define AU_LOG_WARNING(...) AU_LOG(Warning, __VA_ARGS__)
#define AU_LOG_ERROR(...) AU_LOG(Error, __VA_ARGS__)
#define AU_LOG_FATAL(...) AU_LOG(FatalError, __VA_ARGS__);
