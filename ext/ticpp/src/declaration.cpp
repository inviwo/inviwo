#include <ticpp/declaration.h>

#include <ticpp/attribute.h>
#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

TiXmlDeclaration::TiXmlDeclaration(const allocator_type& alloc)
    : TiXmlNode(TiXmlNode::DECLARATION, "", alloc)
    , version{alloc}
    , encoding(alloc)
    , standalone(alloc) {}

TiXmlDeclaration::TiXmlDeclaration(std::string_view _version, std::string_view _encoding,
                                   std::string_view _standalone, const allocator_type& alloc)
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

void TiXmlDeclaration::Print(FILE* cfile, int /*depth*/, std::string* str) const {
    if (cfile) fprintf(cfile, "<?xml ");
    if (str) (*str) += "<?xml ";

    if (!version.empty()) {
        if (cfile) fprintf(cfile, "version=\"%s\" ", version.c_str());
        if (str) {
            (*str) += "version=\"";
            (*str) += version;
            (*str) += "\" ";
        }
    }
    if (!encoding.empty()) {
        if (cfile) fprintf(cfile, "encoding=\"%s\" ", encoding.c_str());
        if (str) {
            (*str) += "encoding=\"";
            (*str) += encoding;
            (*str) += "\" ";
        }
    }
    if (!standalone.empty()) {
        if (cfile) fprintf(cfile, "standalone=\"%s\" ", standalone.c_str());
        if (str) {
            (*str) += "standalone=\"";
            (*str) += standalone;
            (*str) += "\" ";
        }
    }
    if (cfile) fprintf(cfile, "?>");
    if (str) (*str) += "?>";
}

void TiXmlDeclaration::CopyTo(TiXmlDeclaration* target) const {
    TiXmlNode::CopyTo(target);

    target->version = version;
    target->encoding = encoding;
    target->standalone = standalone;
}

bool TiXmlDeclaration::Accept(TiXmlVisitor* visitor) const { return visitor->Visit(*this); }

TiXmlNode* TiXmlDeclaration::Clone() const {
    TiXmlDeclaration* clone = new TiXmlDeclaration();

    if (!clone) return 0;

    CopyTo(clone);
    return clone;
}

const char* TiXmlDeclaration::Parse(const char* p, TiXmlParsingData* data,
                                    const allocator_type& alloc) {
    p = SkipWhiteSpace(p);
    // Find the beginning, find the end, and look for
    // the stuff in-between.
    TiXmlDocument* document = GetDocument();
    if (!p || !*p || !StringEqual(p, "<?xml", true)) {
        if (document) document->SetError(TIXML_ERROR_PARSING_DECLARATION, nullptr, nullptr);
        return 0;
    }
    if (data) {
        data->Stamp(p);
        location = data->Cursor();
    }
    p += 5;

    version = "";
    encoding = "";
    standalone = "";

    while (p && *p) {
        if (*p == '>') {
            ++p;
            return p;
        }

        p = SkipWhiteSpace(p);
        if (StringEqual(p, "version", true)) {
            TiXmlAttribute attrib{alloc};
            p = attrib.Parse(p, data, alloc);
            version = attrib.Value();
        } else if (StringEqual(p, "encoding", true)) {
            TiXmlAttribute attrib{alloc};
            p = attrib.Parse(p, data, alloc);
            encoding = attrib.Value();
        } else if (StringEqual(p, "standalone", true)) {
            TiXmlAttribute attrib{alloc};
            p = attrib.Parse(p, data, alloc);
            standalone = attrib.Value();
        } else {
            // Read over whatever it is.
            while (p && *p && *p != '>' && !IsWhiteSpace(*p)) ++p;
        }
    }
    return 0;
}
