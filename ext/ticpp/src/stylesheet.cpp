#include <ticpp/stylesheet.h>

#include <ticpp/attribute.h>
#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

TiXmlStylesheetReference::TiXmlStylesheetReference(const allocator_type& alloc)
    : TiXmlNode(TiXmlNode::STYLESHEETREFERENCE, "", alloc), type{alloc}, href{alloc} {}

TiXmlStylesheetReference::TiXmlStylesheetReference(std::string_view _type, std::string_view _href,
                                                   const allocator_type& alloc)
    : TiXmlNode(TiXmlNode::STYLESHEETREFERENCE, "", alloc)
    , type{_type, alloc}
    , href{_href, alloc} {}

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

const char* TiXmlStylesheetReference::Parse(const char* p, TiXmlParsingData* data,
                                            const allocator_type& alloc) {
    p = SkipWhiteSpace(p);
    // Find the beginning, find the end, and look for
    // the stuff in-between.
    TiXmlDocument* document = GetDocument();
    if (!p || !*p || !StringEqual(p, "<?xml-stylesheet", true)) {
        if (document) document->SetError(TIXML_ERROR_PARSING_DECLARATION, nullptr, nullptr);
        return 0;
    }
    if (data) {
        data->Stamp(p);
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

        p = SkipWhiteSpace(p);
        if (StringEqual(p, "type", true)) {
            TiXmlAttribute attrib{alloc};
            p = attrib.Parse(p, data, alloc);
            type = attrib.Value();
        } else if (StringEqual(p, "href", true)) {
            TiXmlAttribute attrib{alloc};
            p = attrib.Parse(p, data, alloc);
            href = attrib.Value();
        } else {
            // Read over whatever it is.
            while (p && *p && *p != '>' && !IsWhiteSpace(*p)) ++p;
        }
    }
    return 0;
}
