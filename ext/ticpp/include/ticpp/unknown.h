#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>

#include <ticpp/node.h>

/** Any tag that tinyXml doesn't recognize is saved as an
        unknown. It is a tag of text, but should not be modified.
        It will be written back to the XML, unchanged, when the file
        is saved.

        DTD tags get thrown into TiXmlUnknowns.
*/
class TICPP_API TiXmlUnknown : public TiXmlNode {
public:
    TiXmlUnknown() : TiXmlNode(TiXmlNode::UNKNOWN) {}
    virtual ~TiXmlUnknown() {}

    TiXmlUnknown(const TiXmlUnknown& copy) : TiXmlNode(TiXmlNode::UNKNOWN) { copy.CopyTo(this); }
    void operator=(const TiXmlUnknown& copy) { copy.CopyTo(this); }

    /// Creates a copy of this Unknown and returns it.
    virtual TiXmlNode* Clone() const;
    // Print this Unknown to a FILE stream.
    virtual void Print(FILE* cfile, int depth) const;

    virtual const char* Parse(const char* p, TiXmlParsingData* data);

    /// Cast to a more defined type. Will return null not of the requested type.
    virtual const TiXmlUnknown* ToUnknown() const { return this; }
    /// Cast to a more defined type. Will return null not of the requested type.
    virtual TiXmlUnknown* ToUnknown() { return this; }

    /// Walk the XML tree visiting this node and all of its children.
    virtual bool Accept(TiXmlVisitor* content) const;

protected:
    void CopyTo(TiXmlUnknown* target) const;

private:
};
