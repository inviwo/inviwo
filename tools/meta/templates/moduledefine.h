#pragma once

// clang-format off
#ifdef INVIWO_ALL_DYN_LINK  //DYNAMIC
	// If we are building DLL files we must declare dllexport/dllimport
	#ifdef IVW_MODULE_{{ upper(module/name) }}_EXPORTS
		#ifdef _WIN32
			#define {{ module/api }} __declspec(dllexport)
		#else  //UNIX (GCC)
			#define {{ module/api }} __attribute__ ((visibility ("default")))
		#endif
	#else
		#ifdef _WIN32
			#define {{ module/api }} __declspec(dllimport)
		#else
			#define {{ module/api }}
		#endif
	#endif
#else  //STATIC
	#define {{ module/api }}
#endif
// clang-format on
