#include <ticpp/base.h>

#include <istream>

bool TiXmlBase::condenseWhiteSpace = true;

void TiXmlBase::EncodeString(const std::string& str, std::string* outString) {
    int i = 0;

    while (i < (int)str.length()) {
        unsigned char c = (unsigned char)str[i];

        if (c == '&' && i < ((int)str.length() - 2) && str[i + 1] == '#' && str[i + 2] == 'x') {
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
            while (i < (int)str.length() - 1) {
                outString->append(str.c_str() + i, 1);
                ++i;
                if (str[i] == ';') break;
            }
        } else if (c == '&') {
            outString->append(entity[0].str, entity[0].strLength);
            ++i;
        } else if (c == '<') {
            outString->append(entity[1].str, entity[1].strLength);
            ++i;
        } else if (c == '>') {
            outString->append(entity[2].str, entity[2].strLength);
            ++i;
        } else if (c == '\"') {
            outString->append(entity[3].str, entity[3].strLength);
            ++i;
        } else if (c == '\'') {
            outString->append(entity[4].str, entity[4].strLength);
            ++i;
        } else if (c < 32) {
            // Easy pass at non-alpha/numeric/symbol
            // Below 32 is symbolic.
            char buf[32];

#if defined(TIXML_SNPRINTF)
            TIXML_SNPRINTF(buf, sizeof(buf), "&#x%02X;", (unsigned)(c & 0xff));
#else
            sprintf(buf, "&#x%02X;", (unsigned)(c & 0xff));
#endif

            //*ME:	warning C4267: convert 'size_t' to 'int'
            //*ME:	Int-Cast to make compiler happy ...
            outString->append(buf, (int)strlen(buf));
            ++i;
        } else {
            // char realc = (char) c;
            // outString->append( &realc, 1 );
            *outString += (char)c;  // somewhat more efficient function call.
            ++i;
        }
    }
}

const char* TiXmlBase::errorString[TIXML_ERROR_STRING_COUNT] = {
    "No error",
    "Error",
    "Failed to open file",
    "Memory allocation failed.",
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
    "Error when TiXmlDocument added to document, because TiXmlDocument can only be at the root.",
};



// Note tha "PutString" hardcodes the same list. This
// is less flexible than it appears. Changing the entries
// or order will break putstring.
TiXmlBase::Entity TiXmlBase::entity[NUM_ENTITY] = {{"&amp;", 5, '&'},
                                                   {"&lt;", 4, '<'},
                                                   {"&gt;", 4, '>'},
                                                   {"&quot;", 6, '\"'},
                                                   {"&apos;", 6, '\''}};

// Bunch of unicode info at:
//		http://www.unicode.org/faq/utf_bom.html
// Including the basic of this table, which determines the #bytes in the
// sequence from the lead byte. 1 placed for invalid sequences --
// although the result will be junk, pass it through as much as possible.
// Beware of the non-characters in UTF-8:
//				ef bb bf (Microsoft "lead bytes")
//				ef bf be
//				ef bf bf



// clang-format off
const int TiXmlBase::utf8ByteTable[256] = {
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
        case 3:
            --output;
            *output = (char)((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
        case 2:
            --output;
            *output = (char)((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
        case 1:
            --output;
            *output = (char)(input | FIRST_BYTE_MARK[*length]);
    }
}

int TiXmlBase::IsAlpha(unsigned char anyByte, TiXmlEncoding /*encoding*/) {
    // This will only work for low-ascii, everything else is assumed to be a valid
    // letter. I'm not sure this is the best approach, but it is quite tricky trying
    // to figure out alhabetical vs. not across encoding. So take a very
    // conservative approach.

    //	if ( encoding == TIXML_ENCODING_UTF8 )
    //	{
    if (anyByte < 127)
        return isalpha(anyByte);
    else
        return 1;  // What else to do? The unicode set is huge...get the english ones right.
    //	}
    //	else
    //	{
    //		return isalpha( anyByte );
    //	}
}

int TiXmlBase::IsAlphaNum(unsigned char anyByte, TiXmlEncoding /*encoding*/) {
    // This will only work for low-ascii, everything else is assumed to be a valid
    // letter. I'm not sure this is the best approach, but it is quite tricky trying
    // to figure out alhabetical vs. not across encoding. So take a very
    // conservative approach.

    //	if ( encoding == TIXML_ENCODING_UTF8 )
    //	{
    if (anyByte < 127)
        return isalnum(anyByte);
    else
        return 1;  // What else to do? The unicode set is huge...get the english ones right.
    //	}
    //	else
    //	{
    //		return isalnum( anyByte );
    //	}
}

const char* TiXmlBase::SkipWhiteSpace(const char* p, TiXmlEncoding encoding) {
    if (!p || !*p) {
        return 0;
    }
    if (encoding == TIXML_ENCODING_UTF8) {
        while (*p) {
            const unsigned char* pU = (const unsigned char*)p;

            // Skip the stupid Microsoft UTF-8 Byte order marks
            if (*(pU + 0) == TIXML_UTF_LEAD_0 && *(pU + 1) == TIXML_UTF_LEAD_1 &&
                *(pU + 2) == TIXML_UTF_LEAD_2) {
                p += 3;
                continue;
            } else if (*(pU + 0) == TIXML_UTF_LEAD_0 && *(pU + 1) == 0xbfU && *(pU + 2) == 0xbeU) {
                p += 3;
                continue;
            } else if (*(pU + 0) == TIXML_UTF_LEAD_0 && *(pU + 1) == 0xbfU && *(pU + 2) == 0xbfU) {
                p += 3;
                continue;
            }

            if (IsWhiteSpace(*p) || *p == '\n' ||
                *p == '\r')  // Still using old rules for white space.
                ++p;
            else
                break;
        }
    } else {
        while ((*p && IsWhiteSpace(*p)) || *p == '\n' || *p == '\r') ++p;
    }

    return p;
}

/*static*/ bool TiXmlBase::StreamWhiteSpace(std::istream* in, std::string* tag) {
    for (;;) {
        if (!in->good()) return false;

        int c = in->peek();
        // At this scope, we can't get to a document. So fail silently.
        if (!IsWhiteSpace(c) || c <= 0) return true;

        *tag += (char)in->get();
    }
}

/*static*/ bool TiXmlBase::StreamTo(std::istream* in, int character, std::string* tag) {
    // assert( character > 0 && character < 128 );	// else it won't work in utf-8
    while (in->good()) {
        int c = in->peek();
        if (c == character) return true;
        if (c <= 0)  // Silent failure: can't get document at this scope
            return false;

        in->get();
        *tag += (char)c;
    }
    return false;
}

// One of TinyXML's more performance demanding functions. Try to keep the memory overhead down. The
// "assign" optimization removes over 10% of the execution time.
//
const char* TiXmlBase::ReadName(const char* p, std::string* name, TiXmlEncoding encoding) {
    // Oddly, not supported on some comilers,
    // name->clear();
    // So use this:
    *name = "";
    assert(p);

    // Names start with letters or underscores.
    // Of course, in unicode, tinyxml has no idea what a letter *is*. The
    // algorithm is generous.
    //
    // After that, they can be letters, underscores, numbers,
    // hyphens, or colons. (Colons are valid ony for namespaces,
    // but tinyxml can't tell namespaces from names.)
    if (p && *p && (IsAlpha((unsigned char)*p, encoding) || *p == '_')) {
        const char* start = p;
        while (p && *p &&
               (IsAlphaNum((unsigned char)*p, encoding) || *p == '_' || *p == '-' || *p == '.' ||
                *p == ':')) {
            //(*name) += *p; // expensive
            ++p;
        }
        if (p - start > 0) {
            name->assign(start, p - start);
        }
        return p;
    }
    return 0;
}

const char* TiXmlBase::GetEntity(const char* p, char* value, int* length, TiXmlEncoding encoding) {
    // Presume an entity, and pull it out.
    std::string ent;
    int i;
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
        if (encoding == TIXML_ENCODING_UTF8) {
            // convert the UCS to UTF-8
            ConvertUTF32ToUTF8(ucs, value, length);
        } else {
            *value = (char)ucs;
            *length = 1;
        }
        return p + delta + 1;
    }

    // Now try to match it.
    for (i = 0; i < NUM_ENTITY; ++i) {
        if (strncmp(entity[i].str, p, entity[i].strLength) == 0) {
            assert(strlen(entity[i].str) == entity[i].strLength);
            *value = entity[i].chr;
            *length = 1;
            return (p + entity[i].strLength);
        }
    }

    // So it wasn't an entity, its unrecognized, or something like that.
    *value = *p;  // Don't put back the last one, since we return it!
    //*length = 1;	// Leave unrecognized entities - this doesn't really work.
    // Just writes strange XML.
    return p + 1;
}

bool TiXmlBase::StringEqual(const char* p, const char* tag, bool ignoreCase,
                            TiXmlEncoding encoding) {
    assert(p);
    assert(tag);
    if (!p || !*p) {
        assert(0);
        return false;
    }

    const char* q = p;

    if (ignoreCase) {
        while (*q && *tag && ToLower(*q, encoding) == ToLower(*tag, encoding)) {
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

const char* TiXmlBase::ReadText(const char* p, std::string* text, bool trimWhiteSpace,
                                const char* endTag, bool caseInsensitive, TiXmlEncoding encoding) {
    *text = "";
    if (!trimWhiteSpace          // certain tags always keep whitespace
        || !condenseWhiteSpace)  // if true, whitespace is always kept
    {
        // Keep all the white space.
        while (p && *p && !StringEqual(p, endTag, caseInsensitive, encoding)) {
            int len;
            char cArr[4] = {0, 0, 0, 0};
            p = GetChar(p, cArr, &len, encoding);
            text->append(cArr, len);
        }
    } else {
        bool whitespace = false;

        // Remove leading white space:
        p = SkipWhiteSpace(p, encoding);
        while (p && *p && !StringEqual(p, endTag, caseInsensitive, encoding)) {
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
                p = GetChar(p, cArr, &len, encoding);
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
