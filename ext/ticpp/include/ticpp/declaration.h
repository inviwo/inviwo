#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>

#include <ticpp/node.h>

/** In correct XML the declaration is the first entry in the file.
    @verbatim
        <?xml version="1.0" standalone="yes"?>
    @endverbatim

    TinyXml will happily read or write files without a declaration,
    however. There are 3 possible attributes to the declaration:
    version, encoding, and standalone.

    Note: In this version of the code, the attributes are
    handled as special cases, not generic attributes, simply
    because there can only be at most 3 and they are always the same.
*/
class TICPP_API TiXmlDeclaration : public TiXmlNode {
public:
    /// Construct an empty declaration.
    TiXmlDeclaration() : TiXmlNode(TiXmlNode::DECLARATION) {}

    /// Constructor.
    TiXmlDeclaration(const std::string& _version, const std::string& _encoding,
                     const std::string& _standalone);

    TiXmlDeclaration(std::string_view _version, std::string_view _encoding,
                     std::string_view _standalone);

    /// Construct.
    TiXmlDeclaration(const char* _version, const char* _encoding, const char* _standalone);

    TiXmlDeclaration(const TiXmlDeclaration& copy);
    void operator=(const TiXmlDeclaration& copy);

    virtual ~TiXmlDeclaration() {}

    /// Version. Will return an empty string if none was found.
    const std::string& Version() const { return version; }
    /// Encoding. Will return an empty string if none was found.
    const std::string& Encoding() const { return encoding; }
    /// Is this a standalone document?
    const std::string& Standalone() const { return standalone; }

    /// Creates a copy of this Declaration and returns it.
    virtual TiXmlNode* Clone() const;
    // Print this declaration to a FILE stream.
    virtual void Print(FILE* cfile, int depth, std::string* str) const;
    virtual void Print(FILE* cfile, int depth) const { Print(cfile, depth, 0); }

    virtual const char* Parse(const char* p, TiXmlParsingData* data);

    /// Cast to a more defined type. Will return null not of the requested type.
    virtual const TiXmlDeclaration* ToDeclaration() const { return this; }

    /// Cast to a more defined type. Will return null not of the requested type.
    virtual TiXmlDeclaration* ToDeclaration() { return this; }

    /// Walk the XML tree visiting this node and all of its children.
    virtual bool Accept(TiXmlVisitor* visitor) const;

protected:
    void CopyTo(TiXmlDeclaration* target) const;
    // used to be public
    virtual void StreamIn(std::istream* in, std::string* tag);

private:
    std::string version;
    std::string encoding;
    std::string standalone;
};
