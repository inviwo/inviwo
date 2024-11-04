#include <ticpp/unknown.h>

#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

void TiXmlUnknown::CopyTo(TiXmlUnknown* target) const { TiXmlNode::CopyTo(target); }

bool TiXmlUnknown::Accept(TiXmlVisitor* visitor) const { return visitor->Visit(*this); }

TiXmlNode* TiXmlUnknown::Clone(allocator_type alloc) const {
    TiXmlUnknown* clone = alloc.new_object<TiXmlUnknown>();
    CopyTo(clone);
    return clone;
}

const char* TiXmlUnknown::Parse(const char* p, TiXmlParsingData* data, allocator_type alloc) {
    p = SkipWhiteSpace(p);

    if (!p || !*p || *p != '<') {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_PARSING_UNKNOWN, p, data);
    }
    ++p;
    value.clear();

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
