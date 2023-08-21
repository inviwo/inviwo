#include <ticpp/declaration.h>

#include <ticpp/attribute.h>
#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

TiXmlDeclaration::TiXmlDeclaration(const char* _version, const char* _encoding,
                                   const char* _standalone)
    : TiXmlNode(TiXmlNode::DECLARATION) {
    version = _version;
    encoding = _encoding;
    standalone = _standalone;
}

TiXmlDeclaration::TiXmlDeclaration(const std::string& _version, const std::string& _encoding,
                                   const std::string& _standalone)
    : TiXmlNode(TiXmlNode::DECLARATION) {
    version = _version;
    encoding = _encoding;
    standalone = _standalone;
}

TiXmlDeclaration::TiXmlDeclaration(std::string_view _version, std::string_view _encoding,
                                   std::string_view _standalone)
    : TiXmlNode(TiXmlNode::DECLARATION) {
    version = _version;
    encoding = _encoding;
    standalone = _standalone;
}

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



void TiXmlDeclaration::StreamIn(std::istream* in, std::string* tag) {
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

const char* TiXmlDeclaration::Parse(const char* p, TiXmlParsingData* data,
                                    TiXmlEncoding _encoding) {
    p = SkipWhiteSpace(p, _encoding);
    // Find the beginning, find the end, and look for
    // the stuff in-between.
    TiXmlDocument* document = GetDocument();
    if (!p || !*p || !StringEqual(p, "<?xml", true, _encoding)) {
        if (document) document->SetError(TIXML_ERROR_PARSING_DECLARATION, 0, 0, _encoding);
        return 0;
    }
    if (data) {
        data->Stamp(p, _encoding);
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

        p = SkipWhiteSpace(p, _encoding);
        if (StringEqual(p, "version", true, _encoding)) {
            TiXmlAttribute attrib;
            p = attrib.Parse(p, data, _encoding);
            version = attrib.Value();
        } else if (StringEqual(p, "encoding", true, _encoding)) {
            TiXmlAttribute attrib;
            p = attrib.Parse(p, data, _encoding);
            encoding = attrib.Value();
        } else if (StringEqual(p, "standalone", true, _encoding)) {
            TiXmlAttribute attrib;
            p = attrib.Parse(p, data, _encoding);
            standalone = attrib.Value();
        } else {
            // Read over whatever it is.
            while (p && *p && *p != '>' && !IsWhiteSpace(*p)) ++p;
        }
    }
    return 0;
}
