#pragma once

#include "Aurora/Core/Types.hpp"

namespace Aurora
{
	typedef uint64_t ResourceID_t;
	
	class ResourceName
	{
	private:

	public:
		ResourceName() = default;
		ResourceName(std::string name)
		{

		}

		ResourceName(ResourceID_t id)
		{

		}
	};
}