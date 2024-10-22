#include <ticpp/unknown.h>

#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

void TiXmlUnknown::Print(FILE* cfile, int depth) const {
    for (int i = 0; i < depth; i++) fprintf(cfile, "    ");
    fprintf(cfile, "<%s>", value.c_str());
}

void TiXmlUnknown::CopyTo(TiXmlUnknown* target) const { TiXmlNode::CopyTo(target); }

bool TiXmlUnknown::Accept(TiXmlVisitor* visitor) const { return visitor->Visit(*this); }

TiXmlNode* TiXmlUnknown::Clone() const {
    TiXmlUnknown* clone = new TiXmlUnknown();

    if (!clone) return 0;

    CopyTo(clone);
    return clone;
}

const char* TiXmlUnknown::Parse(const char* p, TiXmlParsingData* data) {
    TiXmlDocument* document = GetDocument();
    p = SkipWhiteSpace(p);

    if (data) {
        data->Stamp(p);
        location = data->Cursor();
    }
    if (!p || !*p || *p != '<') {
        if (document) document->SetError(TIXML_ERROR_PARSING_UNKNOWN, p, data);
        return 0;
    }
    ++p;
    value = "";

    while (p && *p && *p != '>') {
        value += *p;
        ++p;
    }

    if (!p) {
        if (document) document->SetError(TIXML_ERROR_PARSING_UNKNOWN, nullptr, nullptr);
    }
    if (*p == '>') return p + 1;
    return p;
}
