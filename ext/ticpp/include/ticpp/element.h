#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>

#include <ticpp/node.h>
#include <ticpp/attribute.h>

#include <sstream>

/** The element is a container class. It has a value, the element name,
    and can contain other elements, text, comments, and unknowns.
    Elements also contain an arbitrary number of attributes.
*/
class TICPP_API TiXmlElement : public TiXmlNode {
public:
    TiXmlElement(std::string_view value);
    TiXmlElement(const TiXmlElement&);
    void operator=(const TiXmlElement& base);
    virtual ~TiXmlElement();

    /** Given an attribute name, Attribute() returns the value
     *  for the attribute of that name, or null if none exists.
     */
    const std::string* Attribute(std::string_view name) const;

    /** Given an attribute name, Attribute() returns the value
        for the attribute of that name, or null if none exists.
        If the attribute exists and can be converted to an integer,
        the integer value will be put in the return 'i', if 'i'
        is non-null.
     */
    const std::string* Attribute(std::string_view name, int* i) const;

    /** Given an attribute name, Attribute() returns the value
        for the attribute of that name, or null if none exists.
        If the attribute exists and can be converted to an double,
        the double value will be put in the return 'd', if 'd'
        is non-null.
    */
    const std::string* Attribute(std::string_view name, double* d) const;

    /** QueryIntAttribute examines the attribute - it is an alternative to the
        Attribute() method with richer error checking.
        If the attribute is an integer, it is stored in 'value' and
        the call returns TIXML_SUCCESS. If it is not
        an integer, it returns TIXML_WRONG_TYPE. If the attribute
        does not exist, then TIXML_NO_ATTRIBUTE is returned.
    */
    int QueryIntAttribute(std::string_view name, int* _value) const;
    /// QueryDoubleAttribute examines the attribute - see QueryIntAttribute().
    int QueryDoubleAttribute(std::string_view name, double* _value) const;
    /// QueryFloatAttribute examines the attribute - see QueryIntAttribute().
    int QueryFloatAttribute(std::string_view name, float* _value) const {
        double d;
        int result = QueryDoubleAttribute(name, &d);
        if (result == TIXML_SUCCESS) {
            *_value = (float)d;
        }
        return result;
    }
    /** Template form of the attribute query which will try to read the
        attribute into the specified type. Very easy, very powerful, but
        be careful to make sure to call this with the correct type.

        NOTE: This method doesn't work correctly for 'string' types.

        @return TIXML_SUCCESS, TIXML_WRONG_TYPE, or TIXML_NO_ATTRIBUTE
    */
    template <typename T>
    int QueryValueAttribute(std::string_view name, T* outValue) const {
        const TiXmlAttribute* node = attributeSet.Find(name);
        if (!node) return TIXML_NO_ATTRIBUTE;

        std::stringstream sstream(node->Value());
        sstream >> *outValue;
        if (!sstream.fail()) return TIXML_SUCCESS;
        return TIXML_WRONG_TYPE;
    }

    /** Sets an attribute of name to a given value. The attribute
        will be created if it does not exist, or changed if it does.
    */
    void SetAttribute(std::string_view name, std::string_view _value);

    /** Sets an attribute of name to a given value. The attribute
        will be created if it does not exist, or changed if it does.
    */
    void SetAttribute(std::string_view name, int value);

    /** Sets an attribute of name to a given value. The attribute
        will be created if it does not exist, or changed if it does.
    */
    void SetDoubleAttribute(std::string_view name, double value);

    /** Deletes an attribute with the given name.
     */
    void RemoveAttribute(std::string_view name);

    /// Access the first attribute in this element.
    const TiXmlAttribute* FirstAttribute() const { return attributeSet.First(); }
    TiXmlAttribute* FirstAttribute() { return attributeSet.First(); }
    /// Access the first attribute in this element.
    const TiXmlAttribute* LastAttribute() const { return attributeSet.Last(); }
    TiXmlAttribute* LastAttribute() { return attributeSet.Last(); }

    /** Convenience function for easy access to the text inside an element. Although easy
        and concise, GetText() is limited compared to getting the TiXmlText child
        and accessing it directly.

        If the first child of 'this' is a TiXmlText, the GetText()
        returns the character string of the Text node, else null is returned.

        This is a convenient method for getting the text of simple contained text:
        @verbatim
        <foo>This is text</foo>
        const char* str = fooElement->GetText();
        @endverbatim

        'str' will be a pointer to "This is text".

        Note that this function can be misleading. If the element foo was created from
        this XML:
        @verbatim
        <foo><b>This is text</b></foo>
        @endverbatim

        then the value of str would be null. The first child node isn't a text node, it is
        another element. From this XML:
        @verbatim
        <foo>This is <b>text</b></foo>
        @endverbatim
        GetText() will return "This is ".

        WARNING: GetText() accesses a child node - don't become confused with the
                 similarly named TiXmlHandle::Text() and TiXmlNode::ToText() which are
                 safe type casts on the referenced node.
    */
    const std::string* GetText() const;

    const std::string* GetTextStr() const;

    /// Creates a new Element and returns it - the returned element is a copy.
    virtual TiXmlNode* Clone() const;
    // Print the Element to a FILE stream.
    virtual void Print(FILE* cfile, int depth) const;

    /*	Attribute parsing starts: next char past '<'
        returns: next char past '>'
    */
    virtual const char* Parse(const char* p, TiXmlParsingData* data);

    /// Cast to a more defined type. Will return null not of the requested type.
    virtual const TiXmlElement* ToElement() const { return this; }
    /// Cast to a more defined type. Will return null not of the requested type.
    virtual TiXmlElement* ToElement() { return this; }

    /// Walk the XML tree visiting this node and all of its children.
    virtual bool Accept(TiXmlVisitor* visitor) const;

protected:
    void CopyTo(TiXmlElement* target) const;
    void ClearThis();  // like clear, but initializes 'this' object as well

    // Used to be public [internal use]
    virtual void StreamIn(std::istream* in, std::string* tag);

    /*	[internal use]
        Reads the "value" of the element -- another element, or text.
        This should terminate with the current end tag.
    */
    const char* ReadValue(const char* in, TiXmlParsingData* prevData);

private:
    TiXmlAttributeSet attributeSet;
};
