#pragma once

#include <unordered_map>
#include "Aurora/Tools/robin_hood.h"

#if _DEBUG
template <typename A, typename B>
using FastMap = std::unordered_map<A, B>;
#else
template <typename A, typename B>
using FastMap = robin_hood::unordered_map<A, B>;
#endif