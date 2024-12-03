#include <ticpp/text.h>

#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

#include <cstring>

void TiXmlText::CopyTo(TiXmlText* target) const {
    TiXmlNode::CopyTo(target);
    target->cdata = cdata;
}

bool TiXmlText::Accept(TiXmlVisitor* visitor) const { return visitor->Visit(*this); }

TiXmlNode* TiXmlText::Clone(allocator_type alloc) const {
    auto* clone = alloc.new_object<TiXmlText>("", false);
    CopyTo(clone);
    return clone;
}

const char* TiXmlText::Parse(const char* p, TiXmlParsingData* data, allocator_type alloc) {
    value.clear();

    const char* const startTag = "<![CDATA[";
    const char* const endTag = "]]>";

    if (cdata || StringEqual(p, startTag, false)) {
        cdata = true;

        if (!StringEqual(p, startTag, false)) {
            throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_PARSING_CDATA, p, data);
        }
        p += strlen(startTag);

        // Keep all the white space, ignore the encoding, etc.
        while (p && *p && !StringEqual(p, endTag, false)) {
            value += *p;
            ++p;
        }

        std::pmr::string dummy{alloc};
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
