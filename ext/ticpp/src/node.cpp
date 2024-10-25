#include <ticpp/node.h>

#include <ticpp/document.h>
#include <ticpp/element.h>
#include <ticpp/comment.h>
#include <ticpp/text.h>
#include <ticpp/declaration.h>
#include <ticpp/unknown.h>
#include <ticpp/stylesheet.h>

#include <ostream>
#include <cstring>

TiXmlNode::TiXmlNode(NodeType _type, std::string_view _value, const allocator_type& alloc)
    : TiXmlBase()
    , value{_value, alloc}
    , parent{nullptr}
    , firstChild{nullptr}
    , lastChild{nullptr}
    , prev{nullptr}
    , next{nullptr}
    , type{_type} {}

TiXmlNode::~TiXmlNode() {
    TiXmlNode* node = firstChild;
    TiXmlNode* temp = nullptr;

    while (node) {
        temp = node;
        node = node->next;
        delete temp;
    }
}

void TiXmlNode::CopyTo(TiXmlNode* target) const { target->SetValue(value); }

void TiXmlNode::Clear() {
    TiXmlNode* node = firstChild;
    TiXmlNode* temp = nullptr;

    while (node) {
        temp = node;
        node = node->next;
        delete temp;
    }

    firstChild = nullptr;
    lastChild = nullptr;
}

TiXmlNode* TiXmlNode::LinkEndChild(TiXmlNode* node) {
    assert(node->parent == nullptr || node->parent == this);
    assert(node->GetDocument() == nullptr || node->GetDocument() == this->GetDocument());

    if (node->Type() == TiXmlNode::DOCUMENT) {
        delete node;
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_DOCUMENT_TOP_ONLY, nullptr, nullptr);
    }

    node->parent = this;

    node->prev = lastChild;
    node->next = nullptr;

    if (lastChild) {
        lastChild->next = node;
    } else {
        firstChild = node;  // it was an empty list.
    }
    lastChild = node;
    return node;
}

TiXmlNode* TiXmlNode::InsertEndChild(const TiXmlNode& addThis) {
    if (addThis.Type() == TiXmlNode::DOCUMENT) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_DOCUMENT_TOP_ONLY, nullptr, nullptr);
    }
    TiXmlNode* node = addThis.Clone();
    if (!node) return nullptr;

    return LinkEndChild(node);
}

TiXmlNode* TiXmlNode::InsertBeforeChild(TiXmlNode* beforeThis, const TiXmlNode& addThis) {
    if (!beforeThis || beforeThis->parent != this) {
        return nullptr;
    }
    if (addThis.Type() == TiXmlNode::DOCUMENT) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_DOCUMENT_TOP_ONLY, nullptr, nullptr);
    }

    TiXmlNode* node = addThis.Clone();
    if (!node) return 0;
    node->parent = this;

    node->next = beforeThis;
    node->prev = beforeThis->prev;
    if (beforeThis->prev) {
        beforeThis->prev->next = node;
    } else {
        assert(firstChild == beforeThis);
        firstChild = node;
    }
    beforeThis->prev = node;
    return node;
}

TiXmlNode* TiXmlNode::InsertAfterChild(TiXmlNode* afterThis, const TiXmlNode& addThis) {
    if (!afterThis || afterThis->parent != this) {
        return nullptr;
    }
    if (addThis.Type() == TiXmlNode::DOCUMENT) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_DOCUMENT_TOP_ONLY, nullptr, nullptr);
    }

    TiXmlNode* node = addThis.Clone();
    if (!node) return nullptr;
    node->parent = this;

    node->prev = afterThis;
    node->next = afterThis->next;
    if (afterThis->next) {
        afterThis->next->prev = node;
    } else {
        assert(lastChild == afterThis);
        lastChild = node;
    }
    afterThis->next = node;
    return node;
}

TiXmlNode* TiXmlNode::ReplaceChild(TiXmlNode* replaceThis, const TiXmlNode& withThis) {
    if (replaceThis->parent != this) return nullptr;

    TiXmlNode* node = withThis.Clone();
    if (!node) return nullptr;

    node->next = replaceThis->next;
    node->prev = replaceThis->prev;

    if (replaceThis->next) {
        replaceThis->next->prev = node;
    } else {
        lastChild = node;
    }

    if (replaceThis->prev) {
        replaceThis->prev->next = node;
    } else {
        firstChild = node;
    }

    delete replaceThis;
    node->parent = this;
    return node;
}

bool TiXmlNode::RemoveChild(TiXmlNode* removeThis) {
    if (removeThis->parent != this) {
        assert(0);
        return false;
    }

    if (removeThis->next) {
        removeThis->next->prev = removeThis->prev;
    } else {
        lastChild = removeThis->prev;
    }

    if (removeThis->prev) {
        removeThis->prev->next = removeThis->next;
    } else {
        firstChild = removeThis->next;
    }

    delete removeThis;
    return true;
}

const TiXmlNode* TiXmlNode::FirstChild(std::string_view _value) const {
    for (const TiXmlNode* node = firstChild; node; node = node->next) {
        if (node->Value() == _value) return node;
    }
    return nullptr;
}

const TiXmlNode* TiXmlNode::LastChild(std::string_view _value) const {
    for (const TiXmlNode* node = lastChild; node; node = node->prev) {
        if (node->Value() == _value) return node;
    }
    return nullptr;
}

const TiXmlNode* TiXmlNode::IterateChildren(const TiXmlNode* previous) const {
    if (!previous) {
        return FirstChild();
    } else {
        assert(previous->parent == this);
        return previous->NextSibling();
    }
}

const TiXmlNode* TiXmlNode::IterateChildren(std::string_view val, const TiXmlNode* previous) const {
    if (!previous) {
        return FirstChild(val);
    } else {
        assert(previous->parent == this);
        return previous->NextSibling(val);
    }
}

const TiXmlNode* TiXmlNode::NextSibling(std::string_view _value) const {
    for (const TiXmlNode* node = next; node; node = node->next) {
        if (node->Value() == _value) return node;
    }
    return nullptr;
}

const TiXmlNode* TiXmlNode::PreviousSibling(std::string_view _value) const {
    for (const TiXmlNode* node = prev; node; node = node->prev) {
        if (node->Value() == _value) return node;
    }
    return nullptr;
}

std::unique_ptr<TiXmlNode> TiXmlNode::Identify(const char* p, const allocator_type& alloc) {
    p = SkipWhiteSpace(p);
    if (!p || !*p || *p != '<') {
        return nullptr;
    }

    // What is this thing?
    // - Elements start with a letter or underscore, but xml is reserved.
    // - Comments: <!--
    // - Declaration: <?xml
    // - StylesheetReference <?xml-stylesheet
    // - Everything else is unknown to tinyxml.

    const char* xmlHeader = {"<?xml"};
    const char* xmlSSHeader = {"<?xml-stylesheet"};
    const char* commentHeader = {"<!--"};
    const char* dtdHeader = {"<!"};
    const char* cdataHeader = {"<![CDATA["};

    std::unique_ptr<TiXmlNode> returnNode = [&]() -> std::unique_ptr<TiXmlNode> {
        if (StringEqual(p, xmlSSHeader, true)) {
            return std::make_unique<TiXmlStylesheetReference>(alloc);
        } else if (StringEqual(p, xmlHeader, true)) {
            return std::make_unique<TiXmlDeclaration>(alloc);
        } else if (StringEqual(p, commentHeader, false)) {
            return std::make_unique<TiXmlComment>(alloc);
        } else if (StringEqual(p, cdataHeader, false)) {
            return std::make_unique<TiXmlText>("", true, alloc);
        } else if (StringEqual(p, dtdHeader, false)) {
            return std::make_unique<TiXmlUnknown>(alloc);
        } else if (IsAlpha(*(p + 1)) || *(p + 1) == '_') {
            return std::make_unique<TiXmlElement>("", alloc);
        } else {
            return std::make_unique<TiXmlUnknown>(alloc);
        }
    }();

    returnNode->parent = this;

    return returnNode;
}

const TiXmlElement* TiXmlNode::FirstChildElement() const {
    for (const TiXmlNode* node = FirstChild(); node; node = node->NextSibling()) {
        if (node->ToElement()) return node->ToElement();
    }
    return nullptr;
}

const TiXmlElement* TiXmlNode::FirstChildElement(std::string_view _value) const {
    for (const TiXmlNode* node = FirstChild(_value); node; node = node->NextSibling(_value)) {
        if (node->ToElement()) return node->ToElement();
    }
    return nullptr;
}

const TiXmlElement* TiXmlNode::NextSiblingElement() const {
    for (const TiXmlNode* node = NextSibling(); node; node = node->NextSibling()) {
        if (node->ToElement()) return node->ToElement();
    }
    return nullptr;
}

const TiXmlElement* TiXmlNode::NextSiblingElement(std::string_view _value) const {
    for (const TiXmlNode* node = NextSibling(_value); node; node = node->NextSibling(_value)) {
        if (node->ToElement()) return node->ToElement();
    }
    return nullptr;
}

const TiXmlDocument* TiXmlNode::GetDocument() const {
    for (const TiXmlNode* node = this; node; node = node->parent) {
        if (node->ToDocument()) return node->ToDocument();
    }
    return nullptr;
}
