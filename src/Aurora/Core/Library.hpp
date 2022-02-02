#pragma once

#if AU_SHARED
	#ifdef AU_EXPORT
		#define AU_API __declspec(dllexport)
	#else
		#define AU_API __declspec(dllimport)
	#endif
#else
	#define AU_API
#endif

// Define for breakpointing.
#if defined (_WIN32)
	#if defined __WIN32__ || defined _WIN32
		#define ES_BREAK {asm("int $0x03");}
	#elif defined (_MSC_VER)
#define ES_BREAK {__debugbreak();}
		#else
			#define ES_BREAK
	#endif
#else
	#define ES_BREAK {__builtin_trap();}
#endif