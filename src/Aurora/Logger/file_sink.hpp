#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include "Logger.hpp"

namespace Aurora
{
	class file_sink : public Logger::Sink
	{
	private:
		std::ofstream m_Stream;
	public:
		inline explicit file_sink(const std::filesystem::path& path) : m_Stream()
		{
			m_Stream.open(path);
		}

		void Log(const std::string& severity, const std::string& file, const std::string& function, int line, const std::string& message) override
		{
			if(!file.empty()) {
				m_Stream << severity << ": " << file << ": " << function << "(): " << line << ": " << message << std::endl;
			} else {
				m_Stream << severity << ": " << message << std::endl;
			}

			m_Stream.flush();
		}
	};
}