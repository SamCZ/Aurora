#pragma once

#if defined(AU_SHARED) && AU_SHARED==1 && defined(_WIN32)
	#ifdef AU_EXPORT
		#define AU_API __declspec(dllexport)
	#else
		#define AU_API __declspec(dllimport)
	#endif
#else
	#define AU_API
#endif
