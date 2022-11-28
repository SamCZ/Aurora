#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <utility>
#include "Logger.hpp"

namespace Aurora
{
	class file_sink : public Logger::Sink
	{
	private:
		std::filesystem::path m_Path;
		std::ofstream m_Stream;
	public:
		inline explicit file_sink(std::filesystem::path path) : m_Path(std::move(path)), m_Stream()
		{

		}

		void Log(const Logger::Severity& severity, const std::string& severityStr, const std::string& file, const std::string& function, int line, const std::string& message) override
		{
			if(!m_Stream.good()) {
				m_Stream.open(m_Path);

				if(!m_Stream.good()) {
					return;
				}
			}

			if(!file.empty()) {
				m_Stream << severityStr << ": " << file << ": " << function << "(): " << line << ": " << message << std::endl;
			} else {
				m_Stream << severityStr << ": " << message << std::endl;
			}

			m_Stream.flush();
		}
	};
}