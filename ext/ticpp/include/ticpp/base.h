#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>

#include <string>
#include <string_view>
#include <array>
#include <cassert>
#include <stdexcept>
#include <array>
#include <memory>
#include <algorithm>

/**	Internal structure for tracking location of items
 * in the XML file.
 */
struct TICPP_API TiXmlCursor {
    void Clear() {
        row = -1;
        col = -1;
    }

    int row = -1;  // 0 based.
    int col = -1;  // 0 based.
};

enum class TiXmlErrorCode {
    TIXML_NO_ERROR = 0,
    TIXML_ERROR,
    TIXML_ERROR_OPENING_FILE,
    TIXML_ERROR_PARSING_ELEMENT,
    TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME,
    TIXML_ERROR_READING_ELEMENT_VALUE,
    TIXML_ERROR_READING_ATTRIBUTES,
    TIXML_ERROR_PARSING_EMPTY,
    TIXML_ERROR_READING_END_TAG,
    TIXML_ERROR_PARSING_UNKNOWN,
    TIXML_ERROR_PARSING_COMMENT,
    TIXML_ERROR_PARSING_DECLARATION,
    TIXML_ERROR_DOCUMENT_EMPTY,
    TIXML_ERROR_EMBEDDED_NULL,
    TIXML_ERROR_PARSING_CDATA,
    TIXML_ERROR_DOCUMENT_TOP_ONLY,

    TIXML_ERROR_DUPLICATE_ATTRIBUTE,

    TIXML_ERROR_STRING_COUNT
};

class TiXmlParsingData;

#include <warn/push>
#include <warn/ignore/dll-interface-base>
class TICPP_API TiXmlError : public std::runtime_error {
public:
    TiXmlError(TiXmlErrorCode err, const char* errorLocation, TiXmlParsingData* parseData);

    /** Generally, you probably want the error string ( ErrorDesc() ). But if you
        prefer the ErrorId, this function will fetch it.
    */
    TiXmlErrorCode ErrorCode() const { return errorCode; }

    /** Returns the location (if known) of the error. The first column is column 1,
        and the first row is row 1. A value of 0 means the row and column wasn't applicable
        (memory errors, for example, have no row/column) or the parser lost the error. (An
        error in the error reporting, in that case.)

        @sa SetTabSize, Row, Column
    */
    int ErrorRow() const { return location.row + 1; }

    /// The column where the error occurred. See ErrorRow()
    int ErrorCol() const { return location.col + 1; }

private:
    TiXmlErrorCode errorCode;
    TiXmlCursor location;
};
#include <warn/pop>

struct TICPP_API PMRDeleter {
    template <typename T>
    void operator()(T* item) {
        alloc.delete_object(item);
    }
    std::pmr::polymorphic_allocator<> alloc;
};
template <typename T>
using PMRUnique = std::unique_ptr<T, PMRDeleter>;

template <class T, class... Args>
PMRUnique<T> pmr_make_unique(std::pmr::polymorphic_allocator<> alloc, Args&&... args) {
    return PMRUnique<T>(alloc.new_object<T>(std::forward<Args>(args)...), PMRDeleter{alloc});
}

/**
 * TiXmlBase is a base class for every class in TinyXml.
 * It does little except to establish that TinyXml classes
 * can be printed and provide some utility functions.
 *
 * In XML, the document and elements can contain
 * other elements and other types of nodes.
 *
 * @verbatim
 * A Document can contain:	Element	(container or leaf)
 *                          Comment (leaf)
 *                          Unknown (leaf)
 *                          Declaration( leaf )
 *
 * An Element can contain:	Element (container or leaf)
 *                          Text	(leaf)
 *                          Attributes (not on tree)
 *                          Comment (leaf)
 *                          Unknown (leaf)
 *
 * A Declaration contains: Attributes (not on tree)
 * @endverbatim
 */

class TICPP_API TiXmlBase {
public:
    using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

    TiXmlBase() {}
    TiXmlBase(const TiXmlBase&) = delete;
    TiXmlBase& operator=(const TiXmlBase& base) = delete;
    virtual ~TiXmlBase() {}

    /** The world does not agree on whether white space should be kept or
     * not. In order to make everyone happy, these global, static functions
     * are provided to set whether or not TinyXml will condense all white space
     * into a single space or not. The default is to condense. Note changing this
     * value is not thread safe.
     */
    static void SetCondenseWhiteSpace(bool condense) { condenseWhiteSpace = condense; }

    /// Return the current white space setting.
    static bool IsWhiteSpaceCondensed() { return condenseWhiteSpace; }

    virtual const char* Parse(const char* p, TiXmlParsingData* data, allocator_type alloc) = 0;

    /** Expands entities in a string. Note this should not contain the tag's '<', '>', etc,
     * or they will be transformed into entities!
     */
    static void EncodeString(const std::string_view str, std::pmr::string& out);
    static void EncodeStringSlowPath(const std::string_view str, std::pmr::string& out);

    // Table that returns, for a given lead byte, the total number of bytes
    // in the UTF-8 sequence.

    // Bunch of unicode info at:
    //      http://www.unicode.org/faq/utf_bom.html
    // Including the basic of this table, which determines the #bytes in the
    // sequence from the lead byte. 1 placed for invalid sequences --
    // although the result will be junk, pass it through as much as possible.
    // Beware of the non-characters in UTF-8:
    //      ef bb bf (Microsoft "lead bytes")
    //      ef bf be
    //      ef bf bf

    // clang-format off
    static constexpr std::array<char, 256> utf8ByteTable = {
	//	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x00
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x10
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x20
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x30
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x40
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x50
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x60
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x70	End of ASCII range
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x80 0x80 to 0xc1 invalid
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x90
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0xa0
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0xb0
		1,	1,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// 0xc0 0xc2 to 0xdf 2 byte
		2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// 0xd0
		3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	// 0xe0 0xe0 to 0xef 3 byte
		4,	4,	4,	4,	4,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1	// 0xf0 0xf0 to 0xf4 4 byte, 0xf5 and higher invalid
    };
    // clang-format on

    inline static const char* SkipByteOrderMark(const char* p) {
        if (!p || !*p) {
            return nullptr;
        }

        constexpr unsigned char BOM_0 = 0xefU;
        constexpr unsigned char BOM_1 = 0xbbU;
        constexpr unsigned char BOM_2 = 0xbfU;

        const unsigned char* pU = (const unsigned char*)p;

        if (*(pU + 0) == BOM_0 && *(pU + 1) == BOM_1 && *(pU + 2) == BOM_2) {
            p += 3;
        } else if (*(pU + 0) == BOM_0 && *(pU + 1) == 0xbfU && *(pU + 2) == 0xbeU) {
            p += 3;
        } else if (*(pU + 0) == BOM_0 && *(pU + 1) == 0xbfU && *(pU + 2) == 0xbfU) {
            p += 3;
        }

        return p;
    }

    inline static const char* SkipWhiteSpace(const char* p) {
        if (!p || !*p) {
            return nullptr;
        }
        // Still using old rules for white space.
        while (*p && IsWhiteSpace(*p)) {
            ++p;
        }
        return p;
    }
    inline static bool IsWhiteSpace(char c) {
        return (isspace((unsigned char)c) || c == '\n' || c == '\r');
    }
    inline static bool IsWhiteSpace(int c) {
        if (c < 256) return IsWhiteSpace((char)c);
        return false;  // Again, only truly correct for English/Latin...but usually works.
    }

    /*	Reads an XML name into the string provided. Returns
     * a pointer just past the last character of the name,
     * or 0 if the function has an error.
     */
    static const char* ReadName(const char* p, std::pmr::string* name);

    /*	Reads text. Returns a pointer past the given end tag.
     * Wickedly complex options, but it keeps the (sensitive) code in one place.
     */
    static const char* ReadText(const char* in,          // where to start
                                std::pmr::string* text,  // the string read
                                bool ignoreWhiteSpace,   // whether to keep the white space
                                const char* endTag,      // what ends this text
                                bool ignoreCase);        // whether to ignore case in the end tag

    static const char* ReadQuotedText(const char* in, std::pmr::string* text,
                                      TiXmlParsingData* data);

    static const char* ReadNameValue(const char* in, std::pmr::string* name,
                                     std::pmr::string* value, TiXmlParsingData* data);

    // If an entity has been found, transform it into a character.
    static const char* GetEntity(const char* in, char* value, int* length);

    // Get a character, while interpreting entities.
    // The length can be from 0 to 4 bytes.
    inline static const char* GetChar(const char* p, char* value, int* length) {
        assert(p);
        if (*p == '&') return GetEntity(p, value, length);

        *length = utf8ByteTable[*p];
        assert(*length >= 0 && *length < 5);

        if (*length == 1) {
            *value = *p;
            return p + 1;
        } else if (*length) {
            for (int i = 0; p[i] && i < *length; ++i) {
                value[i] = p[i];
            }
            return p + (*length);
        } else {
            // Not valid text.
            return nullptr;
        }
    }

    // Check that s starts with the same chars as the concatenation of args
    template <typename... Ts>
    bool StrEquals(const char* s, const Ts&... args) {
        const auto fun = [&](auto&& arg) {
            auto res = std::strncmp(s, arg.data(), arg.size());
            if (res != 0) {
                return false;
            } else {
                s += arg.size();
                return true;
            }
        };

        return (fun(args) && ...);
    };

    // Return true if the next characters in the stream are any of the endTag sequences.
    // Ignore case only works for english, and should only be relied on when comparing
    // to English words: StringEqual( p, "version", true ) is fine.
    static bool StringEqual(const char* p, const char* endTag, bool ignoreCase);

    // None of these methods are reliable for any language except English.
    // Good for approximation, not great for accuracy.
    static bool IsAlpha(int anyByte);
    static bool IsAlphaNum(int anyByte);
    inline static int ToLower(int v) {
        if (v < 128) return tolower(v);
        return v;
    }
    static void ConvertUTF32ToUTF8(unsigned long input, char* output, int* length);

private:
    struct Entity {
        std::string_view str;
        char chr;
    };
    // Note the "PutString" hardcodes the same list. This
    // is less flexible than it appears. Changing the entries
    // or order will break putstring.
    static constexpr std::array<Entity, 5> entity = {
        {{"&amp;", '&'}, {"&lt;", '<'}, {"&gt;", '>'}, {"&quot;", '\"'}, {"&apos;", '\''}}};
    static bool condenseWhiteSpace;
};

inline void TiXmlBase::EncodeString(const std::string_view str, std::pmr::string& out) {

    // Fast path where we don't have any thing to encode
    // avoid c < 32, " = 34, & = 38, ' = 39, < = 60, > = 62,
    if (std::all_of(str.begin(), str.end(), [](char c) { return c >= 40 && c != 60 && c != 62; })) {
        out.append(str);
    } else {
        EncodeStringSlowPath(str, out);
    }
}
