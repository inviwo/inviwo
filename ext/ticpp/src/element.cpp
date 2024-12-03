#include <ticpp/element.h>

#include <ticpp/attribute.h>
#include <ticpp/document.h>
#include <ticpp/text.h>
#include <ticpp/parsingdata.h>

#include <cstring>

TiXmlElement::TiXmlElement(std::string_view _value, allocator_type alloc)
    : TiXmlNode(TiXmlNode::ELEMENT, _value, alloc), attributeSet{alloc} {}

TiXmlElement::TiXmlElement(const TiXmlElement& copy)
    : TiXmlNode(TiXmlNode::ELEMENT), attributeSet{} {
    copy.CopyTo(this);
}

void TiXmlElement::operator=(const TiXmlElement& rhs) {
    Clear();
    attributeSet.Clear();
    rhs.CopyTo(this);
}

TiXmlElement::~TiXmlElement() { Clear(); }

std::optional<std::string_view> TiXmlElement::Attribute(std::string_view name) const {
    if (const TiXmlAttribute* node = attributeSet.Find(name)) {
        return node->Value();
    }
    return std::nullopt;
}

void TiXmlElement::SetAttribute(std::string_view name, std::string_view _value) {
    if (TiXmlAttribute* attribute = attributeSet.Find(name)) {
        attribute->SetValue(_value);
        return;
    }
    attributeSet.Add(name, _value);
}

void TiXmlElement::AddAttribute(std::string_view name, std::string_view _value) {
    attributeSet.Add(name, _value);
}

void TiXmlElement::RemoveAttribute(std::string_view name) { attributeSet.Remove(name); }

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
        target->LinkEndChild(node->Clone(target->Allocator()));
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

TiXmlNode* TiXmlElement::Clone(allocator_type alloc) const {
    auto* clone = alloc.new_object<TiXmlElement>(Value());
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

const char* TiXmlElement::Parse(const char* p, TiXmlParsingData* data, allocator_type alloc) {
    p = SkipWhiteSpace(p);

    if (!p || !*p) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_PARSING_ELEMENT, nullptr, nullptr);
    }

    if (*p != '<') {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_PARSING_ELEMENT, p, data);
    }

    p = SkipWhiteSpace(p + 1);

    // Read the name.
    const char* pErr = p;

    p = ReadName(p, &value);
    if (!p || !*p) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME, pErr, data);
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
            throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_READING_ATTRIBUTES, pErr, data);
        }
        if (*p == '/') {
            ++p;
            // Empty tag.
            if (*p != '>') {
                throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_PARSING_EMPTY, p, data);
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
                throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_READING_END_TAG, p, data);
            }

            // We should find the end tag now
            if (StringEqual(p, endTag.c_str(), false)) {
                p += endTag.length();
                return p;
            } else {
                throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_READING_END_TAG, p, data);
            }
        } else {
            p = attributeSet.Parse(p, data);
        }
    }
    return p;
}

const char* TiXmlElement::ReadValue(const char* p, TiXmlParsingData* data, allocator_type alloc) {
    // Read in text and elements in any order.
    const char* pWithWhiteSpace = p;
    p = SkipWhiteSpace(p);

    while (p && *p) {
        if (*p != '<') {
            // Take what we have, make a text element.
            auto textNode = std::make_unique<TiXmlText>("", false, alloc);

            if (TiXmlBase::IsWhiteSpaceCondensed()) {
                p = textNode->Parse(p, data, alloc);
            } else {
                // Special case: we want to keep the white space
                // so that leading spaces aren't removed.
                p = textNode->Parse(pWithWhiteSpace, data, alloc);
            }

            if (!textNode->Blank()) {
                LinkEndChild(textNode.release());
            }
        } else {
            // We hit a '<'
            // Have we hit a new element or an end tag? This could also be
            // a TiXmlText in the "CDATA" style.
            if (StringEqual(p, "</", false)) {
                return p;
            } else {
                if (std::unique_ptr<TiXmlNode> node = Identify(p, alloc)) {
                    p = node->Parse(p, data, alloc);
                    LinkEndChild(node.release());
                } else {
                    return nullptr;
                }
            }
        }
        pWithWhiteSpace = p;
        p = SkipWhiteSpace(p);
    }

    if (!p) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_READING_ELEMENT_VALUE, nullptr, nullptr);
    }
    return p;
}
