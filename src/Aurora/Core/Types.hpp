#pragma once

#include <cstdint>
#include <filesystem>
#include "assert.hpp"

typedef uint32_t uint;
typedef uint8_t uint8;
typedef int8_t int8;
using Path = std::filesystem::path;
using DataBlob = std::vector<uint8_t>;