#include <ticpp/element.h>

#include <ticpp/attribute.h>
#include <ticpp/document.h>
#include <ticpp/text.h>
#include <ticpp/parsingdata.h>

#include <sstream>
#include <cstring>

void TiXmlElement::RemoveAttribute(const char* name) {
    TiXmlAttribute* node = attributeSet.Find(name);

    if (node) {
        attributeSet.Remove(node);
        delete node;
    }
}

void TiXmlElement::RemoveAttribute(std::string_view name) {
    TiXmlAttribute* node = attributeSet.Find(name);

    if (node) {
        attributeSet.Remove(node);
        delete node;
    }
}

const TiXmlElement* TiXmlNode::FirstChildElement() const {
    const TiXmlNode* node;

    for (node = FirstChild(); node; node = node->NextSibling()) {
        if (node->ToElement()) return node->ToElement();
    }
    return 0;
}

const TiXmlElement* TiXmlNode::FirstChildElement(const char* _value) const {
    const TiXmlNode* node;

    for (node = FirstChild(_value); node; node = node->NextSibling(_value)) {
        if (node->ToElement()) return node->ToElement();
    }
    return 0;
}

const TiXmlElement* TiXmlNode::FirstChildElement(std::string_view _value) const {
    const TiXmlNode* node;

    for (node = FirstChild(_value); node; node = node->NextSibling(_value)) {
        if (node->ToElement()) return node->ToElement();
    }
    return 0;
}

const TiXmlElement* TiXmlNode::NextSiblingElement() const {
    const TiXmlNode* node;

    for (node = NextSibling(); node; node = node->NextSibling()) {
        if (node->ToElement()) return node->ToElement();
    }
    return 0;
}

const TiXmlElement* TiXmlNode::NextSiblingElement(const char* _value) const {
    const TiXmlNode* node;

    for (node = NextSibling(_value); node; node = node->NextSibling(_value)) {
        if (node->ToElement()) return node->ToElement();
    }
    return 0;
}

const TiXmlDocument* TiXmlNode::GetDocument() const {
    const TiXmlNode* node;

    for (node = this; node; node = node->parent) {
        if (node->ToDocument()) return node->ToDocument();
    }
    return 0;
}

TiXmlElement::TiXmlElement(const char* _value) : TiXmlNode(TiXmlNode::ELEMENT) {
    firstChild = lastChild = 0;
    value = _value;
}

TiXmlElement::TiXmlElement(const std::string& _value) : TiXmlNode(TiXmlNode::ELEMENT) {
    firstChild = lastChild = 0;
    value = _value;
}

TiXmlElement::TiXmlElement(std::string_view _value) : TiXmlNode(TiXmlNode::ELEMENT) {
    firstChild = lastChild = 0;
    value = _value;
}

TiXmlElement::TiXmlElement(const TiXmlElement& copy) : TiXmlNode(TiXmlNode::ELEMENT) {
    firstChild = lastChild = 0;
    copy.CopyTo(this);
}

void TiXmlElement::operator=(const TiXmlElement& base) {
    ClearThis();
    base.CopyTo(this);
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

const char* TiXmlElement::Attribute(const char* name) const {
    const TiXmlAttribute* node = attributeSet.Find(name);
    if (node) return node->Value();
    return nullptr;
}
const std::string* TiXmlElement::AttributeStr(const char* name) const {
    const TiXmlAttribute* node = attributeSet.Find(name);
    if (node) return &node->ValueStr();
    return nullptr;
}

const std::string* TiXmlElement::Attribute(const std::string& name) const {
    const TiXmlAttribute* node = attributeSet.Find(name);
    if (node) return &node->ValueStr();
    return nullptr;
}

const std::string* TiXmlElement::Attribute(std::string_view name) const {
    const TiXmlAttribute* node = attributeSet.Find(name);
    if (node) return &node->ValueStr();
    return nullptr;
}


const char* TiXmlElement::Attribute(const char* name, int* i) const {
    const char* s = Attribute(name);
    if (i) {
        if (s) {
            *i = atoi(s);
        } else {
            *i = 0;
        }
    }
    return s;
}

const std::string* TiXmlElement::Attribute(const std::string& name, int* i) const {
    const std::string* s = Attribute(name);
    if (i) {
        if (s) {
            *i = atoi(s->c_str());
        } else {
            *i = 0;
        }
    }
    return s;
}

const char* TiXmlElement::Attribute(const char* name, double* d) const {
    const char* s = Attribute(name);
    if (d) {
        if (s) {
            *d = atof(s);
        } else {
            *d = 0;
        }
    }
    return s;
}

const std::string* TiXmlElement::Attribute(const std::string& name, double* d) const {
    const std::string* s = Attribute(name);
    if (d) {
        if (s) {
            *d = atof(s->c_str());
        } else {
            *d = 0;
        }
    }
    return s;
}

int TiXmlElement::QueryIntAttribute(const char* name, int* ival) const {
    const TiXmlAttribute* node = attributeSet.Find(name);
    if (!node) return TIXML_NO_ATTRIBUTE;
    return node->QueryIntValue(ival);
}

int TiXmlElement::QueryIntAttribute(const std::string& name, int* ival) const {
    const TiXmlAttribute* node = attributeSet.Find(name);
    if (!node) return TIXML_NO_ATTRIBUTE;
    return node->QueryIntValue(ival);
}

int TiXmlElement::QueryDoubleAttribute(const char* name, double* dval) const {
    const TiXmlAttribute* node = attributeSet.Find(name);
    if (!node) return TIXML_NO_ATTRIBUTE;
    return node->QueryDoubleValue(dval);
}

int TiXmlElement::QueryDoubleAttribute(const std::string& name, double* dval) const {
    const TiXmlAttribute* node = attributeSet.Find(name);
    if (!node) return TIXML_NO_ATTRIBUTE;
    return node->QueryDoubleValue(dval);
}

void TiXmlElement::SetAttribute(const char* name, int val) {
    char buf[64];
#if defined(TIXML_SNPRINTF)
    TIXML_SNPRINTF(buf, sizeof(buf), "%d", val);
#else
    sprintf(buf, "%d", val);
#endif
    SetAttribute(name, buf);
}

void TiXmlElement::SetAttribute(const std::string& name, int val) {
    std::ostringstream oss;
    oss << val;
    SetAttribute(name, oss.str());
}

void TiXmlElement::SetDoubleAttribute(const char* name, double val) {
    char buf[256];
#if defined(TIXML_SNPRINTF)
    TIXML_SNPRINTF(buf, sizeof(buf), "%f", val);
#else
    sprintf(buf, "%f", val);
#endif
    SetAttribute(name, buf);
}

void TiXmlElement::SetAttribute(const char* cname, const char* cvalue) {
    TiXmlAttribute* node = attributeSet.Find(cname);
    if (node) {
        node->SetValue(cvalue);
        return;
    }

    TiXmlAttribute* attrib = new TiXmlAttribute(cname, cvalue);
    if (attrib) {
        attributeSet.Add(attrib);
    } else {
        TiXmlDocument* document = GetDocument();
        if (document) document->SetError(TIXML_ERROR_OUT_OF_MEMORY, 0, 0, TIXML_ENCODING_UNKNOWN);
    }
}

void TiXmlElement::SetAttribute(const std::string& name, const std::string& _value) {
    TiXmlAttribute* node = attributeSet.Find(name);
    if (node) {
        node->SetValue(_value);
        return;
    }

    TiXmlAttribute* attrib = new TiXmlAttribute(name, _value);
    if (attrib) {
        attributeSet.Add(attrib);
    } else {
        TiXmlDocument* document = GetDocument();
        if (document) document->SetError(TIXML_ERROR_OUT_OF_MEMORY, 0, 0, TIXML_ENCODING_UNKNOWN);
    }
}

void TiXmlElement::SetAttribute(std::string_view name, std::string_view _value) {
    TiXmlAttribute* node = attributeSet.Find(name);
    if (node) {
        node->SetValue(_value);
        return;
    }

    TiXmlAttribute* attrib = new TiXmlAttribute(name, _value);
    if (attrib) {
        attributeSet.Add(attrib);
    } else {
        TiXmlDocument* document = GetDocument();
        if (document) document->SetError(TIXML_ERROR_OUT_OF_MEMORY, 0, 0, TIXML_ENCODING_UNKNOWN);
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
    const TiXmlAttribute* attribute = 0;
    for (attribute = attributeSet.First(); attribute; attribute = attribute->Next()) {
        target->SetAttribute(attribute->Name(), attribute->Value());
    }

    TiXmlNode* node = 0;
    for (node = firstChild; node; node = node->NextSibling()) {
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
    if (!clone) return 0;

    CopyTo(clone);
    return clone;
}

const char* TiXmlElement::GetText() const {
    if (const auto* child = this->FirstChild()) {
        if (const auto childText = child->ToText()) {
            return childText->Value();
        }
    }
    return nullptr;
}

const std::string* TiXmlElement::GetTextStr() const {
    if (const auto* child = this->FirstChild()) {
        if (const auto childText = child->ToText()) {
            return &childText->ValueTStr();
        }
    }
    return nullptr;
}


void TiXmlElement::StreamIn(std::istream* in, std::string* tag) {
    // We're called with some amount of pre-parsing. That is, some of "this"
    // element is in "tag". Go ahead and stream to the closing ">"
    while (in->good()) {
        int c = in->get();
        if (c <= 0) {
            TiXmlDocument* document = GetDocument();
            if (document)
                document->SetError(TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN);
            return;
        }
        (*tag) += (char)c;

        if (c == '>') break;
    }

    if (tag->length() < 3) return;

    // Okay...if we are a "/>" tag, then we're done. We've read a complete tag.
    // If not, identify and stream.

    if (tag->at(tag->length() - 1) == '>' && tag->at(tag->length() - 2) == '/') {
        // All good!
        return;
    } else if (tag->at(tag->length() - 1) == '>') {
        // There is more. Could be:
        //		text
        //		cdata text (which looks like another node)
        //		closing tag
        //		another node.
        for (;;) {
            StreamWhiteSpace(in, tag);

            // Do we have text?
            if (in->good() && in->peek() != '<') {
                // Yep, text.
                TiXmlText text("");
                text.StreamIn(in, tag);

                // What follows text is a closing tag or another node.
                // Go around again and figure it out.
                continue;
            }

            // We now have either a closing tag...or another node.
            // We should be at a "<", regardless.
            if (!in->good()) return;
            assert(in->peek() == '<');
            int tagIndex = (int)tag->length();

            bool closingTag = false;
            bool firstCharFound = false;

            for (;;) {
                if (!in->good()) return;

                int c = in->peek();
                if (c <= 0) {
                    TiXmlDocument* document = GetDocument();
                    if (document)
                        document->SetError(TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN);
                    return;
                }

                if (c == '>') break;

                *tag += (char)c;
                in->get();

                // Early out if we find the CDATA id.
                if (c == '[' && tag->size() >= 9) {
                    size_t len = tag->size();
                    const char* start = tag->c_str() + len - 9;
                    if (strcmp(start, "<![CDATA[") == 0) {
                        assert(!closingTag);
                        break;
                    }
                }

                if (!firstCharFound && c != '<' && !IsWhiteSpace(c)) {
                    firstCharFound = true;
                    if (c == '/') closingTag = true;
                }
            }
            // If it was a closing tag, then read in the closing '>' to clean up the input stream.
            // If it was not, the streaming will be done by the tag.
            if (closingTag) {
                if (!in->good()) return;

                int c = in->get();
                if (c <= 0) {
                    TiXmlDocument* document = GetDocument();
                    if (document)
                        document->SetError(TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN);
                    return;
                }
                assert(c == '>');
                *tag += (char)c;

                // We are done, once we've found our closing tag.
                return;
            } else {
                // If not a closing tag, id it, and stream.
                const char* tagloc = tag->c_str() + tagIndex;
                TiXmlNode* node = Identify(tagloc, TIXML_DEFAULT_ENCODING);
                if (!node) return;
                node->StreamIn(in, tag);
                delete node;
                node = 0;

                // No return: go around from the beginning: text, closing tag, or node.
            }
        }
    }
}

const char* TiXmlElement::Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding) {
    p = SkipWhiteSpace(p, encoding);
    TiXmlDocument* document = GetDocument();

    if (!p || !*p) {
        if (document) document->SetError(TIXML_ERROR_PARSING_ELEMENT, 0, 0, encoding);
        return 0;
    }

    if (data) {
        data->Stamp(p, encoding);
        location = data->Cursor();
    }

    if (*p != '<') {
        if (document) document->SetError(TIXML_ERROR_PARSING_ELEMENT, p, data, encoding);
        return 0;
    }

    p = SkipWhiteSpace(p + 1, encoding);

    // Read the name.
    const char* pErr = p;

    p = ReadName(p, &value, encoding);
    if (!p || !*p) {
        if (document)
            document->SetError(TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME, pErr, data, encoding);
        return 0;
    }

    std::string endTag("</");
    endTag += value;
    endTag += ">";

    // Check for and read attributes. Also look for an empty
    // tag or an end tag.
    while (p && *p) {
        pErr = p;
        p = SkipWhiteSpace(p, encoding);
        if (!p || !*p) {
            if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, pErr, data, encoding);
            return 0;
        }
        if (*p == '/') {
            ++p;
            // Empty tag.
            if (*p != '>') {
                if (document) document->SetError(TIXML_ERROR_PARSING_EMPTY, p, data, encoding);
                return 0;
            }
            return (p + 1);
        } else if (*p == '>') {
            // Done with attributes (if there were any.)
            // Read the value -- which can include other
            // elements -- read the end tag, and return.
            ++p;
            p = ReadValue(p, data, encoding);  // Note this is an Element method, and will set the
                                               // error if one happens.
            if (!p || !*p) {
                // We were looking for the end tag, but found nothing.
                // Fix for [ 1663758 ] Failure to report error on bad XML
                if (document) document->SetError(TIXML_ERROR_READING_END_TAG, p, data, encoding);
                return 0;
            }

            // We should find the end tag now
            if (StringEqual(p, endTag.c_str(), false, encoding)) {
                p += endTag.length();
                return p;
            } else {
                if (document) document->SetError(TIXML_ERROR_READING_END_TAG, p, data, encoding);
                return 0;
            }
        } else {
            // Try to read an attribute:
            TiXmlAttribute* attrib = new TiXmlAttribute();
            if (!attrib) {
                if (document) document->SetError(TIXML_ERROR_OUT_OF_MEMORY, pErr, data, encoding);
                return 0;
            }

            attrib->SetDocument(document);
            pErr = p;
            p = attrib->Parse(p, data, encoding);

            if (!p || !*p) {
                if (document) document->SetError(TIXML_ERROR_PARSING_ELEMENT, pErr, data, encoding);
                delete attrib;
                return 0;
            }

            // Handle the strange case of double attributes:
            TiXmlAttribute* node = attributeSet.Find(attrib->NameTStr());

            if (node) {
                node->SetValue(attrib->Value());
                delete attrib;
                return 0;
            }

            attributeSet.Add(attrib);
        }
    }
    return p;
}

const char* TiXmlElement::ReadValue(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding) {
    TiXmlDocument* document = GetDocument();

    // Read in text and elements in any order.
    const char* pWithWhiteSpace = p;
    p = SkipWhiteSpace(p, encoding);

    while (p && *p) {
        if (*p != '<') {
            // Take what we have, make a text element.
            TiXmlText* textNode = new TiXmlText("");

            if (!textNode) {
                if (document) document->SetError(TIXML_ERROR_OUT_OF_MEMORY, 0, 0, encoding);
                return 0;
            }

            if (TiXmlBase::IsWhiteSpaceCondensed()) {
                p = textNode->Parse(p, data, encoding);
            } else {
                // Special case: we want to keep the white space
                // so that leading spaces aren't removed.
                p = textNode->Parse(pWithWhiteSpace, data, encoding);
            }

            if (!textNode->Blank())
                LinkEndChild(textNode);
            else
                delete textNode;
        } else {
            // We hit a '<'
            // Have we hit a new element or an end tag? This could also be
            // a TiXmlText in the "CDATA" style.
            if (StringEqual(p, "</", false, encoding)) {
                return p;
            } else {
                TiXmlNode* node = Identify(p, encoding);
                if (node) {
                    p = node->Parse(p, data, encoding);
                    LinkEndChild(node);
                } else {
                    return 0;
                }
            }
        }
        pWithWhiteSpace = p;
        p = SkipWhiteSpace(p, encoding);
    }

    if (!p) {
        if (document) document->SetError(TIXML_ERROR_READING_ELEMENT_VALUE, 0, 0, encoding);
    }
    return p;
}
