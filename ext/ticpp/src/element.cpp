#include <ticpp/element.h>

#include <ticpp/attribute.h>
#include <ticpp/document.h>
#include <ticpp/text.h>
#include <ticpp/parsingdata.h>

#include <cstring>

TiXmlElement::TiXmlElement(std::string_view _value, const allocator_type& alloc)
    : TiXmlNode(TiXmlNode::ELEMENT, _value, alloc), attributeSet{alloc} {}

TiXmlElement::TiXmlElement(const TiXmlElement& copy)
    : TiXmlNode(TiXmlNode::ELEMENT), attributeSet{} {
    copy.CopyTo(this);
}

void TiXmlElement::operator=(const TiXmlElement& rhs) {
    ClearThis();
    rhs.CopyTo(this);
}

TiXmlElement::~TiXmlElement() { ClearThis(); }

void TiXmlElement::ClearThis() {
    Clear();
    while (attributeSet.First()) {
        TiXmlAttribute* node = attributeSet.First();
        attributeSet.Remove(node);
        delete node;
    }
}

std::optional<std::string_view> TiXmlElement::Attribute(std::string_view name) const {
    if (const TiXmlAttribute* node = attributeSet.Find(name)) {
        return node->Value();
    }
    return std::nullopt;
}

void TiXmlElement::SetAttribute(std::string_view name, std::string_view _value) {
    if (TiXmlAttribute* node = attributeSet.Find(name)) {
        node->SetValue(_value);
        return;
    }

    if (TiXmlAttribute* attrib = new TiXmlAttribute(name, _value, value.get_allocator())) {
        attributeSet.Add(attrib);
    } else {
        if (TiXmlDocument* document = GetDocument()) {
            document->SetError(TIXML_ERROR_OUT_OF_MEMORY, nullptr, nullptr);
        }
    }
}

void TiXmlElement::RemoveAttribute(std::string_view name) {
    if (auto* node = attributeSet.Find(name)) {
        attributeSet.Remove(node);
        delete node;
    }
}

void TiXmlElement::Print(FILE* cfile, int depth) const {
    int i;
    assert(cfile);
    for (i = 0; i < depth; i++) {
        fprintf(cfile, "    ");
    }

    fprintf(cfile, "<%s", value.c_str());

    const TiXmlAttribute* attrib;
    for (attrib = attributeSet.First(); attrib; attrib = attrib->Next()) {
        fprintf(cfile, " ");
        attrib->Print(cfile, depth);
    }

    // There are 3 different formatting approaches:
    // 1) An element without children is printed as a <foo /> node
    // 2) An element with only a text child is printed as <foo> text </foo>
    // 3) An element with children is printed on multiple lines.
    TiXmlNode* node;
    if (!firstChild) {
        fprintf(cfile, " />");
    } else if (firstChild == lastChild && firstChild->ToText()) {
        fprintf(cfile, ">");
        firstChild->Print(cfile, depth + 1);
        fprintf(cfile, "</%s>", value.c_str());
    } else {
        fprintf(cfile, ">");

        for (node = firstChild; node; node = node->NextSibling()) {
            if (!node->ToText()) {
                fprintf(cfile, "\n");
            }
            node->Print(cfile, depth + 1);
        }
        fprintf(cfile, "\n");
        for (i = 0; i < depth; ++i) {
            fprintf(cfile, "    ");
        }
        fprintf(cfile, "</%s>", value.c_str());
    }
}

void TiXmlElement::CopyTo(TiXmlElement* target) const {
    // superclass:
    TiXmlNode::CopyTo(target);

    // Element class:
    // Clone the attributes, then clone the children.
    for (const TiXmlAttribute* attribute = attributeSet.First(); attribute;
         attribute = attribute->Next()) {
        target->SetAttribute(attribute->Name(), attribute->Value());
    }

    for (TiXmlNode* node = firstChild; node; node = node->NextSibling()) {
        target->LinkEndChild(node->Clone());
    }
}

bool TiXmlElement::Accept(TiXmlVisitor* visitor) const {
    if (visitor->VisitEnter(*this, attributeSet.First())) {
        for (const TiXmlNode* node = FirstChild(); node; node = node->NextSibling()) {
            if (!node->Accept(visitor)) break;
        }
    }
    return visitor->VisitExit(*this);
}

TiXmlNode* TiXmlElement::Clone() const {
    TiXmlElement* clone = new TiXmlElement(Value());
    if (!clone) return nullptr;

    CopyTo(clone);
    return clone;
}

std::optional<std::string_view> TiXmlElement::GetText() const {
    if (const auto* child = this->FirstChild()) {
        if (const auto childText = child->ToText()) {
            return childText->Value();
        }
    }
    return std::nullopt;
}

const char* TiXmlElement::Parse(const char* p, TiXmlParsingData* data,
                                const allocator_type& alloc) {
    p = SkipWhiteSpace(p);
    TiXmlDocument* document = GetDocument();

    if (!p || !*p) {
        if (document) document->SetError(TIXML_ERROR_PARSING_ELEMENT, nullptr, nullptr);
        return nullptr;
    }

    if (data) {
        data->Stamp(p);
        location = data->Cursor();
    }

    if (*p != '<') {
        if (document) document->SetError(TIXML_ERROR_PARSING_ELEMENT, p, data);
        return nullptr;
    }

    p = SkipWhiteSpace(p + 1);

    // Read the name.
    const char* pErr = p;

    p = ReadName(p, &value);
    if (!p || !*p) {
        if (document) document->SetError(TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME, pErr, data);
        return nullptr;
    }

    std::pmr::string endTag("</", alloc);
    endTag += value;
    endTag += ">";

    // Check for and read attributes. Also look for an empty
    // tag or an end tag.
    while (p && *p) {
        pErr = p;
        p = SkipWhiteSpace(p);
        if (!p || !*p) {
            if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, pErr, data);
            return nullptr;
        }
        if (*p == '/') {
            ++p;
            // Empty tag.
            if (*p != '>') {
                if (document) document->SetError(TIXML_ERROR_PARSING_EMPTY, p, data);
                return 0;
            }
            return (p + 1);
        } else if (*p == '>') {
            // Done with attributes (if there were any.)
            // Read the value -- which can include other
            // elements -- read the end tag, and return.
            ++p;
            p = ReadValue(p, data, alloc);  // Note this is an Element method, and will set the
                                            // error if one happens.
            if (!p || !*p) {
                // We were looking for the end tag, but found nothing.
                // Fix for [ 1663758 ] Failure to report error on bad XML
                if (document) document->SetError(TIXML_ERROR_READING_END_TAG, p, data);
                return nullptr;
            }

            // We should find the end tag now
            if (StringEqual(p, endTag.c_str(), false)) {
                p += endTag.length();
                return p;
            } else {
                if (document) document->SetError(TIXML_ERROR_READING_END_TAG, p, data);
                return nullptr;
            }
        } else {
            // Try to read an attribute:
            TiXmlAttribute* attrib = new TiXmlAttribute();
            if (!attrib) {
                if (document) document->SetError(TIXML_ERROR_OUT_OF_MEMORY, pErr, data);
                return nullptr;
            }

            attrib->SetDocument(document);
            pErr = p;
            p = attrib->Parse(p, data, alloc);

            if (!p || !*p) {
                if (document) document->SetError(TIXML_ERROR_PARSING_ELEMENT, pErr, data);
                delete attrib;
                return nullptr;
            }

            // Handle the strange case of double attributes:
            if (TiXmlAttribute* node = attributeSet.Find(attrib->Name())) {
                node->SetValue(attrib->Value());
                delete attrib;
                return nullptr;
            }

            attributeSet.Add(attrib);
        }
    }
    return p;
}

const char* TiXmlElement::ReadValue(const char* p, TiXmlParsingData* data,
                                    const allocator_type& alloc) {
    TiXmlDocument* document = GetDocument();

    // Read in text and elements in any order.
    const char* pWithWhiteSpace = p;
    p = SkipWhiteSpace(p);

    while (p && *p) {
        if (*p != '<') {
            // Take what we have, make a text element.
            TiXmlText* textNode = new TiXmlText("", false, alloc);

            if (!textNode) {
                if (document) document->SetError(TIXML_ERROR_OUT_OF_MEMORY, 0, 0);
                return nullptr;
            }

            if (TiXmlBase::IsWhiteSpaceCondensed()) {
                p = textNode->Parse(p, data, alloc);
            } else {
                // Special case: we want to keep the white space
                // so that leading spaces aren't removed.
                p = textNode->Parse(pWithWhiteSpace, data, alloc);
            }

            if (!textNode->Blank()) {
                LinkEndChild(textNode);
            } else {
                delete textNode;
            }
        } else {
            // We hit a '<'
            // Have we hit a new element or an end tag? This could also be
            // a TiXmlText in the "CDATA" style.
            if (StringEqual(p, "</", false)) {
                return p;
            } else {
                if (TiXmlNode* node = Identify(p, alloc)) {
                    p = node->Parse(p, data, alloc);
                    LinkEndChild(node);
                } else {
                    return nullptr;
                }
            }
        }
        pWithWhiteSpace = p;
        p = SkipWhiteSpace(p);
    }

    if (!p) {
        if (document) document->SetError(TIXML_ERROR_READING_ELEMENT_VALUE, nullptr, nullptr);
    }
    return p;
}
