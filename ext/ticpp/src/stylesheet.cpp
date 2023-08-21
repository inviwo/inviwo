#include <ticpp/stylesheet.h>

#include <ticpp/attribute.h>
#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

TiXmlStylesheetReference::TiXmlStylesheetReference(const char* _type, const char* _href)
    : TiXmlNode(TiXmlNode::STYLESHEETREFERENCE) {
    type = _type;
    href = _href;
}

TiXmlStylesheetReference::TiXmlStylesheetReference(const std::string& _type,
                                                   const std::string& _href)
    : TiXmlNode(TiXmlNode::STYLESHEETREFERENCE) {
    type = _type;
    href = _href;
}

TiXmlStylesheetReference::TiXmlStylesheetReference(const TiXmlStylesheetReference& copy)
    : TiXmlNode(TiXmlNode::STYLESHEETREFERENCE) {
    copy.CopyTo(this);
}

void TiXmlStylesheetReference::operator=(const TiXmlStylesheetReference& copy) {
    Clear();
    copy.CopyTo(this);
}

void TiXmlStylesheetReference::Print(FILE* cfile, int /*depth*/, std::string* str) const {
    if (cfile) fprintf(cfile, "<?xml-stylesheet ");
    if (str) (*str) += "<?xml-stylesheet ";

    if (!type.empty()) {
        if (cfile) fprintf(cfile, "type=\"%s\" ", type.c_str());
        if (str) {
            (*str) += "type=\"";
            (*str) += type;
            (*str) += "\" ";
        }
    }
    if (!href.empty()) {
        if (cfile) fprintf(cfile, "href=\"%s\" ", href.c_str());
        if (str) {
            (*str) += "href=\"";
            (*str) += href;
            (*str) += "\" ";
        }
    }
    if (cfile) fprintf(cfile, "?>");
    if (str) (*str) += "?>";
}

void TiXmlStylesheetReference::CopyTo(TiXmlStylesheetReference* target) const {
    TiXmlNode::CopyTo(target);

    target->type = type;
    target->href = href;
}

bool TiXmlStylesheetReference::Accept(TiXmlVisitor* visitor) const { return visitor->Visit(*this); }

TiXmlNode* TiXmlStylesheetReference::Clone() const {
    TiXmlStylesheetReference* clone = new TiXmlStylesheetReference();

    if (!clone) return 0;

    CopyTo(clone);
    return clone;
}

void TiXmlStylesheetReference::StreamIn(std::istream* in, std::string* tag) {
    while (in->good()) {
        int c = in->get();
        if (c <= 0) {
            TiXmlDocument* document = GetDocument();
            if (document)
                document->SetError(TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN);
            return;
        }
        (*tag) += (char)c;

        if (c == '>') {
            // All is well.
            return;
        }
    }
}

const char* TiXmlStylesheetReference::Parse(const char* p, TiXmlParsingData* data,
                                            TiXmlEncoding _encoding) {
    p = SkipWhiteSpace(p, _encoding);
    // Find the beginning, find the end, and look for
    // the stuff in-between.
    TiXmlDocument* document = GetDocument();
    if (!p || !*p || !StringEqual(p, "<?xml-stylesheet", true, _encoding)) {
        if (document) document->SetError(TIXML_ERROR_PARSING_DECLARATION, 0, 0, _encoding);
        return 0;
    }
    if (data) {
        data->Stamp(p, _encoding);
        location = data->Cursor();
    }
    p += 5;

    type = "";
    href = "";

    while (p && *p) {
        if (*p == '>') {
            ++p;
            return p;
        }

        p = SkipWhiteSpace(p, _encoding);
        if (StringEqual(p, "type", true, _encoding)) {
            TiXmlAttribute attrib;
            p = attrib.Parse(p, data, _encoding);
            type = attrib.Value();
        } else if (StringEqual(p, "href", true, _encoding)) {
            TiXmlAttribute attrib;
            p = attrib.Parse(p, data, _encoding);
            href = attrib.Value();
        } else {
            // Read over whatever it is.
            while (p && *p && *p != '>' && !IsWhiteSpace(*p)) ++p;
        }
    }
    return 0;
}
