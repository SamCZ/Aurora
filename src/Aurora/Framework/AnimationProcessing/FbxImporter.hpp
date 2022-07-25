#pragma once

#include <Aurora/Core/Library.hpp>
#include <vector>
#include <cstdint>

namespace FbxImport
{
	AU_API void LoadScene(const std::vector<uint8_t>& data);
}