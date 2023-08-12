#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>

#include <ticpp/node.h>
#include <ticpp/attribute.h>

/** The element is a container class. It has a value, the element name,
        and can contain other elements, text, comments, and unknowns.
        Elements also contain an arbitrary number of attributes.
*/
class TICPP_API TiXmlElement : public TiXmlNode {
public:
    /// Construct an element.
    TiXmlElement(const char* in_value);

    /// std::string constructor.
    TiXmlElement(const std::string& _value);
    TiXmlElement(std::string_view _value);

    TiXmlElement(const TiXmlElement&);

    void operator=(const TiXmlElement& base);

    virtual ~TiXmlElement();

    /** Given an attribute name, Attribute() returns the value
            for the attribute of that name, or null if none exists.
    */
    const char* Attribute(const char* name) const;
    const std::string* AttributeStr(const char* name) const;

    /** Given an attribute name, Attribute() returns the value
            for the attribute of that name, or null if none exists.
            If the attribute exists and can be converted to an integer,
            the integer value will be put in the return 'i', if 'i'
            is non-null.
    */
    const char* Attribute(const char* name, int* i) const;

    /** Given an attribute name, Attribute() returns the value
            for the attribute of that name, or null if none exists.
            If the attribute exists and can be converted to an double,
            the double value will be put in the return 'd', if 'd'
            is non-null.
    */
    const char* Attribute(const char* name, double* d) const;

    /** QueryIntAttribute examines the attribute - it is an alternative to the
            Attribute() method with richer error checking.
            If the attribute is an integer, it is stored in 'value' and
            the call returns TIXML_SUCCESS. If it is not
            an integer, it returns TIXML_WRONG_TYPE. If the attribute
            does not exist, then TIXML_NO_ATTRIBUTE is returned.
    */
    int QueryIntAttribute(const char* name, int* _value) const;
    /// QueryDoubleAttribute examines the attribute - see QueryIntAttribute().
    int QueryDoubleAttribute(const char* name, double* _value) const;
    /// QueryFloatAttribute examines the attribute - see QueryIntAttribute().
    int QueryFloatAttribute(const char* name, float* _value) const {
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
    int QueryValueAttribute(const std::string& name, T* outValue) const {
        const TiXmlAttribute* node = attributeSet.Find(name);
        if (!node) return TIXML_NO_ATTRIBUTE;

        std::stringstream sstream(node->ValueStr());
        sstream >> *outValue;
        if (!sstream.fail()) return TIXML_SUCCESS;
        return TIXML_WRONG_TYPE;
    }
    /*
     This is - in theory - a bug fix for "QueryValueAtribute returns truncated std::string"
     but template specialization is hard to get working cross-compiler. Leaving the bug for now.

    // The above will fail for std::string because the space character is used as a seperator.
    // Specialize for strings. Bug [ 1695429 ] QueryValueAtribute returns truncated std::string
    template<> int QueryValueAttribute( const std::string& name, std::string* outValue ) const
    {
            const TiXmlAttribute* node = attributeSet.Find( name );
            if ( !node )
                    return TIXML_NO_ATTRIBUTE;
            *outValue = node->ValueStr();
            return TIXML_SUCCESS;
    }
    */

    /** Sets an attribute of name to a given value. The attribute
            will be created if it does not exist, or changed if it does.
    */
    void SetAttribute(const char* name, const char* _value);

    const std::string* Attribute(std::string_view name) const;

    const std::string* Attribute(const std::string& name) const;
    const std::string* Attribute(const std::string& name, int* i) const;
    const std::string* Attribute(const std::string& name, double* d) const;
    int QueryIntAttribute(const std::string& name, int* _value) const;
    int QueryDoubleAttribute(const std::string& name, double* _value) const;

    /// STL std::string form.
    void SetAttribute(const std::string& name, const std::string& _value);
    void SetAttribute(std::string_view name, std::string_view _value);


    ///< STL std::string form.
    void SetAttribute(const std::string& name, int _value);

    /** Sets an attribute of name to a given value. The attribute
            will be created if it does not exist, or changed if it does.
    */
    void SetAttribute(const char* name, int value);

    /** Sets an attribute of name to a given value. The attribute
            will be created if it does not exist, or changed if it does.
    */
    void SetDoubleAttribute(const char* name, double value);

    /** Deletes an attribute with the given name.
     */
    void RemoveAttribute(const char* name);

    void RemoveAttribute(const std::string& name) {
        RemoveAttribute(name.c_str());
    }  ///< STL std::string form.

    void RemoveAttribute(std::string_view name);

    const TiXmlAttribute* FirstAttribute() const {
        return attributeSet.First();
    }  ///< Access the first attribute in this element.
    TiXmlAttribute* FirstAttribute() { return attributeSet.First(); }
    const TiXmlAttribute* LastAttribute() const {
        return attributeSet.Last();
    }  ///< Access the last attribute in this element.
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
    const char* GetText() const;

    const std::string* GetTextStr() const;

    /// Creates a new Element and returns it - the returned element is a copy.
    virtual TiXmlNode* Clone() const;
    // Print the Element to a FILE stream.
    virtual void Print(FILE* cfile, int depth) const;

    /*	Attribtue parsing starts: next char past '<'
                                             returns: next char past '>'
    */
    virtual const char* Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding);

    virtual const TiXmlElement* ToElement() const {
        return this;
    }  ///< Cast to a more defined type. Will return null not of the requested type.
    virtual TiXmlElement* ToElement() {
        return this;
    }  ///< Cast to a more defined type. Will return null not of the requested type.

    /** Walk the XML tree visiting this node and all of its children.
     */
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
    const char* ReadValue(const char* in, TiXmlParsingData* prevData, TiXmlEncoding encoding);

private:
    TiXmlAttributeSet attributeSet;
};
