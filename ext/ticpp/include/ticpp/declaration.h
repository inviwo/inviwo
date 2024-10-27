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
class TICPP_API TiXmlDeclaration final : public TiXmlNode {
public:
    /// Construct an empty declaration.
    TiXmlDeclaration(allocator_type alloc = {});
    TiXmlDeclaration(std::string_view _version, std::string_view _encoding,
                     std::string_view _standalone, allocator_type alloc = {});

    TiXmlDeclaration(const TiXmlDeclaration& copy);
    void operator=(const TiXmlDeclaration& copy);

    virtual ~TiXmlDeclaration() {}

    /// Version. Will return an empty string if none was found.
    std::string_view Version() const { return version; }
    /// Encoding. Will return an empty string if none was found.
    std::string_view Encoding() const { return encoding; }
    /// Is this a standalone document?
    std::string_view Standalone() const { return standalone; }

    void Print(std::string* str) const;
    void Print(FILE* file) const;

    virtual const char* Parse(const char* p, TiXmlParsingData* data, allocator_type alloc) override;

    /// Cast to a more defined type. Will return null not of the requested type.
    virtual const TiXmlDeclaration* ToDeclaration() const override { return this; }

    /// Cast to a more defined type. Will return null not of the requested type.
    virtual TiXmlDeclaration* ToDeclaration() override { return this; }

    /// Creates a copy of this Declaration and returns it.
    virtual TiXmlNode* Clone(allocator_type alloc) const override;
    using TiXmlNode::Clone;

    /// Walk the XML tree visiting this node and all of its children.
    virtual bool Accept(TiXmlVisitor* visitor) const override;

protected:
    void CopyTo(TiXmlDeclaration* target) const;

private:
    std::pmr::string version;
    std::pmr::string encoding;
    std::pmr::string standalone;
};
