#pragma once

#ifndef ASSERT_ENABLED
#define ASSERT_ENABLED
#endif

#ifdef ASSERT_ENABLED
#include "Aurora/Logger/Logger.hpp"

#define au_assert(cond) do { if(!(cond)) { AU_LOG_FATAL("Condition was not met ! (", #cond, ")"); } } while(false)
#else
#define au_assert(cond) ((void)cond)
#endif