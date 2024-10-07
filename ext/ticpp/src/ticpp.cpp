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

#include "ticpp.h"
#include "ticpprc.h"
#include "tinyxml.h"

#include <sstream>
#include <cstring>

namespace ticpp {

// In the following Visitor functions, casting away const should be safe, as the object can only be
// referred to by a const &
bool Visitor::VisitEnter(const TiXmlDocument& doc) {
    return VisitEnter(Document(const_cast<TiXmlDocument*>(&doc)));
}

bool Visitor::VisitExit(const TiXmlDocument& doc) {
    return VisitEnter(Document(const_cast<TiXmlDocument*>(&doc)));
}

bool Visitor::VisitEnter(const TiXmlElement& element, const TiXmlAttribute* firstAttribute) {
    if (0 != firstAttribute) {
        Attribute attribute(const_cast<TiXmlAttribute*>(firstAttribute));
        return VisitEnter(Element(const_cast<TiXmlElement*>(&element)), &attribute);
    } else {
        return VisitEnter(Element(const_cast<TiXmlElement*>(&element)), 0);
    }
}

bool Visitor::VisitExit(const TiXmlElement& element) {
    return VisitExit(Element(const_cast<TiXmlElement*>(&element)));
}

bool Visitor::Visit(const TiXmlDeclaration& declaration) {
    return Visit(Declaration(const_cast<TiXmlDeclaration*>(&declaration)));
}

bool Visitor::Visit(const TiXmlStylesheetReference& stylesheet) {
    return Visit(StylesheetReference(const_cast<TiXmlStylesheetReference*>(&stylesheet)));
}

bool Visitor::Visit(const TiXmlText& text) { return Visit(Text(const_cast<TiXmlText*>(&text))); }

bool Visitor::Visit(const TiXmlComment& comment) {
    return Visit(Comment(const_cast<TiXmlComment*>(&comment)));
}

Attribute::Attribute() {
    SetTiXmlPointer(new TiXmlAttribute());
    m_impRC->InitRef();
}

Attribute::Attribute(TiXmlAttribute* attribute) {
    SetTiXmlPointer(attribute);
    m_impRC->IncRef();
}

Attribute::Attribute(const std::string& name, const std::string& value) {
    SetTiXmlPointer(new TiXmlAttribute(name, value));
    m_impRC->InitRef();
}

Attribute& Attribute::operator=(const Attribute& that) {
    if (this != &that) {
        // Dropping the reference to the old object
        m_impRC->DecRef();

        // Pointing to the new Object
        SetTiXmlPointer(that.m_tiXmlPointer);

        // The internal tixml pointer changed in the above line
        m_impRC->IncRef();
    }
    return *this;
}

Attribute::Attribute(const Attribute& copy) : Base() {
    // Dropping the reference to the old object
    m_impRC->DecRef();

    // Pointing to the new Object
    SetTiXmlPointer(copy.m_tiXmlPointer);

    // The internal tixml pointer changed in the above line
    m_impRC->IncRef();
}

Attribute::~Attribute() { m_impRC->DecRef(); }

const std::string& Attribute::Value() const {
    ValidatePointer();
    return m_tiXmlPointer->ValueStr();
}

const std::string& Attribute::Name() const {
    ValidatePointer();
    return m_tiXmlPointer->NameTStr();
}

Attribute* Attribute::Next(bool throwIfNoAttribute) const {
    ValidatePointer();
    TiXmlAttribute* attribute = m_tiXmlPointer->Next();
    if (0 == attribute) {
        if (throwIfNoAttribute) {
            TICPPTHROW("No more attributes found")
        } else {
            return 0;
        }
    }

    Attribute* temp = new Attribute(attribute);
    attribute->m_spawnedWrappers.push_back(temp);

    return temp;
}

Attribute* Attribute::Previous(bool throwIfNoAttribute) const {
    ValidatePointer();
    TiXmlAttribute* attribute = m_tiXmlPointer->Previous();
    if (0 == attribute) {
        if (throwIfNoAttribute) {
            TICPPTHROW("No more attributes found")
        } else {
            return 0;
        }
    }

    Attribute* temp = new Attribute(attribute);
    attribute->m_spawnedWrappers.push_back(temp);

    return temp;
}

void Attribute::IterateNext(const std::string&, Attribute** next) const { *next = Next(false); }

void Attribute::IteratePrevious(const std::string&, Attribute** previous) const {
    *previous = Previous(false);
}

void Attribute::Print(FILE* file, int depth) const {
    ValidatePointer();
    m_tiXmlPointer->Print(file, depth);
}

void Attribute::SetTiXmlPointer(TiXmlAttribute* newPointer) {
    m_tiXmlPointer = newPointer;
    SetImpRC(newPointer);
}

//*****************************************************************************

Node* Node::NodeFactory(TiXmlNode* tiXmlNode, bool throwIfNull, bool rememberSpawnedWrapper) const {
    if (0 == tiXmlNode) {
        if (throwIfNull) {
            TICPPTHROW("tiXmlNode is NULL")
        } else {
            return 0;
        }
    }

    Node* temp;
    switch (tiXmlNode->Type()) {
        case TiXmlNode::DOCUMENT:
            temp = new Document(tiXmlNode->ToDocument());
            break;

        case TiXmlNode::ELEMENT:
            temp = new Element(tiXmlNode->ToElement());
            break;

        case TiXmlNode::COMMENT:
            temp = new Comment(tiXmlNode->ToComment());
            break;

        case TiXmlNode::TEXT:
            temp = new Text(tiXmlNode->ToText());
            break;

        case TiXmlNode::DECLARATION:
            temp = new Declaration(tiXmlNode->ToDeclaration());
            break;

        case TiXmlNode::STYLESHEETREFERENCE:
            temp = new StylesheetReference(tiXmlNode->ToStylesheetReference());
            break;

        default:
            TICPPTHROW("Type is unsupported")
    }

    if (rememberSpawnedWrapper) {
        tiXmlNode->m_spawnedWrappers.push_back(temp);
    }
    return temp;
}

const std::string& Node::Value() const { return GetTiXmlPointer()->ValueStr(); }

void Node::Clear() { GetTiXmlPointer()->Clear(); }

Node* Node::Parent(bool throwIfNoParent) const {
    TiXmlNode* parent = GetTiXmlPointer()->Parent();
    if ((0 == parent) && throwIfNoParent) {
        TICPPTHROW("No parent exists");
    }

    return NodeFactory(parent, false);
}

Node* Node::FirstChild(bool throwIfNoChildren) const { return FirstChild("", throwIfNoChildren); }

Node* Node::FirstChild(const std::string& value, bool throwIfNoChildren) const {
    return FirstChild(value.c_str(), throwIfNoChildren);
}

Node* Node::FirstChild(const char* value, bool throwIfNoChildren) const {
    TiXmlNode* childNode = nullptr;
    if (0 == strlen(value)) {
        childNode = GetTiXmlPointer()->FirstChild();
    } else {
        childNode = GetTiXmlPointer()->FirstChild(value);
    }

    if ((nullptr == childNode) && throwIfNoChildren) {
        TICPPTHROW("Child with the value of \"" << value << "\" not found");
    }

    return NodeFactory(childNode, false);
}

Node* Node::LastChild(bool throwIfNoChildren) const { return LastChild("", throwIfNoChildren); }

Node* Node::LastChild(const std::string& value, bool throwIfNoChildren) const {
    return LastChild(value.c_str(), throwIfNoChildren);
}

Node* Node::LastChild(const char* value, bool throwIfNoChildren) const {
    TiXmlNode* childNode;
    if (0 == strlen(value)) {
        childNode = GetTiXmlPointer()->LastChild();
    } else {
        childNode = GetTiXmlPointer()->LastChild(value);
    }

    if ((0 == childNode) && throwIfNoChildren) {
        TICPPTHROW("Child with the value of \"" << value << "\" not found");
    }

    return NodeFactory(childNode, false);
}

Node* Node::IterateChildren(Node* previous) const {
    TiXmlNode* pointer;
    if (0 == previous) {
        pointer = GetTiXmlPointer()->IterateChildren(0);
    } else {
        pointer = GetTiXmlPointer()->IterateChildren(previous->GetTiXmlPointer());
    }

    return NodeFactory(pointer, false);
}

Node* Node::IterateChildren(const std::string& value, Node* previous) const {
    TiXmlNode* pointer;
    if (0 == previous) {
        pointer = GetTiXmlPointer()->IterateChildren(value, 0);
    } else {
        pointer = GetTiXmlPointer()->IterateChildren(value, previous->GetTiXmlPointer());
    }

    return NodeFactory(pointer, false);
}

Node* Node::InsertEndChild(const Node& addThis) {
    if (addThis.Type() == TiXmlNode::DOCUMENT) {
        TICPPTHROW("Node is a Document and can't be inserted");
    }

    TiXmlNode* pointer = GetTiXmlPointer()->InsertEndChild(*addThis.GetTiXmlPointer());
    if (0 == pointer) {
        TICPPTHROW("Node can't be inserted");
    }

    return NodeFactory(pointer);
}

Node* Node::LinkEndChild(Node* childNode) {
    if (childNode->Type() == TiXmlNode::DOCUMENT) {
        TICPPTHROW("Node is a Document and can't be linked");
    }

    // Increment reference count when adding to the tree
    childNode->m_impRC->IncRef();

    if (0 == GetTiXmlPointer()->LinkEndChild(childNode->GetTiXmlPointer())) {
        TICPPTHROW("Node can't be linked");
    }

    return childNode;
}

Node* Node::InsertBeforeChild(Node* beforeThis, const Node& addThis) {
    if (addThis.Type() == TiXmlNode::DOCUMENT) {
        TICPPTHROW("Node is a Document and can't be inserted");
    }

    // Increment reference count when adding to the tree
    addThis.m_impRC->IncRef();

    TiXmlNode* pointer = GetTiXmlPointer()->InsertBeforeChild(beforeThis->GetTiXmlPointer(),
                                                              *addThis.GetTiXmlPointer());
    if (0 == pointer) {
        TICPPTHROW("Node can't be inserted");
    }

    return NodeFactory(pointer);
}

Node* Node::InsertAfterChild(Node* afterThis, const Node& addThis) {
    if (addThis.Type() == TiXmlNode::DOCUMENT) {
        TICPPTHROW("Node is a Document and can't be inserted");
    }

    // Increment reference count when adding to the tree
    addThis.m_impRC->IncRef();

    TiXmlNode* pointer = GetTiXmlPointer()->InsertAfterChild(afterThis->GetTiXmlPointer(),
                                                             *addThis.GetTiXmlPointer());
    if (0 == pointer) {
        TICPPTHROW("Node can't be inserted");
    }

    return NodeFactory(pointer);
}

Node* Node::ReplaceChild(Node* replaceThis, const Node& withThis) {
    if (withThis.Type() == TiXmlNode::DOCUMENT) {
        TICPPTHROW("Node is a Document and can't be inserted");
    }

    // Increment reference count when adding to the tree
    withThis.m_impRC->IncRef();

    TiXmlNode* pointer = GetTiXmlPointer()->ReplaceChild(replaceThis->GetTiXmlPointer(),
                                                         *withThis.GetTiXmlPointer());
    if (0 == pointer) {
        TICPPTHROW("Node can't be inserted");
    }

    return NodeFactory(pointer);
}

void Node::RemoveChild(Node* removeThis) {
    if (!GetTiXmlPointer()->RemoveChild(removeThis->GetTiXmlPointer())) {
        TICPPTHROW("Node to remove (" << removeThis->Value() << ") is not a child of this Node ("
                                      << Value() << ")")
    }
}

Node* Node::PreviousSibling(bool throwIfNoSiblings) const {
    return PreviousSibling("", throwIfNoSiblings);
}

Node* Node::PreviousSibling(const std::string& value, bool throwIfNoSiblings) const {
    return PreviousSibling(value.c_str(), throwIfNoSiblings);
}

Node* Node::PreviousSibling(const char* value, bool throwIfNoSiblings) const {
    TiXmlNode* sibling;
    if (0 == strlen(value)) {
        sibling = GetTiXmlPointer()->PreviousSibling();
    } else {
        sibling = GetTiXmlPointer()->PreviousSibling(value);
    }

    if ((0 == sibling) && throwIfNoSiblings) {
        TICPPTHROW("No Siblings found with value, '" << value << "', Prior to this Node ("
                                                     << Value() << ")")
    }

    return NodeFactory(sibling, false);
}

Node* Node::NextSibling(bool throwIfNoSiblings) const { return NextSibling("", throwIfNoSiblings); }

Node* Node::NextSibling(const std::string& value, bool throwIfNoSiblings) const {
    return NextSibling(value.c_str(), throwIfNoSiblings);
}

Node* Node::NextSibling(const char* value, bool throwIfNoSiblings) const {
    TiXmlNode* sibling;
    if (0 == strlen(value)) {
        sibling = GetTiXmlPointer()->NextSibling();
    } else {
        sibling = GetTiXmlPointer()->NextSibling(value);
    }

    if ((0 == sibling) && throwIfNoSiblings) {
        TICPPTHROW("No Siblings found with value, '" << value << "', After this Node (" << Value()
                                                     << ")")
    }

    return NodeFactory(sibling, false);
}

Element* Node::NextSiblingElement(bool throwIfNoSiblings) const {
    return NextSiblingElement("", throwIfNoSiblings);
}

Element* Node::NextSiblingElement(const std::string& value, bool throwIfNoSiblings) const {
    return NextSiblingElement(value.c_str(), throwIfNoSiblings);
}

Element* Node::NextSiblingElement(const char* value, bool throwIfNoSiblings) const {
    TiXmlElement* sibling;
    if (0 == strlen(value)) {
        sibling = GetTiXmlPointer()->NextSiblingElement();
    } else {
        sibling = GetTiXmlPointer()->NextSiblingElement(value);
    }

    if (0 == sibling) {
        if (throwIfNoSiblings) {
            TICPPTHROW("No Element Siblings found with value, '" << value << "', After this Node ("
                                                                 << Value() << ")")
        } else {
            return 0;
        }
    }

    Element* temp = new Element(sibling);
    sibling->m_spawnedWrappers.push_back(temp);

    return temp;
}

Element* Node::FirstChildElement(bool throwIfNoChildren) const {
    return FirstChildElement("", throwIfNoChildren);
}

Element* Node::FirstChildElement(const std::string& value, bool throwIfNoChildren) const {
    return FirstChildElement(value.c_str(), throwIfNoChildren);
}

Element* Node::FirstChildElement(std::string_view value, bool throwIfNoChildren) const {
    TiXmlElement* element;
    if (value.empty()) {
        element = GetTiXmlPointer()->FirstChildElement();
    } else {
        element = GetTiXmlPointer()->FirstChildElement(value);
    }

    if (0 == element) {
        if (throwIfNoChildren) {
            TICPPTHROW("Element (" << Value() << ") does NOT contain a child with the value of '"
                                   << value << "'")
        } else {
            return 0;
        }
    }

    Element* temp = new Element(element);
    element->m_spawnedWrappers.push_back(temp);

    return temp;
}

Element* Node::FirstChildElement(const char* value, bool throwIfNoChildren) const {
    TiXmlElement* element;
    if (0 == strlen(value)) {
        element = GetTiXmlPointer()->FirstChildElement();
    } else {
        element = GetTiXmlPointer()->FirstChildElement(value);
    }

    if (0 == element) {
        if (throwIfNoChildren) {
            TICPPTHROW("Element (" << Value() << ") does NOT contain a child with the value of '"
                                   << value << "'")
        } else {
            return 0;
        }
    }

    Element* temp = new Element(element);
    element->m_spawnedWrappers.push_back(temp);

    return temp;
}

int Node::Type() const { return GetTiXmlPointer()->Type(); }

Document* Node::GetDocument(bool throwIfNoDocument) const {
    TiXmlDocument* doc = GetTiXmlPointer()->GetDocument();
    if (0 == doc) {
        if (throwIfNoDocument) {
            TICPPTHROW("This node (" << Value() << ") is not linked under a document")
        } else {
            return 0;
        }
    }
    Document* temp = new Document(doc);
    doc->m_spawnedWrappers.push_back(temp);

    return temp;
}

bool Node::NoChildren() const { return GetTiXmlPointer()->NoChildren(); }

Document* Node::ToDocument() const {
    TiXmlDocument* doc = GetTiXmlPointer()->ToDocument();
    if (0 == doc) {
        TICPPTHROW("This node (" << Value() << ") is not a Document")
    }
    Document* temp = new Document(doc);
    doc->m_spawnedWrappers.push_back(temp);

    return temp;
}

Element* Node::ToElement() const {
    TiXmlElement* doc = GetTiXmlPointer()->ToElement();
    if (0 == doc) {
        TICPPTHROW("This node (" << Value() << ") is not a Element")
    }
    Element* temp = new Element(doc);
    doc->m_spawnedWrappers.push_back(temp);

    return temp;
}

Comment* Node::ToComment() const {
    TiXmlComment* doc = GetTiXmlPointer()->ToComment();
    if (0 == doc) {
        TICPPTHROW("This node (" << Value() << ") is not a Comment")
    }
    Comment* temp = new Comment(doc);
    doc->m_spawnedWrappers.push_back(temp);

    return temp;
}

Text* Node::ToText() const {
    TiXmlText* doc = GetTiXmlPointer()->ToText();
    if (0 == doc) {
        TICPPTHROW("This node (" << Value() << ") is not a Text")
    }
    Text* temp = new Text(doc);
    doc->m_spawnedWrappers.push_back(temp);

    return temp;
}

Declaration* Node::ToDeclaration() const {
    TiXmlDeclaration* doc = GetTiXmlPointer()->ToDeclaration();
    if (0 == doc) {
        TICPPTHROW("This node (" << Value() << ") is not a Declaration")
    }
    Declaration* temp = new Declaration(doc);
    doc->m_spawnedWrappers.push_back(temp);

    return temp;
}

StylesheetReference* Node::ToStylesheetReference() const {
    TiXmlStylesheetReference* doc = GetTiXmlPointer()->ToStylesheetReference();
    if (0 == doc) {
        TICPPTHROW("This node (" << Value() << ") is not a StylesheetReference")
    }
    StylesheetReference* temp = new StylesheetReference(doc);
    doc->m_spawnedWrappers.push_back(temp);

    return temp;
}

std::unique_ptr<Node> Node::Clone() const {
    TiXmlNode* node = GetTiXmlPointer()->Clone();
    if (0 == node) {
        TICPPTHROW("Node could not be cloned");
    }
    std::unique_ptr<Node> temp(NodeFactory(node, false, false));

    // Take ownership of the memory from TiXml
    temp->m_impRC->InitRef();

    return temp;
}

bool Node::Accept(TiXmlVisitor* visitor) const { return GetTiXmlPointer()->Accept(visitor); }

//*****************************************************************************

Comment::Comment() : NodeImp<TiXmlComment>(new TiXmlComment()) { m_impRC->InitRef(); }

Comment::Comment(TiXmlComment* comment) : NodeImp<TiXmlComment>(comment) {}

Comment::Comment(const std::string& comment) : NodeImp<TiXmlComment>(new TiXmlComment()) {
    m_impRC->InitRef();
    m_tiXmlPointer->SetValue(comment);
}
Comment::~Comment() {}

//*****************************************************************************

Text::Text() : NodeImp<TiXmlText>(new TiXmlText("")) { m_impRC->InitRef(); }

Text::Text(const std::string& value) : NodeImp<TiXmlText>(new TiXmlText(value)) {
    m_impRC->InitRef();
}

Text::~Text() {}

Text::Text(TiXmlText* text) : NodeImp<TiXmlText>(text) {}

//*****************************************************************************

Document::Document() : NodeImp<TiXmlDocument>(new TiXmlDocument()) { m_impRC->InitRef(); }

Document::Document(TiXmlDocument* document) : NodeImp<TiXmlDocument>(document) {}

Document::Document(const char* documentName)
    : NodeImp<TiXmlDocument>(new TiXmlDocument(documentName)) {
    m_impRC->InitRef();
}

Document::Document(const std::string& documentName)
    : NodeImp<TiXmlDocument>(new TiXmlDocument(documentName)) {
    m_impRC->InitRef();
}

Document::~Document() {}

void Document::LoadFile(TiXmlEncoding encoding) {
    if (!m_tiXmlPointer->LoadFile(encoding)) {
        TICPPTHROW("Couldn't load " << m_tiXmlPointer->Value());
    }
}

void Document::SaveFile(void) const {
    if (!m_tiXmlPointer->SaveFile()) {
        TICPPTHROW("Couldn't save " << m_tiXmlPointer->Value());
    }
}

void Document::LoadFile(const std::string& filename, TiXmlEncoding encoding) {
    if (!m_tiXmlPointer->LoadFile(filename.c_str(), encoding)) {
        TICPPTHROW("Couldn't load " << filename);
    }
}

void Document::LoadFile(const char* filename, TiXmlEncoding encoding) {
    if (!m_tiXmlPointer->LoadFile(filename, encoding)) {
        TICPPTHROW("Couldn't load " << filename);
    }
}

void Document::SaveFile(const std::string& filename) const {
    if (!m_tiXmlPointer->SaveFile(filename.c_str())) {
        TICPPTHROW("Couldn't save " << filename);
    }
}

void Document::Parse(const std::string& xml, bool throwIfParseError, TiXmlEncoding encoding) {
    m_tiXmlPointer->Parse(xml.c_str(), 0, encoding);
    if (throwIfParseError && m_tiXmlPointer->Error()) {
        TICPPTHROW("Error parsing xml.");
    }
}

//*****************************************************************************

Element::Element()
    : NodeImp<TiXmlElement>(
          new TiXmlElement("DefaultValueCausedByCreatingAnElementWithNoParameters")) {
    m_impRC->InitRef();
}
Element::~Element() {}

Element::Element(const std::string& value) : NodeImp<TiXmlElement>(new TiXmlElement(value)) {
    m_impRC->InitRef();
}

Element::Element(std::string_view value) : NodeImp<TiXmlElement>(new TiXmlElement(value)) {
    m_impRC->InitRef();
}

Element::Element(const char* value) : NodeImp<TiXmlElement>(new TiXmlElement(value)) {
    m_impRC->InitRef();
}

Element::Element(TiXmlElement* element) : NodeImp<TiXmlElement>(element) {}

Attribute* Element::FirstAttribute(bool throwIfNoAttributes) const {
    ValidatePointer();
    TiXmlAttribute* attribute = m_tiXmlPointer->FirstAttribute();
    if ((0 == attribute) && throwIfNoAttributes) {
        TICPPTHROW("This Element (" << Value() << ") has no attributes")
    }

    if (0 == attribute) {
        if (throwIfNoAttributes) {
            TICPPTHROW("Element (" << Value() << ") has no attributes")
        } else {
            return 0;
        }
    }

    Attribute* temp = new Attribute(attribute);
    attribute->m_spawnedWrappers.push_back(temp);

    return temp;
}

Attribute* Element::LastAttribute(bool throwIfNoAttributes) const {
    ValidatePointer();
    TiXmlAttribute* attribute = m_tiXmlPointer->LastAttribute();
    if ((0 == attribute) && throwIfNoAttributes) {
        TICPPTHROW("This Element (" << Value() << ") has no attributes")
    }

    if (0 == attribute) {
        if (throwIfNoAttributes) {
            TICPPTHROW("Element (" << Value() << ") has no attributes")
        } else {
            return 0;
        }
    }

    Attribute* temp = new Attribute(attribute);
    attribute->m_spawnedWrappers.push_back(temp);

    return temp;
}

const std::string& Element::GetAttributeOrDefault(std::string_view name,
                                                  const std::string& defaultValue) const {
    if (auto* str = GetAttributePtr(name)) {
        return *str;
    } else {
        return defaultValue;
    }
}

std::string Element::GetAttributeOrDefault(std::string_view name,
                                           std::string&& defaultValue) const {
    if (auto* str = GetAttributePtr(name)) {
        return *str;
    } else {
        return defaultValue;
    }
}

const std::string& Element::GetAttribute(const std::string& name) const {
    static const std::string empty;
    return GetAttributeOrDefault(name, empty);
}

const std::string& ticpp::Element::GetAttribute(const char* name) const {
    ValidatePointer();

    // Get value from TinyXML, if the attribute exists
    if (auto* str = m_tiXmlPointer->AttributeStr(name)) {
        return *str;
    } else {
        static const std::string empty;
        return empty;
    }
}

const std::string& Element::GetAttribute(std::string_view name) const {
    static const std::string empty;
    return GetAttributeOrDefault(name, empty);
}

bool Element::HasAttribute(std::string_view name) const {
    ValidatePointer();
    return (0 != m_tiXmlPointer->Attribute(name));
}

void Element::RemoveAttribute(std::string_view name) {
    ValidatePointer();
    m_tiXmlPointer->RemoveAttribute(name);
}

const std::string* Element::GetAttributePtr(std::string_view name) const {
    ValidatePointer();

    return m_tiXmlPointer->Attribute(name);
}

const std::string* Element::GetTextImp() const {
    ValidatePointer();

    return m_tiXmlPointer->GetTextStr();
}

//*****************************************************************************

Declaration::Declaration() : NodeImp<TiXmlDeclaration>(new TiXmlDeclaration()) {
    m_impRC->InitRef();
}

Declaration::Declaration(TiXmlDeclaration* declaration) : NodeImp<TiXmlDeclaration>(declaration) {}

Declaration::Declaration(std::string_view version, std::string_view encoding,
                         std::string_view standalone)
    : NodeImp<TiXmlDeclaration>(new TiXmlDeclaration(version, encoding, standalone)) {
    m_impRC->InitRef();
}

Declaration::~Declaration() {}

std::string Declaration::Version() const { return m_tiXmlPointer->Version(); }

std::string Declaration::Encoding() const { return m_tiXmlPointer->Encoding(); }

std::string Declaration::Standalone() const { return m_tiXmlPointer->Standalone(); }

//*****************************************************************************

StylesheetReference::StylesheetReference()
    : NodeImp<TiXmlStylesheetReference>(new TiXmlStylesheetReference()) {
    m_impRC->InitRef();
}

StylesheetReference::StylesheetReference(TiXmlStylesheetReference* stylesheetReference)
    : NodeImp<TiXmlStylesheetReference>(stylesheetReference) {}

StylesheetReference::StylesheetReference(const std::string& type, const std::string& href)
    : NodeImp<TiXmlStylesheetReference>(new TiXmlStylesheetReference(type, href)) {
    m_impRC->InitRef();
}

StylesheetReference::~StylesheetReference() {}

std::string StylesheetReference::Type() const { return m_tiXmlPointer->Type(); }

std::string StylesheetReference::Href() const { return m_tiXmlPointer->Href(); }

}  // namespace ticpp
