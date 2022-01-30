#pragma once

#include "Aurora/Core/Object.hpp"

namespace Aurora
{
	class MaterialClass : public ObjectBase
	{
	public:
		CLASS_OBJ(MaterialClass, ObjectBase);
		virtual ~MaterialClass() override;
	};
}
