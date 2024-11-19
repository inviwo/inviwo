#include <ticpp/attribute.h>

#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

#include <cstring>

#include <fmt/printf.h>

TiXmlAttribute::TiXmlAttribute(allocator_type alloc)
    : name{alloc}, value{alloc}, prev{nullptr}, next{nullptr} {}

TiXmlAttribute::TiXmlAttribute(std::string_view _name, std::string_view _value,
                               allocator_type alloc)
    : name{_name, alloc}, value{_value, alloc}, prev{nullptr}, next{nullptr} {}

TiXmlAttribute::~TiXmlAttribute() = default;


void TiXmlAttribute::Print(FILE* file) const {
    if (!file) return;
    std::pmr::string n{value.get_allocator()};
    std::pmr::string v{value.get_allocator()};

    TiXmlBase::EncodeString(name, n);
    TiXmlBase::EncodeString(value, v);

    if (value.find('\"') == std::pmr::string::npos) {
        fmt::fprintf(file, "%s=\"%s\"", n, v);
    } else {
        fmt::fprintf(file, "%s='%s'", n, v);
    }
}

TiXmlAttributeSet::TiXmlAttributeSet(allocator_type alloc) : sentinel{alloc} {
    sentinel.next = &sentinel;
    sentinel.prev = &sentinel;
}

TiXmlAttributeSet::~TiXmlAttributeSet() { Clear(); }

std::pmr::string& TiXmlAttributeSet::Add(std::string_view name) {
    if (Find(name)) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_DUPLICATE_ATTRIBUTE, nullptr, nullptr);
    }

    auto alloc = sentinel.value.get_allocator();
    auto* attribute = alloc.new_object<TiXmlAttribute>(name, "");

    attribute->next = &sentinel;
    attribute->prev = sentinel.prev;

    sentinel.prev->next = attribute;
    sentinel.prev = attribute;

    return attribute->ValueRef();
}

void TiXmlAttributeSet::Add(std::string_view name, std::string_view value) {
    if (Find(name)) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_DUPLICATE_ATTRIBUTE, nullptr, nullptr);
    }

    auto alloc = sentinel.value.get_allocator();
    auto* attribute = alloc.new_object<TiXmlAttribute>(name, value);

    attribute->next = &sentinel;
    attribute->prev = sentinel.prev;

    sentinel.prev->next = attribute;
    sentinel.prev = attribute;
}

void TiXmlAttributeSet::Remove(std::string_view name) {
    for (TiXmlAttribute* node = sentinel.next; node != &sentinel; node = node->next) {
        if (node->name == name) {
            node->prev->next = node->next;
            node->next->prev = node->prev;
            node->next = nullptr;
            node->prev = nullptr;

            auto alloc = sentinel.value.get_allocator();
            alloc.delete_object(node);
            return;
        }
    }
}
void TiXmlAttributeSet::Clear() {
    TiXmlAttribute* node = sentinel.next;
    auto alloc = sentinel.value.get_allocator();
    while (node != &sentinel) {
        auto temp = node;
        node = node->next;
        alloc.delete_object(temp);
    }
    sentinel.next = &sentinel;
    sentinel.prev = &sentinel;
}



const char* TiXmlAttributeSet::Parse(const char* p, TiXmlParsingData* data) {
    p = TiXmlBase::SkipWhiteSpace(p);
    if (!p || !*p) return nullptr;

    auto alloc = sentinel.value.get_allocator();
    auto attribute = pmr_make_unique<TiXmlAttribute>(alloc);

    const char* pErr = p;
    p = TiXmlBase::ReadNameValue(p, &attribute->name, &attribute->value, data);

    if (Find(attribute->name)) {
        throw TiXmlError(TiXmlErrorCode::TIXML_ERROR_DUPLICATE_ATTRIBUTE, pErr, data);
    }

    attribute->next = &sentinel;
    attribute->prev = sentinel.prev;
    sentinel.prev->next = attribute.get();
    sentinel.prev = attribute.get();
    attribute.release();

    return p;
}
