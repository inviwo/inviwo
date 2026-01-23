#pragma once


#ifdef INVIWO_ALL_DYN_LINK  // DYNAMIC
// If we are building DLL files we must declare dllexport/dllimport
#ifdef IVW_MODULE_DATAFRAME_EXPORTS
#ifdef _WIN32
#define IVW_MODULE_DATAFRAME_API __declspec(dllexport)
#define IVW_MODULE_DATAFRAME_EXT
#define IVW_MODULE_DATAFRAME_TMPL_EXP
#define IVW_MODULE_DATAFRAME_TMPL_INST __declspec(dllexport)
#else  // UNIX (GCC)
#define IVW_MODULE_DATAFRAME_API __attribute__((visibility("default")))
#define IVW_MODULE_DATAFRAME_EXT
#define IVW_MODULE_DATAFRAME_TMPL_EXP __attribute__((__visibility__("default")))
#define IVW_MODULE_DATAFRAME_TMPL_INST
#endif
#else
#ifdef _WIN32
#define IVW_MODULE_DATAFRAME_API __declspec(dllimport)
#define IVW_MODULE_DATAFRAME_EXT extern
#define IVW_MODULE_DATAFRAME_TMPL_EXP __declspec(dllimport)
#define IVW_MODULE_DATAFRAME_TMPL_INST
#else
#define IVW_MODULE_DATAFRAME_API
#define IVW_MODULE_DATAFRAME_EXT extern
#define IVW_MODULE_DATAFRAME_TMPL_EXP __attribute__((__visibility__("default")))
#define IVW_MODULE_DATAFRAME_TMPL_INST
#endif
#endif
#else  // STATIC
#define IVW_MODULE_DATAFRAME_API
#define IVW_MODULE_DATAFRAME_EXT extern
#define IVW_MODULE_DATAFRAME_TMPL_EXP
#define IVW_MODULE_DATAFRAME_TMPL_INST
#endif
