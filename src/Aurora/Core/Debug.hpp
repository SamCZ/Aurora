#pragma once

// Define for breakpointing.
#if defined (_WIN32)
	#if defined (_MSC_VER)
		#define ES_BREAK {__debugbreak();}
	#elif defined __WIN32__ || defined _WIN32
		#define ES_BREAK {asm("int $0x03");}
	#else
		#define ES_BREAK
	#endif
#else
	#define ES_BREAK {__builtin_trap();}
#endif