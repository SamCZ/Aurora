#include "Logger.hpp"

namespace Aurora
{
	std::vector<std::shared_ptr<Logger::Sink>> Logger::m_Sinks;
}