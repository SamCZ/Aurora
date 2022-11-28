#pragma once

#include <RmlUi/Core.h>

namespace Aurora
{
	inline Rml::Element* FindElementUpByClass(Rml::Element* current, const Rml::String& className, int searchDepth = 5)
	{
		if (current->IsClassSet(className))
		{
			return current;
		}

		if (searchDepth <= 0)
		{
			return nullptr;
		}

		if (current->GetParentNode() != nullptr)
		{
			Rml::Element* parent = current->GetParentNode();
			return FindElementUpByClass(parent, className, searchDepth - 1);
		}

		return nullptr;
	}
}