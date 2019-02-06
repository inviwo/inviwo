#pragma once

#ifdef INVIWO_ALL_DYN_LINK  // DYNAMIC
// If we are building DLL files we must declare dllexport/dllimport
#ifdef INVIWO_META_EXPORTS
#ifdef _WIN32
#define INVIWO_META_API __declspec(dllexport)
#else  // UNIX (GCC)
#define INVIWO_META_API __attribute__((visibility("default")))
#endif
#else
#ifdef _WIN32
#define INVIWO_META_API __declspec(dllimport)
#else
#define INVIWO_META_API
#endif
#endif
#else  // STATIC
#define INVIWO_META_API
#endif
