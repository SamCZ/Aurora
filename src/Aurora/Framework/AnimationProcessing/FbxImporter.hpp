#pragma once

#include <Aurora/Core/Library.hpp>
#include <vector>
#include <cstdint>
#include <memory>

namespace Aurora
{
	class Mesh;
}

namespace FbxImport
{
	AU_API std::shared_ptr<Aurora::Mesh> LoadScene(const std::vector<uint8_t>& data);
}