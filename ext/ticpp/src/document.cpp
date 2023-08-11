#include <ticpp/document.h>
#include <ticpp/parsingdata.h>
#include <ticpp/declaration.h>


#include <ostream>

#if defined(WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// FIX: unicode paths
namespace {

#if defined(_WIN32)
std::wstring toWstring(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
    std::wstring result(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], size_needed);
    return result;
}
#endif

}  // namespace

// Microsoft compiler security
FILE* TiXmlFOpen(const char* filename, const char* mode) {
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    FILE* fp = 0;
    // errno_t err = fopen_s( &fp, filename, mode );

    // FIX: unicode paths
    errno_t err = _wfopen_s(&fp, toWstring(filename).c_str(), toWstring(mode).c_str());

    if (!err && fp) return fp;
    return 0;
#else
    return fopen(filename, mode);
#endif
}

TiXmlDocument::TiXmlDocument() : TiXmlNode(TiXmlNode::DOCUMENT) {
    tabsize = 4;
    useMicrosoftBOM = false;
    ClearError();
}

TiXmlDocument::TiXmlDocument(const char* documentName) : TiXmlNode(TiXmlNode::DOCUMENT) {
    tabsize = 4;
    useMicrosoftBOM = false;
    value = documentName;
    ClearError();
}

TiXmlDocument::TiXmlDocument(const std::string& documentName) : TiXmlNode(TiXmlNode::DOCUMENT) {
    tabsize = 4;
    useMicrosoftBOM = false;
    value = documentName;
    ClearError();
}

TiXmlDocument::TiXmlDocument(const TiXmlDocument& copy) : TiXmlNode(TiXmlNode::DOCUMENT) {
    copy.CopyTo(this);
}

void TiXmlDocument::operator=(const TiXmlDocument& copy) {
    Clear();
    copy.CopyTo(this);
}

bool TiXmlDocument::LoadFile(TiXmlEncoding encoding) {
    // See STL_STRING_BUG below.
    // StringToBuffer buf( value );

    return LoadFile(Value(), encoding);
}

bool TiXmlDocument::SaveFile() const {
    // See STL_STRING_BUG below.
    //	StringToBuffer buf( value );
    //
    //	if ( buf.buffer && SaveFile( buf.buffer ) )
    //		return true;
    //
    //	return false;
    return SaveFile(Value());
}

bool TiXmlDocument::LoadFile(const char* _filename, TiXmlEncoding encoding) {
    // There was a really terrifying little bug here. The code:
    //		value = filename
    // in the STL case, cause the assignment method of the std::string to
    // be called. What is strange, is that the std::string had the same
    // address as it's c_str() method, and so bad things happen. Looks
    // like a bug in the Microsoft STL implementation.
    // Add an extra string to avoid the crash.
    std::string filename(_filename);
    value = filename;

    // reading in binary mode so that tinyxml can normalize the EOL
    FILE* file = TiXmlFOpen(value.c_str(), "rb");

    if (file) {
        bool result = LoadFile(file, encoding);
        fclose(file);
        return result;
    } else {
        SetError(TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN);
        return false;
    }
}

bool TiXmlDocument::LoadFile(FILE* file, TiXmlEncoding encoding) {
    if (!file) {
        SetError(TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN);
        return false;
    }

    // Delete the existing data:
    Clear();
    location.Clear();

    // Get the file size, so we can pre-allocate the string. HUGE speed impact.
    long length = 0;
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Strange case, but good to handle up front.
    if (length <= 0) {
        SetError(TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN);
        return false;
    }

    // If we have a file, assume it is all one big XML file, and read it in.
    // The document parser may decide the document ends sooner than the entire file, however.
    std::string data;
    data.reserve(length);

    // Subtle bug here. TinyXml did use fgets. But from the XML spec:
    // 2.11 End-of-Line Handling
    // <snip>
    // <quote>
    // ...the XML processor MUST behave as if it normalized all line breaks in external
    // parsed entities (including the document entity) on input, before parsing, by translating
    // both the two-character sequence #xD #xA and any #xD that is not followed by #xA to
    // a single #xA character.
    // </quote>
    //
    // It is not clear fgets does that, and certainly isn't clear it works cross platform.
    // Generally, you expect fgets to translate from the convention of the OS to the c/unix
    // convention, and not work generally.

    /*
    while( fgets( buf, sizeof(buf), file ) )
    {
            data += buf;
    }
    */

    char* buf = new char[length + 1];
    buf[0] = 0;

    if (fread(buf, length, 1, file) != 1) {
        delete[] buf;
        SetError(TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN);
        return false;
    }

    const char* lastPos = buf;
    const char* p = buf;

    buf[length] = 0;
    while (*p) {
        assert(p < (buf + length));
        if (*p == 0xa) {
            // Newline character. No special rules for this. Append all the characters
            // since the last string, and include the newline.
            data.append(lastPos, (p - lastPos + 1));  // append, include the newline
            ++p;                                      // move past the newline
            lastPos = p;                              // and point to the new buffer (may be 0)
            assert(p <= (buf + length));
        } else if (*p == 0xd) {
            // Carriage return. Append what we have so far, then
            // handle moving forward in the buffer.
            if ((p - lastPos) > 0) {
                data.append(lastPos, p - lastPos);  // do not add the CR
            }
            data += (char)0xa;  // a proper newline

            if (*(p + 1) == 0xa) {
                // Carriage return - new line sequence
                p += 2;
                lastPos = p;
                assert(p <= (buf + length));
            } else {
                // it was followed by something else...that is presumably characters again.
                ++p;
                lastPos = p;
                assert(p <= (buf + length));
            }
        } else {
            ++p;
        }
    }
    // Handle any left over characters.
    if (p - lastPos) {
        data.append(lastPos, p - lastPos);
    }
    delete[] buf;
    buf = 0;

    Parse(data.c_str(), 0, encoding);

    if (Error())
        return false;
    else
        return true;
}

bool TiXmlDocument::SaveFile(const char* filename) const {
    // The old c stuff lives on...
    FILE* fp = TiXmlFOpen(filename, "w");
    if (fp) {
        bool result = SaveFile(fp);
        fclose(fp);
        return result;
    }
    return false;
}

bool TiXmlDocument::SaveFile(FILE* fp) const {
    if (useMicrosoftBOM) {
        const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
        const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
        const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

        fputc(TIXML_UTF_LEAD_0, fp);
        fputc(TIXML_UTF_LEAD_1, fp);
        fputc(TIXML_UTF_LEAD_2, fp);
    }
    Print(fp, 0);
    return (ferror(fp) == 0);
}

void TiXmlDocument::CopyTo(TiXmlDocument* target) const {
    TiXmlNode::CopyTo(target);

    target->error = error;
    target->errorId = errorId;
    target->errorDesc = errorDesc;
    target->tabsize = tabsize;
    target->errorLocation = errorLocation;
    target->useMicrosoftBOM = useMicrosoftBOM;

    TiXmlNode* node = 0;
    for (node = firstChild; node; node = node->NextSibling()) {
        target->LinkEndChild(node->Clone());
    }
}

TiXmlNode* TiXmlDocument::Clone() const {
    TiXmlDocument* clone = new TiXmlDocument();
    if (!clone) return 0;

    CopyTo(clone);
    return clone;
}

void TiXmlDocument::Print(FILE* cfile, int depth) const {
    assert(cfile);
    for (const TiXmlNode* node = FirstChild(); node; node = node->NextSibling()) {
        node->Print(cfile, depth);
        fprintf(cfile, "\n");
    }
}

bool TiXmlDocument::Accept(TiXmlVisitor* visitor) const {
    if (visitor->VisitEnter(*this)) {
        for (const TiXmlNode* node = FirstChild(); node; node = node->NextSibling()) {
            if (!node->Accept(visitor)) break;
        }
    }
    return visitor->VisitExit(*this);
}

void TiXmlDocument::StreamIn(std::istream* in, std::string* tag) {
    // The basic issue with a document is that we don't know what we're
    // streaming. Read something presumed to be a tag (and hope), then
    // identify it, and call the appropriate stream method on the tag.
    //
    // This "pre-streaming" will never read the closing ">" so the
    // sub-tag can orient itself.

    if (!StreamTo(in, '<', tag)) {
        SetError(TIXML_ERROR_PARSING_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN);
        return;
    }

    while (in->good()) {
        int tagIndex = (int)tag->length();
        while (in->good() && in->peek() != '>') {
            int c = in->get();
            if (c <= 0) {
                SetError(TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN);
                break;
            }
            (*tag) += (char)c;
        }

        if (in->good()) {
            // We now have something we presume to be a node of
            // some sort. Identify it, and call the node to
            // continue streaming.
            TiXmlNode* node = Identify(tag->c_str() + tagIndex, TIXML_DEFAULT_ENCODING);

            if (node) {
                node->StreamIn(in, tag);
                bool isElement = node->ToElement() != 0;
                delete node;
                node = 0;

                // If this is the root element, we're done. Parsing will be
                // done by the >> operator.
                if (isElement) {
                    return;
                }
            } else {
                SetError(TIXML_ERROR, 0, 0, TIXML_ENCODING_UNKNOWN);
                return;
            }
        }
    }
    // We should have returned sooner.
    SetError(TIXML_ERROR, 0, 0, TIXML_ENCODING_UNKNOWN);
}

const char* TiXmlDocument::Parse(const char* p, TiXmlParsingData* prevData,
                                 TiXmlEncoding encoding) {
    ClearError();

    // Parse away, at the document level. Since a document
    // contains nothing but other tags, most of what happens
    // here is skipping white space.
    if (!p || !*p) {
        SetError(TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN);
        return 0;
    }

    // Note that, for a document, this needs to come
    // before the while space skip, so that parsing
    // starts from the pointer we are given.
    location.Clear();
    if (prevData) {
        location.row = prevData->cursor.row;
        location.col = prevData->cursor.col;
    } else {
        location.row = 0;
        location.col = 0;
    }
    TiXmlParsingData data(p, TabSize(), location.row, location.col);
    location = data.Cursor();

    if (encoding == TIXML_ENCODING_UNKNOWN) {
        // Check for the Microsoft UTF-8 lead bytes.
        const unsigned char* pU = (const unsigned char*)p;
        if (*(pU + 0) && *(pU + 0) == TIXML_UTF_LEAD_0 && *(pU + 1) &&
            *(pU + 1) == TIXML_UTF_LEAD_1 && *(pU + 2) && *(pU + 2) == TIXML_UTF_LEAD_2) {
            encoding = TIXML_ENCODING_UTF8;
            useMicrosoftBOM = true;
        }
    }

    p = SkipWhiteSpace(p, encoding);
    if (!p) {
        SetError(TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN);
        return 0;
    }

    while (p && *p) {
        TiXmlNode* node = Identify(p, encoding);
        if (node) {
            p = node->Parse(p, &data, encoding);
            LinkEndChild(node);
        } else {
            break;
        }

        // Did we get encoding info?
        if (encoding == TIXML_ENCODING_UNKNOWN && node->ToDeclaration()) {
            TiXmlDeclaration* dec = node->ToDeclaration();
            const char* enc = dec->Encoding();
            assert(enc);

            if (*enc == 0)
                encoding = TIXML_ENCODING_UTF8;
            else if (StringEqual(enc, "UTF-8", true, TIXML_ENCODING_UNKNOWN))
                encoding = TIXML_ENCODING_UTF8;
            else if (StringEqual(enc, "UTF8", true, TIXML_ENCODING_UNKNOWN))
                encoding = TIXML_ENCODING_UTF8;  // incorrect, but be nice
            else
                encoding = TIXML_ENCODING_LEGACY;
        }

        p = SkipWhiteSpace(p, encoding);
    }

    // Was this empty?
    if (!firstChild) {
        SetError(TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, encoding);
        return 0;
    }

    // All is well.
    return p;
}

void TiXmlDocument::SetError(int err, const char* pError, TiXmlParsingData* data,
                             TiXmlEncoding encoding) {
    // The first error in a chain is more accurate - don't set again!
    if (error) return;

    assert(err > 0 && err < TIXML_ERROR_STRING_COUNT);
    error = true;
    errorId = err;
    errorDesc = errorString[errorId];

    errorLocation.Clear();
    if (pError && data) {
        data->Stamp(pError, encoding);
        errorLocation = data->Cursor();
    }
}
