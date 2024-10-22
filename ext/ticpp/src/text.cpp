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
    TiXmlText* clone = new TiXmlText("");

    if (!clone) return nullptr;

    CopyTo(clone);
    return clone;
}

const char* TiXmlText::Parse(const char* p, TiXmlParsingData* data) {
    value = "";

    if (data) {
        data->Stamp(p);
        location = data->Cursor();
    }

    const char* const startTag = "<![CDATA[";
    const char* const endTag = "]]>";

    if (cdata || StringEqual(p, startTag, false)) {
        cdata = true;

        if (!StringEqual(p, startTag, false)) {
            TiXmlDocument* document = GetDocument();
            document->SetError(TIXML_ERROR_PARSING_CDATA, p, data);
            return nullptr;
        }
        p += strlen(startTag);

        // Keep all the white space, ignore the encoding, etc.
        while (p && *p && !StringEqual(p, endTag, false)) {
            value += *p;
            ++p;
        }

        std::string dummy;
        p = ReadText(p, &dummy, false, endTag, false);
        return p;
    } else {
        bool ignoreWhite = true;

        const char* end = "<";
        p = ReadText(p, &value, ignoreWhite, end, false);
        if (p) return p - 1;  // don't truncate the '<'
        return nullptr;
    }
}

bool TiXmlText::Blank() const {
    for (unsigned i = 0; i < value.length(); i++) {
        if (!IsWhiteSpace(value[i])) return false;
    }
    return true;
}
