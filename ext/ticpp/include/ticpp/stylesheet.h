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
class TICPP_API TiXmlStylesheetReference : public TiXmlNode {
public:
    /// Construct an empty declaration.
    TiXmlStylesheetReference() : TiXmlNode(TiXmlNode::STYLESHEETREFERENCE) {}

    /// Constructor.
    TiXmlStylesheetReference(const std::string& _type, const std::string& _href);

    /// Construct.
    TiXmlStylesheetReference(const char* _type, const char* _href);

    TiXmlStylesheetReference(const TiXmlStylesheetReference& copy);
    void operator=(const TiXmlStylesheetReference& copy);

    virtual ~TiXmlStylesheetReference() {}

    /// Type. Will return an empty string if none was found.
    const char* Type() const { return type.c_str(); }
    /// Href. Will return an empty string if none was found.
    const char* Href() const { return href.c_str(); }

    /// Creates a copy of this StylesheetReference and returns it.
    virtual TiXmlNode* Clone() const;
    // Print this declaration to a FILE stream.
    virtual void Print(FILE* cfile, int depth, std::string* str) const;
    virtual void Print(FILE* cfile, int depth) const { Print(cfile, depth, 0); }

    virtual const char* Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding);

    virtual const TiXmlStylesheetReference* ToStylesheetReference() const {
        return this;
    }  ///< Cast to a more defined type. Will return null not of the requested type.
    virtual TiXmlStylesheetReference* ToStylesheetReference() {
        return this;
    }  ///< Cast to a more defined type. Will return null not of the requested type.

    /** Walk the XML tree visiting this node and all of its children.
     */
    virtual bool Accept(TiXmlVisitor* visitor) const;

protected:
    void CopyTo(TiXmlStylesheetReference* target) const;
    // used to be public
    virtual void StreamIn(std::istream* in, std::string* tag);

private:
    std::string type;
    std::string href;
};
