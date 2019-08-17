#ifndef _IVW_MODULE_WEBBROWSER_DEFINE_H_
#define _IVW_MODULE_WEBBROWSER_DEFINE_H_

#ifdef INVIWO_ALL_DYN_LINK  // DYNAMIC
// If we are building DLL files we must declare dllexport/dllimport
#ifdef IVW_MODULE_WEBBROWSER_EXPORTS
#ifdef _WIN32
#define IVW_MODULE_WEBBROWSER_API __declspec(dllexport)
#define IVW_MODULE_WEBBROWSER_EXT
#define IVW_MODULE_WEBBROWSER_TMPL_EXP
#define IVW_MODULE_WEBBROWSER_TMPL_INST __declspec(dllexport)
#else  // UNIX (GCC)
#define IVW_MODULE_WEBBROWSER_API __attribute__((visibility("default")))
#define IVW_MODULE_WEBBROWSER_EXT
#define IVW_MODULE_WEBBROWSER_TMPL_EXP __attribute__((__visibility__("default")))
#define IVW_MODULE_WEBBROWSER_TMPL_INST
#endif
#else
#ifdef _WIN32
#define IVW_MODULE_WEBBROWSER_API __declspec(dllimport)
#define IVW_MODULE_WEBBROWSER_EXT extern
#define IVW_MODULE_WEBBROWSER_TMPL_EXP __declspec(dllimport)
#define IVW_MODULE_WEBBROWSER_TMPL_INST
#else
#define IVW_MODULE_WEBBROWSER_API
#define IVW_MODULE_WEBBROWSER_EXT extern
#define IVW_MODULE_WEBBROWSER_TMPL_EXP __attribute__((__visibility__("default")))
#define IVW_MODULE_WEBBROWSER_TMPL_INST
#endif
#endif
#else  // STATIC
#define IVW_MODULE_WEBBROWSER_API
#define IVW_MODULE_WEBBROWSER_EXT extern
#define IVW_MODULE_WEBBROWSER_TMPL_EXP
#define IVW_MODULE_WEBBROWSER_TMPL_INST
#endif

#endif /* _IVW_MODULE_WEBBROWSER_DEFINE_H_ */