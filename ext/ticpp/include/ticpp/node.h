#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>

#include <ticpp/base.h>
#include <ticpp/printer.h>

#include <memory>
#include <string>
#include <iostream>

/** The parent class for everything in the Document Object Model.
 * (Except for attributes).
 * Nodes have siblings, a parent, and children. A node can be
 * in a document, or stand on its own. The type of a TiXmlNode
 * can be queried, and it can be cast to its more defined type.
 */
class TICPP_API TiXmlNode : public TiXmlBase {
public:
    /** An output stream operator, for every class. Note that this outputs
     * without any newlines or formatting, as opposed to Print(), which
     * includes tabs and new lines.
     *
     * The operator<< and operator>> are not completely symmetric. Writing
     * a node to a stream is very well defined. You'll get a nice stream
     * of output, without any extra whitespace or newlines.
     *
     * But reading is not as well defined. (As it always is.) If you create
     * a TiXmlElement (for example) and read that from an input stream,
     * the text needs to define an element or junk will result. This is
     * true of all input streams, but it's worth keeping in mind.
     *
     * A TiXmlDocument will read nodes until it reads a root element, and
     * all the children of that root element.
     */
    friend std::ostream& operator<<(std::ostream& out, const TiXmlNode& base) {
        TiXmlPrinter printer;
        printer.SetStreamPrinting();
        base.Accept(&printer);
        out << printer.Str();

        return out;
    }

    /// Appends the XML node or attribute to a std::string.
    friend std::string& operator<<(std::string& out, const TiXmlNode& base) {
        TiXmlPrinter printer;
        printer.SetStreamPrinting();
        base.Accept(&printer);
        out.append(printer.Str());

        return out;
    }

    /** The types of XML nodes supported by TinyXml. (All the
        unsupported types are picked up by UNKNOWN.)
    */
    enum NodeType {
        DOCUMENT,
        ELEMENT,
        COMMENT,
        UNKNOWN,
        TEXT,
        DECLARATION,
        STYLESHEETREFERENCE,
        TYPECOUNT
    };

    virtual ~TiXmlNode();

    /** The meaning of 'value' changes for the specific type of
        TiXmlNode.
        @verbatim
        Document:	filename of the xml file
        Element:	name of the element
        Comment:	the comment text
        Unknown:	the tag contents
        Text:		the text string
        @endverbatim

        The subclasses will wrap this function.
    */
    std::string_view Value() const { return value; }

    /** Changes the value of the node. Defined as:
        @verbatim
        Document:	filename of the xml file
        Element:	name of the element
        Comment:	the comment text
        Unknown:	the tag contents
        Text:		the text string
        @endverbatim
    */
    void SetValue(std::string_view _value) { value = _value; }

    /// Delete all the children of this node. Does not affect 'this'.
    void Clear();

    /// One step up the DOM.
    TiXmlNode* Parent() { return parent; }
    const TiXmlNode* Parent() const { return parent; }

    /// The first child of this node. Will be null if there are no children.
    const TiXmlNode* FirstChild() const { return firstChild; }
    TiXmlNode* FirstChild() { return firstChild; }

    /// The first child of this node with the matching 'value'. Will be null if none found.
    const TiXmlNode* FirstChild(std::string_view value) const;

    /// The first child of this node with the matching 'value'. Will be null if none found.
    TiXmlNode* FirstChild(std::string_view _value) {
        return const_cast<TiXmlNode*>((const_cast<const TiXmlNode*>(this))->FirstChild(_value));
    }

    /// The last child of this node. Will be null if there are no children.
    const TiXmlNode* LastChild() const { return lastChild; }
    TiXmlNode* LastChild() { return lastChild; }

    /// The last child of this node matching 'value'. Will be null if there are no children.
    const TiXmlNode* LastChild(std::string_view value) const;
    TiXmlNode* LastChild(std::string_view _value) {
        return const_cast<TiXmlNode*>((const_cast<const TiXmlNode*>(this))->LastChild(_value));
    }

    /** An alternate way to walk the children of a node.
        One way to iterate over nodes is:
        @verbatim
                for( child = parent->FirstChild(); child; child = child->NextSibling() )
        @endverbatim

        IterateChildren does the same thing with the syntax:
        @verbatim
                child = 0;
                while( child = parent->IterateChildren( child ) )
        @endverbatim

        IterateChildren takes the previous child as input and finds
        the next one. If the previous child is null, it returns the
        first. IterateChildren will return null when done.
    */
    const TiXmlNode* IterateChildren(const TiXmlNode* previous) const;
    TiXmlNode* IterateChildren(const TiXmlNode* previous) {
        return const_cast<TiXmlNode*>(
            (const_cast<const TiXmlNode*>(this))->IterateChildren(previous));
    }

    /// This flavor of IterateChildren searches for children with a particular 'value'
    const TiXmlNode* IterateChildren(std::string_view value, const TiXmlNode* previous) const;
    TiXmlNode* IterateChildren(std::string_view _value, const TiXmlNode* previous) {
        return const_cast<TiXmlNode*>(
            (const_cast<const TiXmlNode*>(this))->IterateChildren(_value, previous));
    }

    /** Add a new node related to this. Adds a child past the LastChild.
        Returns a pointer to the new object or NULL if an error occured.
    */
    TiXmlNode* InsertEndChild(const TiXmlNode& addThis);

    /** Add a new node related to this. Adds a child past the LastChild.

            NOTE: the node to be added is passed by pointer, and will be
            henceforth owned (and deleted) by tinyXml. This method is efficient
            and avoids an extra copy, but should be used with care as it
            uses a different memory model than the other insert functions.

            @sa InsertEndChild
    */
    TiXmlNode* LinkEndChild(TiXmlNode* addThis);

    /** Add a new node related to this. Adds a child before the specified child.
        Returns a pointer to the new object or NULL if an error occured.
    */
    TiXmlNode* InsertBeforeChild(TiXmlNode* beforeThis, const TiXmlNode& addThis);

    /** Add a new node related to this. Adds a child after the specified child.
        Returns a pointer to the new object or NULL if an error occured.
    */
    TiXmlNode* InsertAfterChild(TiXmlNode* afterThis, const TiXmlNode& addThis);

    /** Replace a child of this node.
        Returns a pointer to the new object or NULL if an error occured.
    */
    TiXmlNode* ReplaceChild(TiXmlNode* replaceThis, const TiXmlNode& withThis);

    /// Delete a child of this node.
    bool RemoveChild(TiXmlNode* removeThis);

    /// Navigate to a sibling node.
    const TiXmlNode* NextSibling() const { return next; }
    TiXmlNode* NextSibling() { return next; }

    /// Navigate to a sibling node with the given 'value'.
    const TiXmlNode* NextSibling(std::string_view _value) const;
    TiXmlNode* NextSibling(std::string_view _value) {
        return const_cast<TiXmlNode*>((const_cast<const TiXmlNode*>(this))->NextSibling(_value));
    }

    /// Navigate to a sibling node.
    const TiXmlNode* PreviousSibling() const { return prev; }
    TiXmlNode* PreviousSibling() { return prev; }

    /// Navigate to a sibling node.
    const TiXmlNode* PreviousSibling(std::string_view _value) const;
    TiXmlNode* PreviousSibling(std::string_view _value) {
        return const_cast<TiXmlNode*>(
            (const_cast<const TiXmlNode*>(this))->PreviousSibling(_value));
    }

    /** Convenience function to get through elements.
        calls NextSibling and ToElement. Will skip all non-Element
        nodes. Returns 0 if there is not another element.
    */
    const TiXmlElement* NextSiblingElement() const;
    TiXmlElement* NextSiblingElement() {
        return const_cast<TiXmlElement*>(
            (const_cast<const TiXmlNode*>(this))->NextSiblingElement());
    }

    /** Convenience function to get through elements.
            Calls NextSibling and ToElement. Will skip all non-Element
            nodes. Returns 0 if there is not another element.
    */
    const TiXmlElement* NextSiblingElement(std::string_view _value) const;
    TiXmlElement* NextSiblingElement(std::string_view _value) {
        return const_cast<TiXmlElement*>(
            (const_cast<const TiXmlNode*>(this))->NextSiblingElement(_value));
    }

    /// Convenience function to get through elements.
    const TiXmlElement* FirstChildElement() const;
    TiXmlElement* FirstChildElement() {
        return const_cast<TiXmlElement*>((const_cast<const TiXmlNode*>(this))->FirstChildElement());
    }

    const TiXmlElement* FirstChildElement(std::string_view _value) const;
    TiXmlElement* FirstChildElement(std::string_view _value) {
        return const_cast<TiXmlElement*>(
            (const_cast<const TiXmlNode*>(this))->FirstChildElement(_value));
    }

    /** Query the type (as an enumerated value, above) of this node.
        The possible types are: DOCUMENT, ELEMENT, COMMENT,
        UNKNOWN, TEXT, and DECLARATION.
    */
    int Type() const { return type; }

    /** Return a pointer to the Document this node lives in.
        Returns null if not in a document.
    */
    const TiXmlDocument* GetDocument() const;
    TiXmlDocument* GetDocument() {
        return const_cast<TiXmlDocument*>((const_cast<const TiXmlNode*>(this))->GetDocument());
    }

    /// Returns true if this node has no children.
    bool NoChildren() const { return !firstChild; }

    virtual const TiXmlDocument* ToDocument() const {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.
    virtual const TiXmlElement* ToElement() const {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.
    virtual const TiXmlComment* ToComment() const {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.
    virtual const TiXmlUnknown* ToUnknown() const {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.
    virtual const TiXmlText* ToText() const {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.
    virtual const TiXmlDeclaration* ToDeclaration() const {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.
    virtual const TiXmlStylesheetReference* ToStylesheetReference() const {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.

    virtual TiXmlDocument* ToDocument() {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.
    virtual TiXmlElement* ToElement() {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.
    virtual TiXmlComment* ToComment() {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.
    virtual TiXmlUnknown* ToUnknown() {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.
    virtual TiXmlText* ToText() {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.
    virtual TiXmlDeclaration* ToDeclaration() {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.
    virtual TiXmlStylesheetReference* ToStylesheetReference() {
        return nullptr;
    }  ///< Cast to a more defined type. Will return null if not of the requested type.

    /** Create an exact duplicate of this node and return it. The memory must be deleted
        by the caller.
    */
    virtual TiXmlNode* Clone(allocator_type alloc) const = 0;
    TiXmlNode* Clone() const { return Clone(Allocator()); }

    allocator_type Allocator() const { return value.get_allocator(); }

    /** Accept a hierarchical visit the nodes in the TinyXML DOM. Every node in the
        XML tree will be conditionally visited and the host will be called back
        via the TiXmlVisitor interface.

        This is essentially a SAX interface for TinyXML. (Note however it doesn't re-parse
        the XML for the callbacks, so the performance of TinyXML is unchanged by using this
        interface versus any other.)

        The interface has been based on ideas from:

        - http://www.saxproject.org/
        - http://c2.com/cgi/wiki?HierarchicalVisitorPattern

        Which are both good references for "visiting".

        An example of using Accept():
        @verbatim
        TiXmlPrinter printer;
        tinyxmlDoc.Accept( &printer );
        const char* xmlcstr = printer.CStr();
        @endverbatim
    */
    virtual bool Accept(TiXmlVisitor* visitor) const = 0;

protected:
    TiXmlNode(NodeType _type, std::string_view _value = "", allocator_type alloc = {});

    // Copy to the allocated object. Shared functionality between Clone, Copy constructor,
    // and the assignment operator.
    void CopyTo(TiXmlNode* target) const;

    // Figure out what is at *p, and parse it. Returns null if it is not an xml node.
    std::unique_ptr<TiXmlNode> Identify(const char* start, allocator_type alloc);

    std::pmr::string value;
    TiXmlNode* parent;
    TiXmlNode* firstChild;
    TiXmlNode* lastChild;
    TiXmlNode* prev;
    TiXmlNode* next;
    NodeType type;

private:
    TiXmlNode(const TiXmlNode&);            // not implemented.
    void operator=(const TiXmlNode& base);  // not allowed.
};
