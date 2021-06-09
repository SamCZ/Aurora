#pragma once

#include <string>
#include "Format.hpp"

namespace Aurora
{
	struct ShaderInputVariable
	{
		std::string Name;
		size_t Size;
		GraphicsFormat Format;
		bool Instanced : 1;
		int32_t SemanticIndex;
	};

	struct VertexAttributeDesc
	{
		std::string Name;
		GraphicsFormat Format;
		uint32_t BufferIndex;
		uint32_t Offset;
		bool IsInstanced;
		uint32_t SemanticIndex;
	};

	typedef std::map<uint8_t, ShaderInputVariable> ShaderInputVariables_t;
}