#include <ticpp/attribute.h>

#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

#include <cstring>

const TiXmlAttribute* TiXmlAttribute::Next() const {
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if (next->value.empty() && next->name.empty()) return nullptr;
    return next;
}

const TiXmlAttribute* TiXmlAttribute::Previous() const {
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if (prev->value.empty() && prev->name.empty()) return nullptr;
    return prev;
}

void TiXmlAttribute::Print(FILE* cfile, int /*depth*/) const {
    if (!cfile) return;
    std::string n, v;

    EncodeString(name, &n);
    EncodeString(value, &v);

    if (value.find('\"') == std::string::npos) {
        fprintf(cfile, "%s=\"%s\"", n.c_str(), v.c_str());
    } else {
        fprintf(cfile, "%s='%s'", n.c_str(), v.c_str());
    }
}

void TiXmlAttribute::Print(std::string* str, int /*depth*/) const {
    if (!str) return;
    std::string n, v;

    EncodeString(name, &n);
    EncodeString(value, &v);

    if (value.find('\"') == std::string::npos) {
        (*str) += n;
        (*str) += "=\"";
        (*str) += v;
        (*str) += "\"";
    } else {
        (*str) += n;
        (*str) += "='";
        (*str) += v;
        (*str) += "'";
    }
}

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

const char* TiXmlAttribute::Parse(const char* p, TiXmlParsingData* data) {
    p = SkipWhiteSpace(p);
    if (!p || !*p) return nullptr;

    if (data) {
        data->Stamp(p);
        location = data->Cursor();
    }
    // Read the name, the '=' and the value.
    const char* pErr = p;
    p = ReadName(p, &name);
    if (!p || !*p) {
        if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, pErr, data);
        return nullptr;
    }
    p = SkipWhiteSpace(p);
    if (!p || !*p || *p != '=') {
        if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, p, data);
        return nullptr;
    }

    ++p;  // skip '='
    p = SkipWhiteSpace(p);
    if (!p || !*p) {
        if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, p, data);
        return nullptr;
    }

    const char* end;
    const char SINGLE_QUOTE = '\'';
    const char DOUBLE_QUOTE = '\"';

    if (*p == SINGLE_QUOTE) {
        ++p;
        end = "\'";  // single quote in string
        p = ReadText(p, &value, false, end, false);
    } else if (*p == DOUBLE_QUOTE) {
        ++p;
        end = "\"";  // double quote in string
        p = ReadText(p, &value, false, end, false);
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
                if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, p, data);
                return nullptr;
            }
            value += *p;
            ++p;
        }
    }
    return p;
}
