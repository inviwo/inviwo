#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>

#include <ticpp/node.h>

/** XML text. A text node can have 2 ways to output the next. "normal" output
        and CDATA. It will default to the mode it was parsed from the XML file and
        you generally want to leave it alone, but you can change the output mode with
        SetCDATA() and query it with CDATA().
*/
class TICPP_API TiXmlText : public TiXmlNode {
    friend class TiXmlElement;

public:
    /** Constructor for text element. By default, it is treated as
        normal, encoded text. If you want it be output as a CDATA text
        element, set the parameter _cdata to 'true'
    */
    TiXmlText(std::string_view initValue, bool _cdata = false)
        : TiXmlNode(TiXmlNode::TEXT, initValue), cdata{_cdata} {}

    virtual ~TiXmlText() {}

    TiXmlText(const TiXmlText& copy) : TiXmlNode(TiXmlNode::TEXT) { copy.CopyTo(this); }
    void operator=(const TiXmlText& base) { base.CopyTo(this); }

    // Write this text object to a FILE stream.
    virtual void Print(FILE* cfile, int depth) const;

    /// Queries whether this represents text using a CDATA section.
    bool CDATA() const { return cdata; }
    /// Turns on or off a CDATA representation of text.
    void SetCDATA(bool _cdata) { cdata = _cdata; }

    virtual const char* Parse(const char* p, TiXmlParsingData* data);

    /// Cast to a more defined type. Will return null not of the requested type.
    virtual const TiXmlText* ToText() const { return this; }
    /// Cast to a more defined type. Will return null not of the requested type.
    virtual TiXmlText* ToText() { return this; }

    /// Walk the XML tree visiting this node and all of its children.
    virtual bool Accept(TiXmlVisitor* content) const;

protected:
    ///  [internal use] Creates a new Element and returns it.
    virtual TiXmlNode* Clone() const;
    void CopyTo(TiXmlText* target) const;

    bool Blank() const;  // returns true if all white space and new lines
                         // [internal use]

private:
    bool cdata;  // true if this should be input and output as a CDATA style text element
};
