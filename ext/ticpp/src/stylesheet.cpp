#include <ticpp/stylesheet.h>
#include <ticpp/parsingdata.h>

#include <fmt/printf.h>

TiXmlStylesheetReference::TiXmlStylesheetReference(allocator_type alloc)
    : TiXmlNode(TiXmlNode::STYLESHEETREFERENCE, "", alloc), type{alloc}, href{alloc} {}

TiXmlStylesheetReference::TiXmlStylesheetReference(std::string_view _type, std::string_view _href,
                                                   allocator_type alloc)
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

void TiXmlStylesheetReference::Print(std::pmr::string& out) const {
    out += "<?xml-stylesheet ";
    if (!type.empty()) {
        out += "type=\"";
        out += type;
        out += "\" ";
    }
    if (!href.empty()) {
        out += "href=\"";
        out += href;
        out += "\" ";
    }
    out += "?>";
}

void TiXmlStylesheetReference::Print(FILE* file) const {
    if (!file) return;

    fmt::fprintf(file, "<?xml-stylesheet ");
    if (!type.empty()) {
        fmt::fprintf(file, "type=\"%s\" ", type);
    }
    if (!href.empty()) {
        fmt::fprintf(file, "href=\"%s\" ", href);
    }
    fmt::fprintf(file, "?>");
}

void TiXmlStylesheetReference::CopyTo(TiXmlStylesheetReference* target) const {
    TiXmlNode::CopyTo(target);

    target->type = type;
    target->href = href;
}

bool TiXmlStylesheetReference::Accept(TiXmlVisitor* visitor) const { return visitor->Visit(*this); }

TiXmlNode* TiXmlStylesheetReference::Clone(allocator_type alloc) const {
    auto* clone = alloc.new_object<TiXmlStylesheetReference>();
    CopyTo(clone);
    return clone;
}

const char* TiXmlStylesheetReference::Parse(const char* p, TiXmlParsingData* data,
                                            allocator_type alloc) {
    p = SkipWhiteSpace(p);
    // Find the beginning, find the end, and look for
    // the stuff in-between.
    if (!p || !*p || !StringEqual(p, "<?xml-stylesheet", true)) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_PARSING_DECLARATION, nullptr, nullptr);
    }

    constexpr size_t size = std::string_view{"<?xml-stylesheet"}.size();
    p += size;

    type.clear();
    href.clear();

    while (p && *p) {
        if (*p == '>') {
            ++p;
            return p;
        }

        p = SkipWhiteSpace(p);

        std::pmr::string dummy{alloc};
        if (StringEqual(p, "type", true)) {
            p = ReadNameValue(p, &dummy, &type, data);
        } else if (StringEqual(p, "href", true)) {
            p = ReadNameValue(p, &dummy, &href, data);
        } else {
            // Read over whatever it is.
            while (p && *p && *p != '>' && !IsWhiteSpace(*p)) ++p;
        }
    }
    return nullptr;
}
