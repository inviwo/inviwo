#include <ticpp/base.h>
#include <ticpp/parsingdata.h>

#include <algorithm>
#include <iterator>
#include <fmt/format.h>

namespace {
constexpr std::array<std::string_view, static_cast<int>(TiXmlErrorCode::TIXML_ERROR_STRING_COUNT)>
    TiXmlErrorStrings = {
        "No error",
        "Error",
        "Failed to open file",
        "Error parsing Element.",
        "Failed to read Element name",
        "Error reading Element value.",
        "Error reading Attributes.",
        "Error: empty tag.",
        "Error reading end tag.",
        "Error parsing Unknown.",
        "Error parsing Comment.",
        "Error parsing Declaration.",
        "Error document empty.",
        "Error null (0) or unexpected EOF found in input stream.",
        "Error parsing CDATA.",
        "Error when TiXmlDocument added to document, because TiXmlDocument can only be at the "
        "root.",
        "Error trying to add duplicate attribute to element"};

std::string formatError(TiXmlErrorCode err, const char* errorLocation,
                        TiXmlParsingData* parseData) {
    if (errorLocation && parseData) {
        parseData->Stamp(errorLocation);
        TiXmlCursor location = parseData->Cursor();
        return fmt::format("{} Line: {} Column: {}", TiXmlErrorStrings[static_cast<int>(err)],
                           location.row + 1, location.col + 1);
    } else {
        return std::string{TiXmlErrorStrings[static_cast<int>(err)]};
    }
}

}  // namespace

TiXmlError::TiXmlError(TiXmlErrorCode err, const char* errorLocation, TiXmlParsingData* parseData)
    : std::runtime_error{formatError(err, errorLocation, parseData)}
    , errorCode{err}
    , location{[&]() {
        if (errorLocation && parseData) {
            parseData->Stamp(errorLocation);
            return parseData->Cursor();
        } else {
            return TiXmlCursor{};
        }
    }()} {}

bool TiXmlBase::condenseWhiteSpace = true;

void TiXmlBase::ConvertUTF32ToUTF8(unsigned long input, char* output, int* length) {
    const unsigned long BYTE_MASK = 0xBF;
    const unsigned long BYTE_MARK = 0x80;
    const unsigned long FIRST_BYTE_MARK[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

    if (input < 0x80)
        *length = 1;
    else if (input < 0x800)
        *length = 2;
    else if (input < 0x10000)
        *length = 3;
    else if (input < 0x200000)
        *length = 4;
    else {
        *length = 0;
        return;
    }  // This code won't covert this correctly anyway.

    output += *length;

    // Scary scary fall throughs.
    switch (*length) {
        case 4:
            --output;
            *output = (char)((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
            [[fallthrough]];
        case 3:
            --output;
            *output = (char)((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
            [[fallthrough]];
        case 2:
            --output;
            *output = (char)((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
            [[fallthrough]];
        case 1:
            --output;
            *output = (char)(input | FIRST_BYTE_MARK[*length]);
    }
}

bool TiXmlBase::IsAlpha(int anyByte) {
    // This will only work for low-ascii, everything else is assumed to be a valid
    // letter. I'm not sure this is the best approach, but it is quite tricky trying
    // to figure out alphabetical vs. not across encoding. So take a very
    // conservative approach.

    if (anyByte < 127) {
        return std::isalpha(anyByte);
    } else {
        return true;  // What else to do? The unicode set is huge...get the english ones right.
    }
}

bool TiXmlBase::IsAlphaNum(int anyByte) {
    // This will only work for low-ascii, everything else is assumed to be a valid
    // letter. I'm not sure this is the best approach, but it is quite tricky trying
    // to figure out alphabetical vs. not across encoding. So take a very
    // conservative approach.

    if (anyByte < 127) {
        return std::isalnum(anyByte);
    } else {
        return true;  // What else to do? The unicode set is huge...get the english ones right.
    }
}

// One of TinyXML's more performance demanding functions. Try to keep the memory overhead down. The
// "assign" optimization removes over 10% of the execution time.
const char* TiXmlBase::ReadName(const char* p, std::pmr::string* name) {
    name->clear();
    assert(p);

    // Names start with letters or underscores.
    // Of course, in unicode, tinyxml has no idea what a letter *is*. The
    // algorithm is generous.
    //
    // After that, they can be letters, underscores, numbers,
    // hyphens, or colons. (Colons are valid only for namespaces,
    // but tinyxml can't tell namespaces from names.)
    if (p && *p && (IsAlpha(*p) || *p == '_')) {
        const char* start = p;
        while (p && *p && (IsAlphaNum(*p) || *p == '_' || *p == '-' || *p == '.' || *p == ':')) {
            ++p;
        }
        if (p - start > 0) {
            name->assign(start, p - start);
        }
        return p;
    }
    return nullptr;
}

const char* TiXmlBase::GetEntity(const char* p, char* value, int* length) {
    // Presume an entity, and pull it out.
    *length = 0;

    if (*(p + 1) && *(p + 1) == '#' && *(p + 2)) {
        unsigned long ucs = 0;
        ptrdiff_t delta = 0;
        unsigned mult = 1;

        if (*(p + 2) == 'x') {
            // Hexadecimal.
            if (!*(p + 3)) return 0;

            const char* q = p + 3;
            q = strchr(q, ';');

            if (!q || !*q) return 0;

            delta = q - p;
            --q;

            while (*q != 'x') {
                if (*q >= '0' && *q <= '9')
                    ucs += mult * (*q - '0');
                else if (*q >= 'a' && *q <= 'f')
                    ucs += mult * (*q - 'a' + 10);
                else if (*q >= 'A' && *q <= 'F')
                    ucs += mult * (*q - 'A' + 10);
                else
                    return 0;
                mult *= 16;
                --q;
            }
        } else {
            // Decimal.
            if (!*(p + 2)) return 0;

            const char* q = p + 2;
            q = strchr(q, ';');

            if (!q || !*q) return 0;

            delta = q - p;
            --q;

            while (*q != '#') {
                if (*q >= '0' && *q <= '9')
                    ucs += mult * (*q - '0');
                else
                    return 0;
                mult *= 10;
                --q;
            }
        }

        // convert the UCS to UTF-8
        ConvertUTF32ToUTF8(ucs, value, length);

        return p + delta + 1;
    }

    // Now try to match it.
    for (size_t i = 0; i < entity.size(); ++i) {
        if (strncmp(entity[i].str.data(), p, entity[i].str.size()) == 0) {
            *value = entity[i].chr;
            *length = 1;
            return (p + entity[i].str.size());
        }
    }

    // So it wasn't an entity, its unrecognized, or something like that.
    *value = *p;  // Don't put back the last one, since we return it!
    //*length = 1;	// Leave unrecognized entities - this doesn't really work.
    // Just writes strange XML.
    return p + 1;
}

bool TiXmlBase::StringEqual(const char* p, const char* tag, bool ignoreCase) {
    assert(p);
    assert(tag);
    if (!p || !*p) {
        assert(0);
        return false;
    }

    const char* q = p;

    if (ignoreCase) {
        while (*q && *tag && ToLower(*q) == ToLower(*tag)) {
            ++q;
            ++tag;
        }

        if (*tag == 0) return true;
    } else {
        while (*q && *tag && *q == *tag) {
            ++q;
            ++tag;
        }

        if (*tag == 0)  // Have we found the end of the tag, and everything equal?
            return true;
    }
    return false;
}

const char* TiXmlBase::ReadText(const char* p, std::pmr::string* text, bool trimWhiteSpace,
                                const char* endTag, bool caseInsensitive) {
    *text = "";
    if (!trimWhiteSpace          // certain tags always keep whitespace
        || !condenseWhiteSpace)  // if true, whitespace is always kept
    {
        // Keep all the white space.
        while (p && *p && !StringEqual(p, endTag, caseInsensitive)) {
            int len;
            char cArr[4] = {0, 0, 0, 0};
            p = GetChar(p, cArr, &len);
            text->append(cArr, len);
        }
    } else {
        bool whitespace = false;

        // Remove leading white space:
        p = SkipWhiteSpace(p);
        while (p && *p && !StringEqual(p, endTag, caseInsensitive)) {
            if (*p == '\r' || *p == '\n') {
                whitespace = true;
                ++p;
            } else if (IsWhiteSpace(*p)) {
                whitespace = true;
                ++p;
            } else {
                // If we've found whitespace, add it before the
                // new character. Any whitespace just becomes a space.
                if (whitespace) {
                    (*text) += ' ';
                    whitespace = false;
                }
                int len;
                char cArr[4] = {0, 0, 0, 0};
                p = GetChar(p, cArr, &len);
                if (len == 1)
                    (*text) += cArr[0];  // more efficient
                else
                    text->append(cArr, len);
            }
        }
    }
    if (p) p += strlen(endTag);
    return p;
}

namespace {

template <const char endTag>
const char* ReadQuote(const char* p, std::pmr::string* dest) {
    dest->clear();

    while (p && *p && *p != endTag) {
        const char* start = p;
        while (p && *p && *p != endTag && *p != '&') {
            ++p;
        }
        if (p - start > 0) {
            dest->append(start, p - start);
        }
        if (*p == '&') {
            int len{0};
            std::array<char, 4> buffer{};
            p = TiXmlBase::GetEntity(p, buffer.data(), &len);
            dest->append(buffer.data(), len);
        }
    }

    if (p && *p == endTag) {
        return ++p;
    } else {
        return p;
    }
}

}  // namespace
const char* TiXmlBase::ReadQuotedText(const char* p, std::pmr::string* dest,
                                      TiXmlParsingData* data) {

    constexpr char singleQuote = '\'';
    constexpr char doubleQuote = '\"';

    const char* pErr = p;

    if (*p == singleQuote) {
        p = ReadQuote<singleQuote>(++p, dest);  // single quote in string
    } else if (*p == doubleQuote) {
        p = ReadQuote<doubleQuote>(++p, dest);  // double quote in string
    } else {
        // All attribute values should be in single or double quotes.
        // But this is such a common error that the parser will try
        // its best, even without them.
        dest->clear();
        while (p && *p                                                      // existence
               && !TiXmlBase::IsWhiteSpace(*p) && *p != '\n' && *p != '\r'  // whitespace
               && *p != '/' && *p != '>')                                   // tag end
        {
            if (*p == singleQuote || *p == doubleQuote) {
                // [ 1451649 ] Attribute values with trailing quotes not handled correctly
                // We did not have an opening quote but seem to have a
                // closing one. Give up and throw an error.
                throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_READING_ATTRIBUTES, p, data);
            }
            *dest += *p;
            ++p;
        }
    }

    if (!p || !*p) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_READING_ATTRIBUTES, pErr, data);
    }

    return p;
}

const char* TiXmlBase::ReadNameValue(const char* p, std::pmr::string* name, std::pmr::string* value,
                                     TiXmlParsingData* data) {
    // Read the name, the '=' and the value.

    const char* pErr = p;
    p = TiXmlBase::ReadName(p, name);
    if (!p || !*p) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_READING_ATTRIBUTES, pErr, data);
    }

    p = TiXmlBase::SkipWhiteSpace(p);
    if (!p || !*p || *p != '=') {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_READING_ATTRIBUTES, p, data);
    }

    ++p;  // skip '='
    p = TiXmlBase::SkipWhiteSpace(p);
    if (!p || !*p) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_READING_ATTRIBUTES, p, data);
    }

    p = TiXmlBase::ReadQuotedText(p, value, data);

    return p;
}

void TiXmlBase::EncodeStringSlowPath(const std::string_view str, std::pmr::string& out) {
    size_t i = 0;
    while (i < str.length()) {
        const auto c = str[i];

        if (c == '&' && i + 2 < str.length() && str[i + 1] == '#' && str[i + 2] == 'x') {
            // Hexadecimal character reference.
            // Pass through unchanged.
            // &#xA9;	-- copyright symbol, for example.
            //
            // The -1 is a bug fix from Rob Laveaux. It keeps
            // an overflow from happening if there is no ';'.
            // There are actually 2 ways to exit this loop -
            // while fails (error case) and break (semicolon found).
            // However, there is no mechanism (currently) for
            // this function to return an error.
            while (i + 1 < str.length()) {
                out.push_back(str[i]);
                ++i;
                if (str[i] == ';') break;
            }
        } else if (c == entity[0].chr) {
            out.append(entity[0].str);
            ++i;
        } else if (c == entity[1].chr) {
            out.append(entity[1].str);
            ++i;
        } else if (c == entity[2].chr) {
            out.append(entity[2].str);
            ++i;
        } else if (c == entity[3].chr) {
            out.append(entity[3].str);
            ++i;
        } else if (c == entity[4].chr) {
            out.append(entity[4].str);
            ++i;
        } else if (c >= 0 && c < 32) {
            // Easy pass at non-alpha/numeric/symbol
            // Below 32 is symbolic.
            fmt::format_to(std::back_inserter(out), "&#x{:02X};", c);
            ++i;
        } else {
            out.push_back(c);
            ++i;
        }
    }
}
