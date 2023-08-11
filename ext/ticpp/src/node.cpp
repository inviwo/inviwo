#include <ticpp/node.h>

#include <ticpp/document.h>
#include <ticpp/element.h>
#include <ticpp/comment.h>
#include <ticpp/text.h>
#include <ticpp/declaration.h>
#include <ticpp/unknown.h>
#include <ticpp/stylesheet.h>


#include <ostream>

TiXmlNode::TiXmlNode(NodeType _type) : TiXmlBase() {
    parent = 0;
    type = _type;
    firstChild = 0;
    lastChild = 0;
    prev = 0;
    next = 0;
}

TiXmlNode::~TiXmlNode() {
    TiXmlNode* node = firstChild;
    TiXmlNode* temp = 0;

    while (node) {
        temp = node;
        node = node->next;
        delete temp;
    }
}

void TiXmlNode::CopyTo(TiXmlNode* target) const {
    target->SetValue(value.c_str());
    target->userData = userData;
}

void TiXmlNode::Clear() {
    TiXmlNode* node = firstChild;
    TiXmlNode* temp = 0;

    while (node) {
        temp = node;
        node = node->next;
        delete temp;
    }

    firstChild = 0;
    lastChild = 0;
}

TiXmlNode* TiXmlNode::LinkEndChild(TiXmlNode* node) {
    assert(node->parent == 0 || node->parent == this);
    assert(node->GetDocument() == 0 || node->GetDocument() == this->GetDocument());

    if (node->Type() == TiXmlNode::DOCUMENT) {
        delete node;
        if (GetDocument())
            GetDocument()->SetError(TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN);
        return 0;
    }

    node->parent = this;

    node->prev = lastChild;
    node->next = 0;

    if (lastChild)
        lastChild->next = node;
    else
        firstChild = node;  // it was an empty list.

    lastChild = node;
    return node;
}

TiXmlNode* TiXmlNode::InsertEndChild(const TiXmlNode& addThis) {
    if (addThis.Type() == TiXmlNode::DOCUMENT) {
        if (GetDocument())
            GetDocument()->SetError(TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN);
        return 0;
    }
    TiXmlNode* node = addThis.Clone();
    if (!node) return 0;

    return LinkEndChild(node);
}

TiXmlNode* TiXmlNode::InsertBeforeChild(TiXmlNode* beforeThis, const TiXmlNode& addThis) {
    if (!beforeThis || beforeThis->parent != this) {
        return 0;
    }
    if (addThis.Type() == TiXmlNode::DOCUMENT) {
        if (GetDocument())
            GetDocument()->SetError(TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN);
        return 0;
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
        return 0;
    }
    if (addThis.Type() == TiXmlNode::DOCUMENT) {
        if (GetDocument())
            GetDocument()->SetError(TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN);
        return 0;
    }

    TiXmlNode* node = addThis.Clone();
    if (!node) return 0;
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
    if (replaceThis->parent != this) return 0;

    TiXmlNode* node = withThis.Clone();
    if (!node) return 0;

    node->next = replaceThis->next;
    node->prev = replaceThis->prev;

    if (replaceThis->next)
        replaceThis->next->prev = node;
    else
        lastChild = node;

    if (replaceThis->prev)
        replaceThis->prev->next = node;
    else
        firstChild = node;

    delete replaceThis;
    node->parent = this;
    return node;
}

bool TiXmlNode::RemoveChild(TiXmlNode* removeThis) {
    if (removeThis->parent != this) {
        assert(0);
        return false;
    }

    if (removeThis->next)
        removeThis->next->prev = removeThis->prev;
    else
        lastChild = removeThis->prev;

    if (removeThis->prev)
        removeThis->prev->next = removeThis->next;
    else
        firstChild = removeThis->next;

    delete removeThis;
    return true;
}

const TiXmlNode* TiXmlNode::FirstChild(const char* _value) const {
    const TiXmlNode* node;
    for (node = firstChild; node; node = node->next) {
        if (strcmp(node->Value(), _value) == 0) return node;
    }
    return 0;
}

const TiXmlNode* TiXmlNode::LastChild(const char* _value) const {
    const TiXmlNode* node;
    for (node = lastChild; node; node = node->prev) {
        if (strcmp(node->Value(), _value) == 0) return node;
    }
    return 0;
}

const TiXmlNode* TiXmlNode::IterateChildren(const TiXmlNode* previous) const {
    if (!previous) {
        return FirstChild();
    } else {
        assert(previous->parent == this);
        return previous->NextSibling();
    }
}

const TiXmlNode* TiXmlNode::IterateChildren(const char* val, const TiXmlNode* previous) const {
    if (!previous) {
        return FirstChild(val);
    } else {
        assert(previous->parent == this);
        return previous->NextSibling(val);
    }
}

const TiXmlNode* TiXmlNode::NextSibling(const char* _value) const {
    const TiXmlNode* node;
    for (node = next; node; node = node->next) {
        if (strcmp(node->Value(), _value) == 0) return node;
    }
    return 0;
}

const TiXmlNode* TiXmlNode::PreviousSibling(const char* _value) const {
    const TiXmlNode* node;
    for (node = prev; node; node = node->prev) {
        if (strcmp(node->Value(), _value) == 0) return node;
    }
    return 0;
}



TiXmlNode* TiXmlNode::Identify(const char* p, TiXmlEncoding encoding) {
    TiXmlNode* returnNode = 0;

    p = SkipWhiteSpace(p, encoding);
    if (!p || !*p || *p != '<') {
        return 0;
    }

    TiXmlDocument* doc = GetDocument();
    p = SkipWhiteSpace(p, encoding);

    if (!p || !*p) {
        return 0;
    }

    // What is this thing?
    // - Elements start with a letter or underscore, but xml is reserved.
    // - Comments: <!--
    // - Decleration: <?xml
    // - StylesheetReference <?xml-stylesheet
    // - Everthing else is unknown to tinyxml.
    //

    const char* xmlHeader = {"<?xml"};
    const char* xmlSSHeader = {"<?xml-stylesheet"};
    const char* commentHeader = {"<!--"};
    const char* dtdHeader = {"<!"};
    const char* cdataHeader = {"<![CDATA["};

    if (StringEqual(p, xmlSSHeader, true, encoding)) {
#ifdef DEBUG_PARSER
        TIXML_LOG("XML parsing Stylesheet Reference\n");
#endif
        returnNode = new TiXmlStylesheetReference();
    } else if (StringEqual(p, xmlHeader, true, encoding)) {
#ifdef DEBUG_PARSER
        TIXML_LOG("XML parsing Declaration\n");
#endif
        returnNode = new TiXmlDeclaration();
    } else if (StringEqual(p, commentHeader, false, encoding)) {
#ifdef DEBUG_PARSER
        TIXML_LOG("XML parsing Comment\n");
#endif
        returnNode = new TiXmlComment();
    } else if (StringEqual(p, cdataHeader, false, encoding)) {
#ifdef DEBUG_PARSER
        TIXML_LOG("XML parsing CDATA\n");
#endif
        TiXmlText* text = new TiXmlText("");
        text->SetCDATA(true);
        returnNode = text;
    } else if (StringEqual(p, dtdHeader, false, encoding)) {
#ifdef DEBUG_PARSER
        TIXML_LOG("XML parsing Unknown(1)\n");
#endif
        returnNode = new TiXmlUnknown();
    } else if (IsAlpha(*(p + 1), encoding) || *(p + 1) == '_') {
#ifdef DEBUG_PARSER
        TIXML_LOG("XML parsing Element\n");
#endif
        returnNode = new TiXmlElement("");
    } else {
#ifdef DEBUG_PARSER
        TIXML_LOG("XML parsing Unknown(2)\n");
#endif
        returnNode = new TiXmlUnknown();
    }

    if (returnNode) {
        // Set the parent, so it can report errors
        returnNode->parent = this;
    } else {
        if (doc) doc->SetError(TIXML_ERROR_OUT_OF_MEMORY, 0, 0, TIXML_ENCODING_UNKNOWN);
    }
    return returnNode;
}
