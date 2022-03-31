#pragma once

#include <utility>

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/UUID.hpp"

namespace Aurora
{
	struct ResourceName
	{
		UUID ID;
		std::string Name;

		ResourceName() = default;
		ResourceName(const UUID& uuid, std::string name) : ID(uuid), Name(std::move(name)) {}
		explicit ResourceName(const std::string& name)
		{
			std::string_view view = name;
			size_t uuidEnd = 0;

			if(view.front() == '{')
			{
				std::string uuidStr;
				for (int i = 1; i < view.size(); ++i)
				{
					if (view[i] == '}')
						break;
					uuidEnd++;

					uuidStr += view[i];

					if(i == view.size() - 1)
					{
						uuidEnd = 0;
						break;
					}
				}

				if(uuidEnd == 0 && uuidStr.length())
				{

				}
				else
				{
					auto opt = UUID::FromString<std::string>(uuidStr);
					if(opt.has_value())
					{
						ID = opt.value();
					}

					uuidEnd++;
					uuidEnd++;
				}
			}

			Name.append(view.substr(uuidEnd));
		}
	};
}