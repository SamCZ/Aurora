#pragma once

#ifndef ASSERT_ENABLED
#define ASSERT_ENABLED
#endif

#ifdef ASSERT_ENABLED
	#include "Aurora/Logger/Logger.hpp"
	#include "Debug.hpp"

	#define au_assert(cond) do { if(!(cond)) { AU_LOG_ERROR("Condition was not met ! (", #cond, ")"); ES_BREAK } } while(false)
#else
#define au_assert(cond) ((void)cond)
#endif