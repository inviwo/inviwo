#pragma once

#include <ticpp/ticppapi.h>

constexpr int TIXML_MAJOR_VERSION = 3;
constexpr int TIXML_MINOR_VERSION = 0;
constexpr int TIXML_PATCH_VERSION = 0;

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

// Only used by Attribute::Query functions
enum { TIXML_SUCCESS, TIXML_NO_ATTRIBUTE, TIXML_WRONG_TYPE };
