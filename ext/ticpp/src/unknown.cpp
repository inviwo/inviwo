#include <ticpp/unknown.h>

#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

void TiXmlUnknown::CopyTo(TiXmlUnknown* target) const { TiXmlNode::CopyTo(target); }

bool TiXmlUnknown::Accept(TiXmlVisitor* visitor) const { return visitor->Visit(*this); }

TiXmlNode* TiXmlUnknown::Clone() const {
    TiXmlUnknown* clone = new TiXmlUnknown();

    if (!clone) return 0;

    CopyTo(clone);
    return clone;
}

const char* TiXmlUnknown::Parse(const char* p, TiXmlParsingData* data,
                                const allocator_type& alloc) {
    p = SkipWhiteSpace(p);

    if (data) {
        data->Stamp(p);
        location = data->Cursor();
    }
    if (!p || !*p || *p != '<') {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_PARSING_UNKNOWN, p, data);
    }
    ++p;
    value = "";

    while (p && *p && *p != '>') {
        value += *p;
        ++p;
    }

    if (!p) {
         throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_PARSING_UNKNOWN, nullptr, nullptr);
    }
    if (*p == '>') return p + 1;
    return p;
}
