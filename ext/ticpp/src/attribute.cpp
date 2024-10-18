#include <ticpp/attribute.h>

#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

#include <cstring>

const TiXmlAttribute* TiXmlAttribute::Next() const {
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if (next->value.empty() && next->name.empty()) return 0;
    return next;
}

const TiXmlAttribute* TiXmlAttribute::Previous() const {
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if (prev->value.empty() && prev->name.empty()) return 0;
    return prev;
}

void TiXmlAttribute::Print(FILE* cfile, int /*depth*/, std::string* str) const {
    std::string n, v;

    EncodeString(name, &n);
    EncodeString(value, &v);

    if (value.find('\"') == std::string::npos) {
        if (cfile) {
            fprintf(cfile, "%s=\"%s\"", n.c_str(), v.c_str());
        }
        if (str) {
            (*str) += n;
            (*str) += "=\"";
            (*str) += v;
            (*str) += "\"";
        }
    } else {
        if (cfile) {
            fprintf(cfile, "%s='%s'", n.c_str(), v.c_str());
        }
        if (str) {
            (*str) += n;
            (*str) += "='";
            (*str) += v;
            (*str) += "'";
        }
    }
}

int TiXmlAttribute::QueryIntValue(int* ival) const {
    if (TIXML_SSCANF(value.c_str(), "%d", ival) == 1) return TIXML_SUCCESS;
    return TIXML_WRONG_TYPE;
}

int TiXmlAttribute::QueryDoubleValue(double* dval) const {
    if (TIXML_SSCANF(value.c_str(), "%lf", dval) == 1) return TIXML_SUCCESS;
    return TIXML_WRONG_TYPE;
}

void TiXmlAttribute::SetIntValue(int _value) {
    char buf[64];
#if defined(TIXML_SNPRINTF)
    TIXML_SNPRINTF(buf, sizeof(buf), "%d", _value);
#else
    sprintf(buf, "%d", _value);
#endif
    SetValue(buf);
}

void TiXmlAttribute::SetDoubleValue(double _value) {
    char buf[256];
#if defined(TIXML_SNPRINTF)
    TIXML_SNPRINTF(buf, sizeof(buf), "%lf", _value);
#else
    sprintf(buf, "%lf", _value);
#endif
    SetValue(buf);
}

int TiXmlAttribute::IntValue() const { return atoi(value.c_str()); }

double TiXmlAttribute::DoubleValue() const { return atof(value.c_str()); }

TiXmlAttributeSet::TiXmlAttributeSet() {
    sentinel.next = &sentinel;
    sentinel.prev = &sentinel;
}

TiXmlAttributeSet::~TiXmlAttributeSet() {
    assert(sentinel.next == &sentinel);
    assert(sentinel.prev == &sentinel);
}

void TiXmlAttributeSet::Add(TiXmlAttribute* addMe) {
    assert(!Find(addMe->Name()));  // Shouldn't be multiply adding to the set.

    addMe->next = &sentinel;
    addMe->prev = sentinel.prev;

    sentinel.prev->next = addMe;
    sentinel.prev = addMe;
}

void TiXmlAttributeSet::Remove(TiXmlAttribute* removeMe) {
    TiXmlAttribute* node;

    for (node = sentinel.next; node != &sentinel; node = node->next) {
        if (node == removeMe) {
            node->prev->next = node->next;
            node->next->prev = node->prev;
            node->next = nullptr;
            node->prev = nullptr;
            return;
        }
    }
    assert(0);  // we tried to remove a non-linked attribute.
}

const TiXmlAttribute* TiXmlAttributeSet::Find(std::string_view name) const {
    for (const TiXmlAttribute* node = sentinel.next; node != &sentinel; node = node->next) {
        if (node->name == name) return node;
    }
    return nullptr;
}

const char* TiXmlAttribute::Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding) {
    p = SkipWhiteSpace(p, encoding);
    if (!p || !*p) return nullptr;

    if (data) {
        data->Stamp(p, encoding);
        location = data->Cursor();
    }
    // Read the name, the '=' and the value.
    const char* pErr = p;
    p = ReadName(p, &name, encoding);
    if (!p || !*p) {
        if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, pErr, data, encoding);
        return 0;
    }
    p = SkipWhiteSpace(p, encoding);
    if (!p || !*p || *p != '=') {
        if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding);
        return nullptr;
    }

    ++p;  // skip '='
    p = SkipWhiteSpace(p, encoding);
    if (!p || !*p) {
        if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding);
        return nullptr;
    }

    const char* end;
    const char SINGLE_QUOTE = '\'';
    const char DOUBLE_QUOTE = '\"';

    if (*p == SINGLE_QUOTE) {
        ++p;
        end = "\'";  // single quote in string
        p = ReadText(p, &value, false, end, false, encoding);
    } else if (*p == DOUBLE_QUOTE) {
        ++p;
        end = "\"";  // double quote in string
        p = ReadText(p, &value, false, end, false, encoding);
    } else {
        // All attribute values should be in single or double quotes.
        // But this is such a common error that the parser will try
        // its best, even without them.
        value = "";
        while (p && *p                                           // existence
               && !IsWhiteSpace(*p) && *p != '\n' && *p != '\r'  // whitespace
               && *p != '/' && *p != '>')                        // tag end
        {
            if (*p == SINGLE_QUOTE || *p == DOUBLE_QUOTE) {
                // [ 1451649 ] Attribute values with trailing quotes not handled correctly
                // We did not have an opening quote but seem to have a
                // closing one. Give up and throw an error.
                if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding);
                return nullptr;
            }
            value += *p;
            ++p;
        }
    }
    return p;
}
