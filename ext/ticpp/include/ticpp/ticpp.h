/*
http://code.google.com/p/ticpp/
Copyright (c) 2006 Ryan Pusztai, Ryan Mulder

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
@copydoc ticpp
@file
@author		Ryan Pusztai
@author		Ryan Mulder
@date		04/11/2006

@version	0.04a by edam@waxworlds.org: based Exception based on std::exception; added stream
                                        << and >> support; added Document::Parse(); bug fix;
improved THROW() macro.
@version	0.04 Added NodeImp class. Also made all the classes inherit from NodeImp.
@version	0.03 Added Declaration class
@version	0.02 Added Element class
@version	0.01 Added Exception class, Document class

@todo add UNKNOWN support. See ticpp::NodeFactory.
@todo add TYPECOUNT support. See ticpp::NodeFactory.
@todo Add a quick reference
*/

/*
 * THIS FILE WAS ALTERED BY Matt Janisz, 12. October 2012.
 *
 * - added ticppapi.h include and TICPP_API dll-interface to support building DLL using VS200X
 */

#pragma once

#include <ticpp/ticppapi.h>

#include <ticpp/visitor.h>
#include <ticpp/base.h>
#include <ticpp/node.h>
#include <ticpp/document.h>
#include <ticpp/attribute.h>
#include <ticpp/element.h>
#include <ticpp/comment.h>
#include <ticpp/text.h>
#include <ticpp/declaration.h>
#include <ticpp/stylesheet.h>

#include <sstream>
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <type_traits>
#include <string>
#include <string_view>
#include <cstring>

#include <fmt/format.h>
#include <fast_float/fast_float.h>
#include <source_location>

/**
@subpage ticpp is a TinyXML wrapper that uses a lot more C++ ideals.
It throws exceptions, uses templates, is in its own name space, and
<b>requires</b> STL (Standard Template Library). This is done to ease the use
of getting values in and out of the xml.

If you don't prefer to use some of the concepts just don't use it.
It is just a wrapper that extends TinyXML. It doesn't actually change
any of TinyXML.
*/
namespace ticpp {
/**
 * This is a ticpp exception class
 */
class TICPP_API Exception : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Forward Declarations for Visitor, and others.
class TICPP_API Document;
class TICPP_API Element;
class TICPP_API Declaration;
class TICPP_API StylesheetReference;
class TICPP_API Text;
class TICPP_API Comment;
class TICPP_API Attribute;

/** Wrapper around TiXmlVisitor */
class TICPP_API Visitor : public TiXmlVisitor {
public:
    // Overload the TiXmlVisitor functions, wrap objects, call ticpp::Visitor functions
    /// @internal
    virtual bool VisitEnter(const TiXmlDocument& doc) final;
    /// @internal
    virtual bool VisitExit(const TiXmlDocument& doc) final;
    /// @internal
    virtual bool VisitEnter(const TiXmlElement& element,
                            const TiXmlAttribute* firstAttribute) final;
    /// @internal
    virtual bool VisitExit(const TiXmlElement& element) final;
    /// @internal
    virtual bool Visit(const TiXmlDeclaration& declaration) final;
    /// @internal
    virtual bool Visit(const TiXmlStylesheetReference& stylesheet) final;
    /// @internal
    virtual bool Visit(const TiXmlText& text) final;
    /// @internal
    virtual bool Visit(const TiXmlComment& comment) final;

    /// Visit a document.
    virtual bool VisitEnter(const Document& /*doc*/) { return true; }
    /// Visit a document.
    virtual bool VisitExit(const Document& /*doc*/) { return true; }
    /// Visit an element.
    virtual bool VisitEnter(const Element& /*element*/, const Attribute* /*firstAttribute*/) {
        return true;
    }
    /// Visit an element.
    virtual bool VisitExit(const Element& /*element*/) { return true; }
    /// Visit a declaration
    virtual bool Visit(const Declaration& /*declaration*/) { return true; }
    /// Visit a stylesheet reference
    virtual bool Visit(const StylesheetReference& /*stylesheet*/) { return true; }
    /// Visit a text node
    virtual bool Visit(const Text& /*text*/) { return true; }
    /// Visit a comment node
    virtual bool Visit(const Comment& /*comment*/) { return true; }
};

/** Wrapper around TiXmlBase */
class TICPP_API Base {
public:
    [[noreturn]] void error(std::string_view message,
                            std::source_location loc = std::source_location::current()) const {
        throw Exception(fmt::format("{} <{}@{}>{}", message, loc.file_name(), loc.line(),
                                    BuildDetailedErrorString()));
    }

    /**
    Converts any class with a proper overload of the << opertor to a std::string
    @param value The value to be converted
    @throws Exception When value cannot be converted to a std::string
    */
    template <class T>
    decltype(auto) ToString(const T& value) const {
        if constexpr (std::is_same_v<T, std::string>) {
            return value;
        } else if constexpr (std::is_arithmetic_v<T>) {
            return fmt::to_string(value);
        } else {
            std::stringstream convert;
            convert << value;
            if (convert.fail()) {
                error("Could not convert value to text");
            }
            return std::move(convert).str();
        }
    }

    /**
    Converts a std::string to any class with a proper overload of the >> opertor
    @param temp			The string to be converted
    @param out	[OUT]	The container for the returned value
    @throws Exception When temp cannot be converted to the target type
    */
    template <class T>
    void FromString(const std::string& temp, T* out) const {
        if constexpr (std::is_same_v<T, std::string>) {
            *out = temp;
        } else if constexpr (std::is_arithmetic_v<T>) {
            const auto answer =
                fast_float::from_chars(temp.data(), temp.data() + temp.size(), *out);
            if (answer.ec != std::errc()) {
                error(fmt::format("Could not convert \"{}\" to target type", temp));
            }
        } else {
            std::istringstream val(temp);
            val >> *out;
            if (val.fail()) {
                error(fmt::format("Could not convert \"{}\" to target type", temp));
            }
        }
    }

    /**
    Return the position, in the original source file, of this node or attribute.
    Wrapper around TiXmlBase::Row()
    */
    int Row() const { return GetBasePointer()->Row(); }

    /**
    Return the position, in the original source file, of this node or attribute.
    Wrapper around TiXmlBase::Row()
    */
    int Column() const { return GetBasePointer()->Column(); }

    /**
    Compare internal TiXml pointers to determine is both are wrappers around the same node
    */
    bool operator==(const Base& rhs) const { return (GetBasePointer() == rhs.GetBasePointer()); }

    /**
    Compare internal TiXml pointers to determine is both are wrappers around the same node
    */
    bool operator!=(const Base& rhs) const { return (GetBasePointer() != rhs.GetBasePointer()); }

    /**
    Builds detailed error string using TiXmlDocument::Error() and others
    */
    std::string BuildDetailedErrorString() const {
        if (auto* node = dynamic_cast<TiXmlNode*>(GetBasePointer())) {
            if (auto* doc = node->GetDocument()) {
                if (doc->Error()) {
                    return fmt::format("\nDescription: {}\nFile: {} \nLine: {}\nColumn: {}",
                                       doc->ErrorDesc(),
                                       (strlen(doc->Value()) > 0 ? doc->Value() : "<unnamed-file>"),
                                       doc->ErrorRow(), doc->ErrorCol());
                }
            }
        }
        return {};
    }

    /**
    Destructor
    */
    virtual ~Base() {}

protected:
    mutable TiCppRCImp* m_impRC; /**< Holds status of internal TiXmlPointer - use this to determine
                                    if object has been deleted already */

    /**
    @internal
    Updates the pointer to the reference counter to point at the counter in the new node.

    @param node TiXmlBase containing the new reference counter
    */
    void SetImpRC(TiXmlBase* node) { m_impRC = node->m_tiRC; }

    void ValidatePointer() const {
        if (m_impRC->IsNull()) {
            error("Internal TiXml Pointer is NULL");
        }
    }

    /**
    @internal
    Get internal TiXmlBase*
    */
    virtual TiXmlBase* GetBasePointer() const = 0;
};

/**
Wrapper around TiXmlAttribute
*/
class TICPP_API Attribute final : public Base {
private:
    TiXmlAttribute* m_tiXmlPointer;
    TiXmlBase* GetBasePointer() const {
        ValidatePointer();
        return m_tiXmlPointer;
    }

public:
    /**
    Construct an empty attribute.
    */
    Attribute();

    /**
    Construct an attribute with @a name and @a value

    @param name The name of the attribute
    @param value The value of the attribute
    */
    Attribute(const std::string& name, const std::string& value);

    /**
    @internal
    Construct an attribute with the internal pointer

    @param attribute The internal pointer
    */
    Attribute(TiXmlAttribute* attribute);

    /**
    @internal
    Updates the reference count for the old and new pointers.
    */
    Attribute(const Attribute& copy);

    /**
    @internal
    Updates the reference count for the old and new pointers.
    */
    Attribute& operator=(const Attribute& copy);

    /*
    Decrements reference count.
    */
    virtual ~Attribute();

    /**
    Get the value of this attribute
    Uses Base::FromString to convert TiXmlAttribute::ValueStr from a std::string,
    and puts it in the passed pointer.

    @param value [OUT] A pointer to fill with the value
    */
    template <class T>
    void GetValue(T* value) const {
        ValidatePointer();
        FromString(m_tiXmlPointer->ValueStr(), value);
    }

    /**
    Get the value of this attribute.
    Simple wrapper for TiXmlAttribute::ValueStr.

    @see GetValue
    */
    const std::string& Value() const;

    /**
    Set the value of this node.
    Uses Base::ToString to convert value to a std::string, then calls TiXmlAttribute::SetValue.

    @param value The value to set
    */
    template <class T>
    void SetValue(const T& value) {
        ValidatePointer();
        m_tiXmlPointer->SetValue(ToString(value));
    }

    /**
    Get the value of this attribute
    Uses Base::FromString to convert TiXmlAttribute::Name from a std::string,
    and puts it in the passed pointer.

    @param name [OUT] A pointer to fill with the name
    */
    template <class T>
    void GetName(T* name) const {
        ValidatePointer();
        FromString(m_tiXmlPointer->Name(), name);
    }

    /**
    Get the value of this attribute.
    Simple wrapper for TiXmlAttribute::Name.

    @see GetName
    */
    const std::string& Name() const;

    /**
    Set the value of this attribute.
    Uses Base::ToString to convert @a name to a std::string, then calls TiXmlAttribute::SetName.

    @param name The name to set
    */
    template <class T>
    void SetName(const T& name) {
        ValidatePointer();
        m_tiXmlPointer->SetName(ToString(name));
    }

    /**
    Get the next sibling attribute in the DOM.
    */
    Attribute* Next(bool throwIfNoAttribute = true) const;

    /**
    Get the previous sibling attribute in the DOM.
    */
    Attribute* Previous(bool throwIfNoAttribute = true) const;

    /**
    @internal
    Just for Iterator<>

    @param next [OUT] The pointer to the next valid attribute
    @return true if there is a next attribute, false if not
    */
    void IterateNext(const std::string&, Attribute** next) const;

    /**
    @internal
    Just for Iterator<>

    @param previous [OUT] The pointer to the previous valid attribute
    @return true if there is a previous attribute, false if not
    */
    void IteratePrevious(const std::string&, Attribute** previous) const;

    /**
    All TinyXml classes can print themselves to a filestream.
    */
    virtual void Print(FILE* file, int depth) const;

private:
    /**
    @internal
    Sets the internal pointer.
    Saves a copy of the pointer to the RC object.

    @param newPointer TiXmlAttribute* to set.
    */
    void SetTiXmlPointer(TiXmlAttribute* newPointer);
};

/**
Wrapper around TiXmlNode
*/
class TICPP_API Node : public Base {
public:
    /**
    Get the value of this node
    Uses Base::FromString to convert TiXmlNode::ValueStr from a std::string,
    and puts it in the passed pointer.

    @param value [OUT] A pointer to fill with the value
    */
    template <class T>
    void GetValue(T* value) const {
        FromString(GetTiXmlPointer()->ValueStr(), value);
    }

    /**
    Get the value of this node.
    Simple wrapper for TiXmlNode::ValueStr.

    @see GetValue
    */
    const std::string& Value() const;

    /**
    Set the value of this node.
    Uses Base::ToString to convert value to a std::string, then calls TiXmlNode::SetValue.

    @param value The value to set
    */
    template <class T>
    void SetValue(const T& value) {
        GetTiXmlPointer()->SetValue(ToString(value));
    }

    void SetValue(const char* value) { GetTiXmlPointer()->SetValue(value); }

    void SetValue(std::string_view value) { GetTiXmlPointer()->SetValue(value); }

    /**
    Clear all Nodes below this.
    Simple wrapper for TiXmlNode::Clear.
    */
    void Clear();

    /**
    The Parent of this Node.
    Simple wrapper for TiXmlNode::Parent.

    @param throwIfNoParent [DEF] If true, throws when Parent = NULL.
    @return The parent of this node, NULL if there is no Parent.
    @throws Exception When throwIfNoParent is true, and TiXmlNode::Parent returns Null.
    */
    Node* Parent(bool throwIfNoParent = true) const;

    /**
    The first child of this node.

    @param throwIfNoChildren [DEF] If true, will throw an exception if there are no children.
    @return Pointer to child, Null if no children and 'throwIfNoChildren' is false.
    @throws Exception When throwIfNoChildren is true, and TiXmlNode::FirstChild returns Null.

    @see TiXmlNode::FirstChild
    */
    Node* FirstChild(bool throwIfNoChildren = true) const;

    /**
    @internal
    The first child of this node with the matching @a value.

    @overload
    @param value				Value to match.
    @param throwIfNoChildren	[DEF] If true, will throw an exception if there are no children.

    @see FirstChild( bool throwIfNoChildren = true )
    */
    Node* FirstChild(const char* value, bool throwIfNoChildren = true) const;

    /**
    The first child of this node with the matching @a value.

    @overload
    @param value				Value to match.
    @param throwIfNoChildren	[DEF] If true, will throw an exception if there are no children.

    @see FirstChild( const char* value, bool throwIfNoChildren = true )
    */
    Node* FirstChild(const std::string& value, bool throwIfNoChildren = true) const;

    /**
    The last child of this node.

    @param throwIfNoChildren [DEF] If true, will throw an exception if there are no children.
    @return Pointer to child, Null if no children and 'throwIfNoChildren' is false.
    @throws Exception When throwIfNoChildren is true, and TiXmlNode::LastChild returns Null.

    @see TiXmlNode::LastChild
    */
    Node* LastChild(bool throwIfNoChildren = true) const;

    /**
    @internal
    The last child of this node with the matching @a value.

    @overload
    @param value				Value to match.
    @param throwIfNoChildren	[DEF] If true, will throw an exception if there are no children.

    @see LastChild( bool throwIfNoChildren = true )
    */
    Node* LastChild(const char* value, bool throwIfNoChildren = true) const;

    /**
    The last child of this node with the matching @a value.

    @overload
    @param value				Value to match.
    @param throwIfNoChildren	[DEF] If true, will throw an exception if there are no children.

    @see LastChild( const char* value, bool throwIfNoChildren = true )
    */
    Node* LastChild(const std::string& value, bool throwIfNoChildren = true) const;

    /**
    An alternate way to walk the children of a node.
    Simple wrapper for TiXmlNode::IterateChildren.

    @param previous The previous Node* that was returned from IterateChildren.
    @return NULL When there are no more children.
    */
    Node* IterateChildren(Node* previous) const;

    /**
    This flavor of IterateChildren searches for children with a particular @a value.
    Simple wrapper for TiXmlNode::IterateChildren.

    @param value	The value you want to search for.
    @param previous The previous Node* that was returned from IterateChildren.
    @return NULL When there are no more children.
    */
    Node* IterateChildren(const std::string& value, Node* previous) const;

    /**
    Adds a child past the LastChild.
    Throws if you try to insert a document.

    @note This takes a copy of @a addThis so it is not as efficiant as LinkEndChild.
    @param addThis Node to insert.
    @throws Exception When TiXmlNode::InsertEndChild returns Null

    @see LinkEndChild
    @see TiXmlNode::InsertEndChild
    */
    Node* InsertEndChild(const Node& addThis);

    /**
    Adds a child past the LastChild.
    Throws if you try to link a document.

    @param childNode Node to link.
    @throws Exception When TiXmlNode::LinkEndChild returns Null.

    @see InsertEndChild
    @see TiXmlNode::LinkEndChild
    */
    Node* LinkEndChild(Node* childNode);

    /**
    Adds a child before the specified child.
    Throws if you try to insert a document.

    @param beforeThis	Node that will have @a addThis linked before.
    @param addThis		Node to insert before.
    @throws Exception When TiXmlNode::InsertBeforeChild returns Null.

    @see InsertAfterChild
    @see TiXmlNode::InsertBeforeChild
    */
    Node* InsertBeforeChild(Node* beforeThis, const Node& addThis);

    /**
    Adds a child after the specified child.
    Throws if you try to insert a document.

    @param afterThis	Node that will have @a addThis linked after.
    @param addThis		Node to insert after.
    @throws Exception When TiXmlNode::InsertAfterChild returns Null.

    @see InsertBeforeChild
    @see TiXmlNode::InsertAfterChild
    */
    Node* InsertAfterChild(Node* afterThis, const Node& addThis);

    /**
    Replace a child of this node.
    Throws if you try to replace with a document.

    @param replaceThis	Node to replace.
    @param withThis		Node that is replacing @a replaceThis.
    @throws Exception When TiXmlNode::ReplaceChild returns Null.

    @see TiXmlNode::ReplaceChild
    */
    Node* ReplaceChild(Node* replaceThis, const Node& withThis);

    /**
    Delete a child of this node.

    @param removeThis Node to delete.
    @throws Exception When removeThis is not a child of this Node.

    @see TiXmlNode::RemoveChild
    */
    void RemoveChild(Node* removeThis);

    /**
    Navigate to a sibling node.
    Wrapper around TiXmlNode::PreviousSibling.

    @param throwIfNoSiblings [DEF] If true, will throw an exception if there are no siblings.
    @return Pointer to sibling, Null if no siblings and 'throwIfNoSiblings' is false.
    @throws Exception When TiXmlNode::PreviousSibling returns Null and 'throwIfNoSiblings' is true.
    */
    Node* PreviousSibling(bool throwIfNoSiblings = true) const;

    /**
    Navigate to a sibling node with the given @a value.

    @overload
    @param value The value of the node to look for.
    @param throwIfNoSiblings [DEF] If true, will throw an exception if there are no siblings.

    @see PreviousSibling( bool throwIfNoSiblings )
    */
    Node* PreviousSibling(const std::string& value, bool throwIfNoSiblings = true) const;

    /**
    @internal
    Navigate to a sibling node with the given @a value.

    @overload
    @param value The value of the node to look for.
    @param throwIfNoSiblings [DEF] If true, will throw an exception if there are no siblings.

    @see PreviousSibling( const std::string& value, bool throwIfNoSiblings )
    */
    Node* PreviousSibling(const char* value, bool throwIfNoSiblings = true) const;

    /**
    Navigate to a sibling node.
    Wrapper around TiXmlNode::NextSibling.

    @param throwIfNoSiblings [DEF] If true, will throw an exception if there are no siblings.
    @return Pointer to sibling, Null if no siblings and 'throwIfNoSiblings' is false.
    @throws Exception When TiXmlNode::NextSibling returns Null and 'throwIfNoSiblings' is true.
    */
    Node* NextSibling(bool throwIfNoSiblings = true) const;

    /**
    Navigate to a sibling node with the given @a value.

    @overload
    @param value The value of the node to look for.
    @param throwIfNoSiblings [DEF] If true, will throw an exception if there are no siblings.

    @see NextSibling( bool throwIfNoSiblings )
    */
    Node* NextSibling(const std::string& value, bool throwIfNoSiblings = true) const;

    /**
    @internal
    Navigate to a sibling node with the given @a value.

    @overload
    @param value The value of the node to look for.
    @param throwIfNoSiblings [DEF] If true, will throw an exception if there are no siblings.

    @see NextSibling( const std::string& value, bool throwIfNoSiblings )
    */
    Node* NextSibling(const char* value, bool throwIfNoSiblings = true) const;

    /**
    @internal
    Just for Iterator<>

    @param value The value of nodes to iterate through
    @param next [OUT] The pointer to the first valid node
    */
    template <class T>
    T* IterateFirst(const std::string& value) const;

    virtual void IterateFirst(const std::string&, Attribute**) const {
        error("Attributes can only be iterated with Elements.");
    }

    /**
    @internal
    Just for Iterator<>

    @param value The value of nodes to iterate through
    @param next [OUT] The pointer to the next valid node
    */
    template <class T>
    void IterateNext(const std::string& value, T** next) const;

    /**
    @internal
    Just for Iterator<>

    @param value The value of nodes to iterate through
    @param previous [OUT] The pointer to the previous valid node
    */
    template <class T>
    void IteratePrevious(const std::string& value, T** previous) const;

    /**
    Navigate to a sibling element.
    Wrapper around TiXmlNode::NextSibling.

    @param throwIfNoSiblings [DEF] If true, will throw an exception if there are no sibling element.
    @return Pointer to sibling, Null if no siblings and 'throwIfNoSiblings' is false.
    @throws Exception When TiXmlNode::NextSibling returns Null and 'throwIfNoSiblings' is true.
    */
    Element* NextSiblingElement(bool throwIfNoSiblings = true) const;

    /**
    Navigate to a sibling element with the given @a value.

    @overload
    @param value The value of the element to look for.
    @param throwIfNoSiblings [DEF] If true, will throw an exception if there are no sibling
    elements.
    @see NextSiblingElement( bool throwIfNoSiblings )
    */
    Element* NextSiblingElement(const std::string& value, bool throwIfNoSiblings = true) const;

    /**
    @internal
    Navigate to a sibling element with the given @a value.

    @overload
    @param value The value of the element to look for.
    @param throwIfNoSiblings [DEF] If true, will throw an exception if there are no sibling
    elements.

    @see NextSiblingElement( const std::string& value, bool throwIfNoSiblings )
    */
    Element* NextSiblingElement(const char* value, bool throwIfNoSiblings = true) const;

    /**
    The first child element of this node.

    @param throwIfNoChildren [DEF] If true, will throw an exception if there are no element
    children.
    @return Pointer to child, Null if no element children and 'throwIfNoChildren' is false.
    @throws Exception When throwIfNoChildren is true, and TiXmlNode::FirstChildElement returns Null.

    @see TiXmlNode::FirstChildElement
    */
    Element* FirstChildElement(bool throwIfNoChildren = true) const;

    /**
    @internal
    The first child element of this node with the matching @a value.

    @overload
    @param value Value to match.
    @param throwIfNoChildren [DEF] If true, will throw an exception if there are no element
    children.

    @see FirstChildElement( bool throwIfNoChildren = true )
    */
    Element* FirstChildElement(const char* value, bool throwIfNoChildren = true) const;

    /**
    The first child element of this node with the matching @a value.

    @overload
    @param value Value to match.
    @param throwIfNoChildren [DEF] If true, will throw an exception if there are no element
    children.

    @see FirstChildElement( const char* value, bool throwIfNoChildren = true )
    */
    Element* FirstChildElement(const std::string& value, bool throwIfNoChildren = true) const;

    Element* FirstChildElement(std::string_view value, bool throwIfNoChildren = true) const;

    /**
    Query the type (as TiXmlNode::NodeType ) of this node.
    */
    int Type() const;

    /**
    Return a pointer to the Document this node lives in.

    @param throwIfNoDocument [DEF] If true, will throw an exception if this node is not linked under
    a Document.
    @return A pointer to the Document this node lives in, NULL if not linked under a Document, and
    'throwIfNoDocument' is false.
    @throws Exception When this node is not linked under a Document and 'throwIfNoDocument' is true.
    */
    Document* GetDocument(bool throwIfNoDocument = true) const;

    /**
    Check if this node has no children.

    @return true if this node has no children.
    */
    bool NoChildren() const;

    /**
    Pointer conversion ( NOT OBJECT CONVERSION ) - replaces TiXmlNode::ToElement,
    TiXmlNode::ToDocument, TiXmlNode::ToComment, etc.

    @throws Exception When the target is not an object of class T
    @warning Some ancient compilers do not support explicit specification of member template
    arguments, which this depends on ( e.g. VC6 ).
    */
    template <class T>
    T* To() const;

    /**
    Pointer conversion - replaces TiXmlNode::ToDocument.

    @throws Exception When this node is not a Document.
    */
    Document* ToDocument() const;

    /**
    Pointer conversion - replaces TiXmlNode::ToElement.

    @throws Exception When this node is not a Element.
    */
    Element* ToElement() const;

    /**
    Pointer conversion - replaces TiXmlNode::ToComment.

    @throws Exception When this node is not a Comment.
    */
    Comment* ToComment() const;

    /**
    Pointer conversion - replaces TiXmlNode::ToText.

    @throws Exception When this node is not a Text.
    */
    Text* ToText() const;

    /**
    Pointer conversion - replaces TiXmlNode::ToDeclaration.

    @throws Exception When this node is not a Declaration.
    */
    Declaration* ToDeclaration() const;

    /**
    Pointer conversion - replaces TiXmlNode::ToStylesheetReference.

    @throws Exception When this node is not a StylesheetReference.
    */
    StylesheetReference* ToStylesheetReference() const;

    /**
    Create an exact duplicate of this node and return it.

    @note Using auto_ptr to manage the memory declared on the heap by TiXmlNode::Clone.
    @code
    // Now using clone
    ticpp::Document doc( "C:\\Test.xml" );
    ticpp::Node* sectionToClone;
    sectionToClone = doc.FirstChild( "settings" );
    std::unique_ptr< ticpp::Node > clonedNode = sectionToClone->Clone();
    // Now you can use the clone.
    ticpp::Node* node2 = clonedNode->FirstChildElement()->FirstChild();
    ...
    // After the variable clonedNode goes out of scope it will automatically be cleaned up.
    @endcode
    @return Pointer the duplicate node.
    */
    std::unique_ptr<Node> Clone() const;

    /**
    Accept a hierarchical visit the nodes in the TinyXML DOM.
    @return The boolean returned by the visitor.
    */
    bool Accept(TiXmlVisitor* visitor) const;

    /**
    Stream input operator.
    */
    friend std::istream& operator>>(std::istream& in, Node& base) {
        in >> *base.GetTiXmlPointer();
        return in;
    }

    /**
    Stream output operator.
    */
    friend std::ostream& operator<<(std::ostream& out, const Node& base) {
        out << *base.GetTiXmlPointer();
        return out;
    }

protected:
    /**
    @internal
    Allows NodeImp to use Node*'s.
    */
    virtual TiXmlNode* GetTiXmlPointer() const = 0;

    TiXmlBase* GetBasePointer() const { return GetTiXmlPointer(); }

    /**
    @internal
    Constructs the correct child of Node, based on the Type of the TiXmlNode*.
    */
    Node* NodeFactory(TiXmlNode* tiXmlNode, bool throwIfNull = true,
                      bool rememberSpawnedWrapper = true) const;
};

/** Iterator for conveniently stepping through Nodes and Attributes.
TinyXML++ introduces iterators:
@code
ticpp::Iterator< ticpp::Node > child;
for ( child = child.begin( parent ); child != child.end(); child++ )
@endcode

Iterators have the added advantage of filtering by type:
@code
// Only iterates through Comment nodes
ticpp::Iterator< ticpp::Comment > child;
for ( child = child.begin( parent ); child != child.end(); child++ )
@endcode

@code
// Only iterates through Element nodes with value "ElementValue"
ticpp::Iterator< ticpp::Element > child( "ElementValue" );
for ( child = child.begin( parent ); child != child.end(); child++ )
@endcode

Finally, Iterators also work with Attributes
@code
ticpp::Iterator< ticpp::Attribute > attribute;
for ( attribute = attribute.begin( element ); attribute != attribute.end(); attribute++ )
@endcode
*/
template <class T = Node>
class Iterator {
private:
    T* m_p;              /**< Internal Pointer */
    std::string m_value; /**< Value for NextSibling  calls */

public:
    /**
    For for loop comparisons.
    @param parent The parent of the nodes to iterate.
    @return The first child of type T.
    @code
    ticpp::Iterator< ticpp::Node > child;
    for ( child = child.begin( parent ); child != child.end(); child++ )
    @endcode
    */
    T* begin(const Node* parent) const { return parent->IterateFirst<T>(m_value); }

    /**
    For for loop comparisons.
    @return NULL
    @code
    ticpp::Iterator< ticpp::Node > child;
    for ( child = child.begin( parent ); child != child.end(); child++ )
    @endcode
    */
    T* end() const { return nullptr; }

    /** Constructor.
    @param value If not empty, this iterator will only visit nodes with matching value.
    @code
    // Only iterates through Element nodes with value "ElementValue"
    ticpp::Iterator< ticpp::Element > child( "ElementValue" );
    for ( child = child.begin( parent ); child != child.end(); child++ )
    @endcode
    */
    Iterator(std::string_view value = "") : m_p(nullptr), m_value(value) {}

    /// Constructor
    Iterator(T* node, std::string_view value = "") : m_p(node), m_value(value) {}

    /// Constructor
    Iterator(const Iterator& it) : m_p(it.m_p), m_value(it.m_value) {}

    /**
    Gets internal pointer.
    @return The internal pointer.
    */
    T* Get() const { return m_p; }

    /** Sets internal pointer */
    Iterator& operator=(const Iterator& it) {
        m_p = it.m_p;
        m_value = it.m_value;
        return *this;
    }

    /** Sets internal pointer */
    Iterator& operator=(T* p) {
        m_p = p;
        return *this;
    }

    /** Sets internal pointer to the Next Sibling, or Iterator::END, if there are no more siblings
     */
    Iterator& operator++() {
        m_p->IterateNext(m_value, &m_p);
        return *this;
    }

    /** Sets internal pointer to the Next Sibling, or Iterator::END, if there are no more siblings
     */
    Iterator operator++(int) {
        Iterator tmp(*this);
        ++(*this);
        return tmp;
    }

    /** Sets internal pointer to the Previous Sibling, or Iterator::END, if there are no prior
     * siblings */
    Iterator& operator--() {
        m_p->IteratePrevious(m_value, &m_p);
        return *this;
    }

    /** Sets internal pointer to the Previous Sibling, or Iterator::END, if there are no prior
     * siblings */
    Iterator operator--(int) {
        Iterator tmp(*this);
        --(*this);
        return tmp;
    }

    /** Compares internal pointer */
    bool operator!=(const T* p) const {
        if (m_p == p) {
            return false;
        }
        if (nullptr == m_p || nullptr == p) {
            return true;
        }
        return *m_p != *p;
    }

    /** Compares internal pointer */
    bool operator!=(const Iterator& it) const { return operator!=(it.m_p); }

    /** Compares internal pointer* */
    bool operator==(T* p) const {
        if (m_p == p) {
            return true;
        }
        if (nullptr == m_p || nullptr == p) {
            return false;
        }
        return *m_p == *p;
    }

    /** Compares internal pointer */
    bool operator==(const Iterator& it) const { return operator==(it.m_p); }

    /** So Iterator behaves like a STL iterator */
    T* operator->() const { return m_p; }

    /** So Iterator behaves like a STL iterator */
    T& operator*() const { return *m_p; }
};

/** Implementation of Node wrapper */
template <class T>
class NodeImp : public Node {
protected:
    T* m_tiXmlPointer; /**< Internal pointer to the TiXml Class which is being wrapped */

    /**
    @internal
    Gets the internal TinyXML pointer.

    @returns The internal TiXmlNode*.
    */
    TiXmlNode* GetTiXmlPointer() const {
        ValidatePointer();
        return m_tiXmlPointer;
    }

    /**
    @internal
    Sets the internal pointer.
    Saves a copy of the pointer to the RC object.

    @param newPointer TiXmlNode* to set.
    */
    void SetTiXmlPointer(T* newPointer) {
        m_tiXmlPointer = newPointer;
        SetImpRC(newPointer);
    }

    /**
    @internal
    Constructor used by child classes.
    */
    NodeImp(T* tiXmlPointer) {
        // Check for NULL pointers
        if (!tiXmlPointer) {
            error(fmt::format("Can not create a {}", typeid(T).name()));
        }
        SetTiXmlPointer(tiXmlPointer);
        m_impRC->IncRef();
    }

    /**
    @internal
    Updates the reference count for the old and new pointers.
    In addition, the spawnedWrappers must be cleared out before a new TiXml object is loaded in.
    */
    void operator=(const NodeImp<T>& that) {
        if (this != &that) {
            // Dropping the reference to the old object
            m_impRC->DecRef();

            // Pointing to the new Object
            SetTiXmlPointer(that.m_tiXmlPointer);

            // The internal tixml pointer changed in the above line
            m_impRC->IncRef();
        }
    }

    /**
    @internal
    Updates the reference count for the old and new pointers.
    In addition, the spawnedWrappers must be cleared out before a new TiXml object is loaded in
    */
    NodeImp(const NodeImp<T>& copy) : Node(copy) {
        // Pointing to the new Object
        SetTiXmlPointer(copy.m_tiXmlPointer);

        // The internal tixml pointer changed in the above line
        m_impRC->IncRef();
    }

public:
    /*
    Deletes the spawned wrapper objects.
    Decrements reference count.
    */
    virtual ~NodeImp() { m_impRC->DecRef(); }
};

/** Wrapper around TiXmlComment */
class TICPP_API Comment final : public NodeImp<TiXmlComment> {
public:
    /**
    Constructor.
    */
    Comment();

    /**
    Constructor.
    */
    Comment(TiXmlComment* comment);

    /**
    Constructor.
    */
    Comment(const std::string& comment);

    virtual ~Comment();
};

/** Wrapper around TiXmlText */
class TICPP_API Text final : public NodeImp<TiXmlText> {
public:
    /**
    Constructor.
    */
    Text();

    /**
    Constructor.
    @overload
    */
    Text(TiXmlText* text);

    virtual ~Text();

    /**
    Constructor.
    @overload
    */
    Text(const std::string& value);

    /**
    Streams value into a string and creates a Text with it.
    Uses ToString to covert the parameter to a string.

    @param value The value of the Text node.
    @throws Exception

    @see TiXmlText
*/
    template <class T>
    Text(const T& value) : NodeImp<TiXmlText>(new TiXmlText(ToString(value))) {
        m_impRC->InitRef();
    }
};

/** Wrapper around TiXmlDocument */
class TICPP_API Document final : public NodeImp<TiXmlDocument> {
public:
    /**
    Default	Constructor.
    Create an empty document, that has no name.
    */
    Document();

    /**
    Constructor.
    */
    Document(TiXmlDocument* document);

    /**
    Constructor.
    */
    Document(const char* documentName);

    /**
     * Constructor.
     * Create a document with a name. The name of the document is also the filename of the xml.
     * @param documentName Name to set in the Document.
     * @note LoadFile() needs to be called to actually load the data from the file specified by
     * documentName SaveFile() needs to be called to save data to file specified by documentName.
     */
    Document(const std::string& documentName);

    virtual ~Document();

    /**
    Load a file using the current document value. Throws if load is unsuccessful.

    @param encoding Sets the documents encoding.
    @see TiXmlEncoding
    @throws Exception
    */
    void LoadFile(TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING);

    /**
    Save a file using the current document value. Throws if it can't save the file.

    @throws Exception
    */
    void SaveFile() const;

    /**
    Load a file using the given filename. Throws if load is unsuccessful.

    @param filename File to load.
    @param encoding Sets the documents encoding.
    @see TiXmlEncoding
    @throws Exception
    */
    void LoadFile(const std::string& filename, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING);

    /**
    @copydoc Document::LoadFile( const std::string&, TiXmlEncoding )
    */
    void LoadFile(const char* filename, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING);

    /**
    Save a file using the given filename. Throws if it can't save the file.

    @param filename File to save.
    @throws Exception
    */
    void SaveFile(const std::string& filename) const;

    /**
    Parse the given xml data.

    @param xml Xml to parse.
    @param throwIfParseError [DEF] If true, throws when there is a parse error.
    @param encoding Sets the documents encoding.
    @throws Exception
    */
    void Parse(const std::string& xml, bool throwIfParseError = true,
               TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING);
};

/** Wrapper around TiXmlElement */
class TICPP_API Element final : public NodeImp<TiXmlElement> {
public:
    Element();
    virtual ~Element();
    /**
    Default	Constructor. Initializes all the variables.
    @param value The value of the element.
    */
    Element(const std::string& value);

    Element(std::string_view value);

    /**
    Default	Constructor. Initializes all the variables.
    @param value The value of the element.
    */
    Element(const char* value);

    /**
    Constructor.
    */
    Element(TiXmlElement* element);

    /**
    Constructor that allows you to set the element text
    @param value The value of the element.
    @param text The text to set.
    */
    template <class T>
    Element(const std::string& value, const T& text)
        : NodeImp<TiXmlElement>(new TiXmlElement(value)) {
        m_impRC->InitRef();
        SetText(text);
    }

    /**
    Access the first attribute in this element.

    @param throwIfNoAttributes [DEF] If true, throws when there are no attributes
    @return The first attribute, NULL if there are none and @a throwIfNoAttributes is true
    */
    Attribute* FirstAttribute(bool throwIfNoAttributes = true) const;

    /**
    Access the last attribute in this element.

    @param throwIfNoAttributes [DEF] If true, throws when there are no attributes
    @return The last attribute, NULL if there are none and @a throwIfNoAttributes is true
    */
    Attribute* LastAttribute(bool throwIfNoAttributes = true) const;

    /**
    @internal
    Just for Iterator<>

    @param value The value of nodes to iterate through
    @param next [OUT] The pointer to the first valid node
    */
    void IterateFirst(const std::string&, Attribute** first) const {
        *first = 0;
        for (Attribute* child = FirstAttribute(false); child; child = child->Next(false)) {
            *first = dynamic_cast<Attribute*>(child);
            if (0 != *first) {
                return;
            }
        }
    }

    /**
    Sets an attribute of name to a given value.
    The attribute will be created if it does not exist, or changed if it does.
    Uses ToString to convert the @a value to a string, so there is no need to use any other
    SetAttribute methods.

    @see GetAttribute
    */
    template <class T>
    void SetAttribute(const std::string& name, const T& value) {
        ValidatePointer();
        if constexpr (std::is_same_v<T, std::string>) {
            m_tiXmlPointer->SetAttribute(name, value);
        } else {
            m_tiXmlPointer->SetAttribute(name, ToString(value));
        }
    }

    void SetAttribute(const char* name, const char* value) {
        ValidatePointer();
        m_tiXmlPointer->SetAttribute(name, value);
    }
    void SetAttribute(std::string_view name, std::string_view value) {
        ValidatePointer();
        m_tiXmlPointer->SetAttribute(name, value);
    }
    void SetAttribute(const char* name, int value) {
        ValidatePointer();
        m_tiXmlPointer->SetAttribute(name, value);
    }

    /**
    Gets the text of an Element.

    @param throwIfNotFound	[DEF]	If true, will throw an exception if there is no text in this
    element
    @note This only works if the Text is the FirstChild node
    @throws Exception When there is no text and throwIfNotFound is true

    @see GetText( T* value, bool throwIfNotFound = false )
    @see GetTextOrDefault
    @see GetTextOrDefault( T* value, const DefaultT& defaultValue )
    @see TiXmlElement::GetText
    */
    const std::string& GetText(bool throwIfNotFound = true) const {
        // Get the element's text value as a std::string
        if (auto* str = GetTextImp()) {
            return *str;
        } else if (throwIfNotFound) {
            error("Text does not exists in the current element");
        }

        static std::string empty;
        return empty;
    }

    /**
    Gets the text of an Element, if it doesn't exist it will return the defaultValue.

    @param defaultValue			What to put in 'value' if there is no text in this element
    @note This only works if the Text is the FirstChild node

    @see GetText
    @see GetText( T* value, bool throwIfNotFound = false )
    @see GetTextOrDefault( T* value, const DefaultT& defaultValue )
    @see TiXmlElement::GetText
    */
    const std::string& GetTextOrDefault(const std::string& defaultValue) const {
        if (auto* str = GetTextImp()) {
            return *str;
        } else {
            return defaultValue;
        }
    }
    const std::string& GetTextOrDefault(std::string&& defaultValue) const = delete;

    /**
    Gets the text value of an Element, if it doesn't exist it will return the defaultValue.
    Uses FromString to convert the string to the type of choice

    @param value		[OUT]	The container for the returned value
    @param defaultValue			What to put in 'value' if there is no text in this element
    @note This is different than GetText() in that it will covert the text to what ever type you
    want.
    @note This only works if the Text is the FirstChild node

    @see GetText
    @see GetText( T* value, bool throwIfNotFound = false )
    @see GetTextOrDefault( const std::string& defaultValue )
    @see TiXmlElement::GetText
    */
    template <class T, class DefaultT>
    void GetTextOrDefault(T* value, const DefaultT& defaultValue) const {
        if (auto* str = GetTextImp()) {
            FromString(*str, value);
        } else {
            *value = defaultValue;
        }
    }

    /**
    Gets the text of an Element.
    Uses FromString to convert the string to the type of choice.

    @param value			[OUT]	The container for the returned value
    @param throwIfNotFound	[DEF]	If true, will throw an exception if there is no text in this
    element
    @note This is different than GetText() in that it will covert the text to what ever type you
    want
    @note This only works if the Text is the FirstChild node
    @throws Exception When there is no text and throwIfNotFound is true

    @see GetText
    @see GetTextOrDefault
    @see GetTextOrDefault( T* value, const DefaultT& defaultValue )
    @see TiXmlElement::GetText
    */
    template <class T>
    void GetText(T* value, bool throwIfNotFound = true) const {
        if (auto* str = GetTextImp()) {
            FromString(*str, value);
        } else if (throwIfNotFound) {
            error("Text does not exists in the current element");
        }
    }

    /**
    Convenience function to set the text of an element.
    Creates a Text node and inserts it as the first child.
    Uses ToString to convert the parameter to a string.

    @param value The text to set.
    */
    template <class T>
    void SetText(const T& value) {
        ValidatePointer();
        std::string temp = ToString(value);

        if (m_tiXmlPointer->NoChildren()) {
            m_tiXmlPointer->LinkEndChild(new TiXmlText(temp));
        } else {
            if (0 == m_tiXmlPointer->GetText()) {
                m_tiXmlPointer->InsertBeforeChild(m_tiXmlPointer->FirstChild(), TiXmlText(temp));
            } else {
                // There already is text, so change it
                m_tiXmlPointer->FirstChild()->SetValue(temp);
            }
        }
    }

    /**
    Gets an attribute of @a name from an element, if it doesn't exist it will return the
    defaultValue. Uses FromString to convert the string to the type of choice.

    @param name			The name of the attribute you are querying.
    @param value		[OUT] The container for the returned value.
    @param defaultValue	What to put in @a value if there is no attribute in this element.
    @throws Exception

    @see GetAttribute
    */
    template <class T, class DefaulT>
    void GetAttributeOrDefault(const std::string& name, T* value,
                               const DefaulT& defaultValue) const {
        if (auto* str = GetAttributePtr(name)) {
            FromString(*str, value);
        } else {
            *value = defaultValue;
        }
    }

    /**
    Gets an attribute of @a name from an element, if it doesn't exist it will return the
    defaultValue.

    @param name			The name of the attribute you are querying.
    @param defaultValue	What to put in @a value if there is no attribute in this element.

    @see GetAttribute
    */
    const std::string& GetAttributeOrDefault(std::string_view name,
                                             const std::string& defaultValue) const;

    [[deprecated(
        "Use GetAttributeOrDefault(std::string_view, const std::string&) or "
        "GetAttribute(std::string_view) or GetAttributePtr(std::string_view)")]] std::string
    GetAttributeOrDefault(const std::string_view name, std::string&& defaultValue) const;

    /**
    Returns an attribute of @a name from an element.
    Uses FromString to convert the string to the type of choice.

    @param name				The name of the attribute you are querying.
    @param throwIfNotFound	[DEF]	If true, will throw an exception if the attribute doesn't
    exist
    @throws Exception When the attribute doesn't exist and throwIfNotFound is true
    @see GetAttributeOrDefault
    */
    template <class T>
    T GetAttribute(std::string_view name, bool throwIfNotFound = true) const {
        T value;
        if (auto* str = GetAttributePtr(name)) {
            FromString(*str, &value);
        } else if (throwIfNotFound) {
            error(fmt::format("Attribute '{}' does not exist", name));
        }
        return value;
    }

    /**
    Gets an attribute of @a name from an element.
    Uses FromString to convert the string to the type of choice.

    @param name				The name of the attribute you are querying.
    @param value			[OUT]	The container for the returned value
    @param throwIfNotFound	[DEF]	If true, will throw an exception if the attribute doesn't
    exist
    @throws Exception When the attribute doesn't exist and throwIfNotFound is true

    @see GetAttributeOrDefault
    */
    template <class T>
    void GetAttribute(std::string_view name, T* value, bool throwIfNotFound = true) const {
        if (auto* str = GetAttributePtr(name)) {
            FromString(*str, value);
        } else if (throwIfNotFound) {
            error(fmt::format("Attribute '{}' does not exist", name));
        }
    }

    /**
    Gets an attribute of @a name from an element.
    Returns an empty string if the attribute does not exist.

    @param name	The name of the attribute you are querying.
    @return The value of the attribute, or an empty string if it does not exist.

    @see GetAttributeOrDefault
    */
    const std::string& GetAttribute(const std::string& name) const;
    const std::string& GetAttribute(const char* name) const;
    const std::string& GetAttribute(std::string_view name) const;

    /**
    Gets an attribute of @a name from an element.
    Returns an nullptr if the attribute does not exist.

    @param name	The name of the attribute you are querying.
    @return a pointer to the value of the attribute, or nullptr if it does not exist.
    */
    const std::string* GetAttributePtr(std::string_view name) const;

    /**
    Returns true, if attribute exists

    @param name The name of the attribute you are checking.
    @return Existence of attribute
    */
    bool HasAttribute(std::string_view name) const;

    /**
    Removes attribute from element.

    @param name The name of the attribute to remove.
    */
    void RemoveAttribute(std::string_view name);

private:
    /**
    @internal
    Implimentation of the GetText, GetTextOrDefault, GetTextValue, and GetTextValueOrDefault
    template methods.
    */
    const std::string* GetTextImp() const;
};

/** Wrapper around TiXmlDeclaration */
class TICPP_API Declaration final : public NodeImp<TiXmlDeclaration> {
public:
    Declaration();
    Declaration(TiXmlDeclaration* declaration);
    Declaration(const std::string_view version, std::string_view encoding,
                std::string_view standalone);

    virtual ~Declaration();
    /**
    Version. Will return an empty string if none was found.
    */
    const std::string& Version() const;

    /**
    Encoding. Will return an empty string if none was found.
    */
    const std::string& Encoding() const;

    /**
    StandAlone. Is this a standalone document?
    */
    const std::string& Standalone() const;
};

/** Wrapper around TiXmlStylesheetReference */
class TICPP_API StylesheetReference final : public NodeImp<TiXmlStylesheetReference> {
public:
    StylesheetReference();
    StylesheetReference(TiXmlStylesheetReference* stylesheetReference);
    StylesheetReference(const std::string& type, const std::string& href);

    virtual ~StylesheetReference();
    /**
    Type. Will return an empty string if none was found.
    */
    const std::string& Type() const;

    /**
    Href. Will return an empty string if none was found.
    */
    const std::string& Href() const;
};

template <class T>
T* Node::IterateFirst(const std::string& value) const {
    for (Node* child = FirstChild(value, false); child; child = child->NextSibling(value, false)) {
        if (T* tChild = dynamic_cast<T*>(child)) {
            return tChild;
        }
    }
    return nullptr;
}

template <class T>
void Node::IterateNext(const std::string& value, T** next) const {
    Node* sibling = NextSibling(value, false);
    *next = dynamic_cast<T*>(sibling);
    while ((nullptr != sibling) && (nullptr == *next)) {
        sibling = sibling->NextSibling(value, false);
        *next = dynamic_cast<T*>(sibling);
    }
}

template <class T>
void Node::IteratePrevious(const std::string& value, T** previous) const {
    Node* sibling = PreviousSibling(value, false);
    *previous = dynamic_cast<T*>(sibling);
    while ((nullptr != sibling) && (nullptr == *previous)) {
        sibling = sibling->PreviousSibling(value, false);
        *previous = dynamic_cast<T*>(sibling);
    }
}

template <class T>
T* Node::To() const {
    if (auto* pointer = dynamic_cast<T*>(this)) {
        return pointer;
    } else {
        const std::string_view thisType = typeid(this).name();
        const std::string_view targetType = typeid(T).name();
        const std::string_view thatType = typeid(*this).name();

        error(fmt::format(
            "The {} could not be casted to a {}*, because the target object is not a "
            "{} . (It is a {})",
            thisType.substr(6), targetType.substr(6), targetType.substr(6), thatType.substr(6)));
        return nullptr;
    }
}

}  // namespace ticpp
