#pragma once

#include "Aurora/Core/Object.hpp"
#include "Aurora/Core/Library.hpp"

namespace Aurora
{
	class AU_API MaterialClass : public ObjectBase
	{
	public:
		CLASS_OBJ(MaterialClass, ObjectBase);
		virtual ~MaterialClass() override;
	};
}
