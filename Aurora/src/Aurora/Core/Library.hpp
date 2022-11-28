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
