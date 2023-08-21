#pragma once

#include <ticpp/ticppapi.h>

// Deprecated library function hell. Compilers want to use the
// new safe versions. This probably doesn't fully address the problem,
// but it gets closer. There are too many compilers for me to fully
// test. If you get compilation troubles, undefine TIXML_SAFE
#define TIXML_SAFE

#ifdef TIXML_SAFE
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
// Microsoft visual studio, version 2005 and higher.
#define TIXML_SNPRINTF _snprintf_s
#define TIXML_SNSCANF _snscanf_s
#define TIXML_SSCANF sscanf_s
#elif defined(_MSC_VER) && (_MSC_VER >= 1200)
// Microsoft visual studio, version 6 and higher.
// #pragma message( "Using _sn* functions." )
#define TIXML_SNPRINTF _snprintf
#define TIXML_SNSCANF _snscanf
#define TIXML_SSCANF sscanf
#elif defined(__GNUC__) && (__GNUC__ >= 3)
// GCC version 3 and higher.s
// #warning( "Using sn* functions." )
#define TIXML_SNPRINTF snprintf
#define TIXML_SNSCANF snscanf
#define TIXML_SSCANF sscanf
#else
#define TIXML_SSCANF sscanf
#endif
#endif

constexpr int TIXML_MAJOR_VERSION = 2;
constexpr int TIXML_MINOR_VERSION = 5;
constexpr int TIXML_PATCH_VERSION = 3;

constexpr unsigned char TIXML_UTF_LEAD_0 = 0xefU;
constexpr unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
constexpr unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

class TICPP_API TiXmlDocument;
class TICPP_API TiXmlElement;
class TICPP_API TiXmlComment;
class TICPP_API TiXmlUnknown;
class TICPP_API TiXmlAttribute;
class TICPP_API TiXmlText;
class TICPP_API TiXmlDeclaration;
class TICPP_API TiXmlStylesheetReference;
class TICPP_API TiXmlParsingData;
class TICPP_API TiXmlVisitor;

// Used by the parsing routines.
enum TiXmlEncoding { TIXML_ENCODING_UNKNOWN, TIXML_ENCODING_UTF8, TIXML_ENCODING_LEGACY };

constexpr TiXmlEncoding TIXML_DEFAULT_ENCODING = TIXML_ENCODING_UNKNOWN;

// Only used by Attribute::Query functions
enum { TIXML_SUCCESS, TIXML_NO_ATTRIBUTE, TIXML_WRONG_TYPE };
