#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>

#include <ticpp/node.h>

/** A stylesheet reference looks like this:
    @verbatim
            <?xml-stylesheet type="text/xsl" href="style.xsl"?>
    @endverbatim

    Note: In this version of the code, the attributes are
    handled as special cases, not generic attributes, simply
    because there can only be at most 2 and they are always the same.
*/
class TICPP_API TiXmlStylesheetReference final : public TiXmlNode {
public:
    /// Construct an empty declaration.
    TiXmlStylesheetReference(const allocator_type& alloc = {});

    /// Constructor.
    TiXmlStylesheetReference(std::string_view _type, std::string_view _href,
                             const allocator_type& alloc = {});

    TiXmlStylesheetReference(const TiXmlStylesheetReference& copy);
    void operator=(const TiXmlStylesheetReference& copy);

    virtual ~TiXmlStylesheetReference() {}

    /// Type. Will return an empty string if none was found.
    std::string_view Type() const { return type; }
    /// Href. Will return an empty string if none was found.
    std::string_view Href() const { return href; }

    /// Creates a copy of this StylesheetReference and returns it.
    virtual TiXmlNode* Clone() const override;

    // Print this declaration to a std::string
    void Print(std::string* str) const;

    // Print this declaration to a FILE stream.
    virtual void Print(FILE* file) const;

    virtual const char* Parse(const char* p, TiXmlParsingData* data,
                              const allocator_type& alloc) override;

    /// Cast to a more defined type. Will return null not of the requested type.
    virtual const TiXmlStylesheetReference* ToStylesheetReference() const override { return this; }
    /// Cast to a more defined type. Will return null not of the requested type.
    virtual TiXmlStylesheetReference* ToStylesheetReference() override { return this; }

    /// Walk the XML tree visiting this node and all of its children.
    virtual bool Accept(TiXmlVisitor* visitor) const override;

protected:
    void CopyTo(TiXmlStylesheetReference* target) const;

private:
    std::pmr::string type;
    std::pmr::string href;
};
