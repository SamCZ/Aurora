#pragma once

#include <iostream>
#include "Logger.hpp"

namespace Aurora
{
	class std_sink : public Logger::Sink
	{
	public:
		void Log(const Logger::Severity& severity, const std::string& severityStr, const std::string& file, const std::string& function, int line, const std::string& message) override
		{
			if(!file.empty()) {
				std::cout << severityStr << ": " << file << ": " << function << "(): " << line << ": " << message << std::endl;
			} else {
				std::cout << severityStr << ": " << message << std::endl;
			}
		}
	};
}