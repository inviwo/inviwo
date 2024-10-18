#include <ticpp/text.h>

#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

#include <cstring>

void TiXmlText::Print(FILE* cfile, int depth) const {
    assert(cfile);
    if (cdata) {
        int i;
        fprintf(cfile, "\n");
        for (i = 0; i < depth; i++) {
            fprintf(cfile, "    ");
        }
        fprintf(cfile, "<![CDATA[%s]]>\n", value.c_str());  // unformatted output
    } else {
        std::string buffer;
        EncodeString(value, &buffer);
        fprintf(cfile, "%s", buffer.c_str());
    }
}

void TiXmlText::CopyTo(TiXmlText* target) const {
    TiXmlNode::CopyTo(target);
    target->cdata = cdata;
}

bool TiXmlText::Accept(TiXmlVisitor* visitor) const { return visitor->Visit(*this); }

TiXmlNode* TiXmlText::Clone() const {
    TiXmlText* clone = 0;
    clone = new TiXmlText("");

    if (!clone) return 0;

    CopyTo(clone);
    return clone;
}


void TiXmlText::StreamIn(std::istream* in, std::string* tag) {
    while (in->good()) {
        int c = in->peek();
        if (!cdata && (c == '<')) {
            return;
        }
        if (c <= 0) {
            TiXmlDocument* document = GetDocument();
            if (document)
                document->SetError(TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN);
            return;
        }

        (*tag) += (char)c;
        in->get();  // "commits" the peek made above

        if (cdata && c == '>' && tag->size() >= 3) {
            size_t len = tag->size();
            if ((*tag)[len - 2] == ']' && (*tag)[len - 3] == ']') {
                // terminator of cdata.
                return;
            }
        }
    }
}

const char* TiXmlText::Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding) {
    value = "";
    TiXmlDocument* document = GetDocument();

    if (data) {
        data->Stamp(p, encoding);
        location = data->Cursor();
    }

    const char* const startTag = "<![CDATA[";
    const char* const endTag = "]]>";

    if (cdata || StringEqual(p, startTag, false)) {
        cdata = true;

        if (!StringEqual(p, startTag, false)) {
            document->SetError(TIXML_ERROR_PARSING_CDATA, p, data, encoding);
            return 0;
        }
        p += strlen(startTag);

        // Keep all the white space, ignore the encoding, etc.
        while (p && *p && !StringEqual(p, endTag, false)) {
            value += *p;
            ++p;
        }

        std::string dummy;
        p = ReadText(p, &dummy, false, endTag, false, encoding);
        return p;
    } else {
        bool ignoreWhite = true;

        const char* end = "<";
        p = ReadText(p, &value, ignoreWhite, end, false, encoding);
        if (p) return p - 1;  // don't truncate the '<'
        return 0;
    }
}

bool TiXmlText::Blank() const {
    for (unsigned i = 0; i < value.length(); i++)
        if (!IsWhiteSpace(value[i])) return false;
    return true;
}
