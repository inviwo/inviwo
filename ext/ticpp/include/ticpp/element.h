#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>
#include <ticpp/node.h>
#include <ticpp/attribute.h>

#include <optional>

/** The element is a container class. It has a value, the element name,
    and can contain other elements, text, comments, and unknowns.
    Elements also contain an arbitrary number of attributes.
*/
class TICPP_API TiXmlElement final : public TiXmlNode {
public:
    explicit TiXmlElement(std::string_view _value, allocator_type alloc = {});

    TiXmlElement(const TiXmlElement&);
    void operator=(const TiXmlElement& base);
    virtual ~TiXmlElement();

    /** Given an attribute name, Attribute() returns the value
     *  for the attribute of that name, or nullopt if none exists.
     */
    std::optional<std::string_view> Attribute(std::string_view name) const;

    std::string GetAttribute(std::string_view name) const {
        if (const auto attribute = Attribute(name)) {
            return std::string{*attribute};
        } else {
            return std::string{};
        }
    }

    /** Sets an attribute of name to a given value. The attribute
        will be created if it does not exist, or changed if it does.
    */
    void SetAttribute(std::string_view name, std::string_view _value);

    /** Adds an attribute name with the given value. The attribute
        will be created; If it exists an exception will be thrown.
    */
    void AddAttribute(std::string_view name, std::string_view _value);

    /** Adds a new attribute name and return a reference to the empty value,
        The attribute will be created; If it exists an exception will be thrown.
    */
    std::pmr::string& AddAttribute(std::string_view name);

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
    std::optional<std::string_view> GetText() const;

    /*	Attribute parsing starts: next char past '<'
        returns: next char past '>'
    */
    virtual const char* Parse(const char* p, TiXmlParsingData* data, allocator_type alloc) override;

    /// Cast to a more defined type. Will return null not of the requested type.
    virtual const TiXmlElement* ToElement() const override { return this; }
    /// Cast to a more defined type. Will return null not of the requested type.
    virtual TiXmlElement* ToElement() override { return this; }

    /// Creates a new Element and returns it - the returned element is a copy.
    virtual TiXmlNode* Clone(allocator_type alloc) const override;
    using TiXmlNode::Clone;

    /// Walk the XML tree visiting this node and all of its children.
    virtual bool Accept(TiXmlVisitor* visitor) const override;

protected:
    void CopyTo(TiXmlElement* target) const;

    /*	[internal use]
        Reads the "value" of the element -- another element, or text.
        This should terminate with the current end tag.
    */
    const char* ReadValue(const char* in, TiXmlParsingData* prevData, allocator_type alloc);

private:
    TiXmlAttributeSet attributeSet;
};

inline bool TiXmlElement::Accept(TiXmlVisitor* visitor) const {
    if (visitor->VisitEnter(*this, attributeSet.First())) {
        for (const TiXmlNode* node = FirstChild(); node; node = node->NextSibling()) {
            if (!node->Accept(visitor)) break;
        }
    }
    return visitor->VisitExit(*this);
}
