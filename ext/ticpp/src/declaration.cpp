#include <ticpp/declaration.h>
#include <ticpp/parsingdata.h>

#include <fmt/printf.h>

TiXmlDeclaration::TiXmlDeclaration(allocator_type alloc)
    : TiXmlNode(TiXmlNode::DECLARATION, "", alloc)
    , version{alloc}
    , encoding(alloc)
    , standalone(alloc) {}

TiXmlDeclaration::TiXmlDeclaration(std::string_view _version, std::string_view _encoding,
                                   std::string_view _standalone, allocator_type alloc)
    : TiXmlNode(TiXmlNode::DECLARATION, "", alloc)
    , version{_version, alloc}
    , encoding(_encoding, alloc)
    , standalone(_standalone, alloc) {}

TiXmlDeclaration::TiXmlDeclaration(const TiXmlDeclaration& copy)
    : TiXmlNode(TiXmlNode::DECLARATION) {
    copy.CopyTo(this);
}

void TiXmlDeclaration::operator=(const TiXmlDeclaration& copy) {
    Clear();
    copy.CopyTo(this);
}

void TiXmlDeclaration::Print(std::string* str) const {
    if (!str) return;

    *str += "<?xml ";
    if (!version.empty()) {
        *str += "version=\"";
        *str += version;
        *str += "\" ";
    }
    if (!encoding.empty()) {
        *str += "encoding=\"";
        *str += encoding;
        *str += "\" ";
    }
    if (!standalone.empty()) {
        *str += "standalone=\"";
        *str += standalone;
        *str += "\" ";
    }
    *str += "?>";
}

void TiXmlDeclaration::Print(FILE* file) const {
    if (!file) return;

    fmt::fprintf(file, "<?xml ");
    if (!version.empty()) {
        fmt::fprintf(file, "version=\"%s\" ", version);
    }
    if (!encoding.empty()) {
        fmt::fprintf(file, "encoding=\"%s\" ", encoding);
    }
    if (!standalone.empty()) {
        fmt::fprintf(file, "standalone=\"%s\" ", standalone);
    }
    fmt::fprintf(file, "?>");
}

void TiXmlDeclaration::CopyTo(TiXmlDeclaration* target) const {
    TiXmlNode::CopyTo(target);

    target->version = version;
    target->encoding = encoding;
    target->standalone = standalone;
}

bool TiXmlDeclaration::Accept(TiXmlVisitor* visitor) const { return visitor->Visit(*this); }

TiXmlNode* TiXmlDeclaration::Clone(allocator_type alloc) const {
    TiXmlDeclaration* clone = alloc.new_object<TiXmlDeclaration>();
    CopyTo(clone);
    return clone;
}

const char* TiXmlDeclaration::Parse(const char* p, TiXmlParsingData* data, allocator_type alloc) {
    p = SkipWhiteSpace(p);
    // Find the beginning, find the end, and look for the stuff in-between.
    if (!p || !*p || !StringEqual(p, "<?xml", true)) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_PARSING_DECLARATION, nullptr, nullptr);
    }

    constexpr size_t size = std::string_view{"<?xml"}.size();
    p += size;

    version.clear();
    encoding.clear();
    standalone.clear();

    while (p && *p) {
        if (*p == '>') {
            ++p;
            return p;
        }

        p = SkipWhiteSpace(p);

        std::pmr::string dummy{alloc};
        if (StringEqual(p, "version", true)) {
            p = ReadNameValue(p, &dummy, &version, data);
        } else if (StringEqual(p, "encoding", true)) {
            p = ReadNameValue(p, &dummy, &encoding, data);
        } else if (StringEqual(p, "standalone", true)) {
            p = ReadNameValue(p, &dummy, &standalone, data);
        } else {
            // Read over whatever it is.
            while (p && *p && *p != '>' && !IsWhiteSpace(*p)) ++p;
        }
    }
    return nullptr;
}
