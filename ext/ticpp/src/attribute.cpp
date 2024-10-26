#include <ticpp/attribute.h>

#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

#include <cstring>

#include <fmt/printf.h>

TiXmlAttribute::TiXmlAttribute(const allocator_type& alloc)
    : name{alloc}, value{alloc}, prev{nullptr}, next{nullptr}, location{} {}

TiXmlAttribute::TiXmlAttribute(TiXmlCursor _location, const allocator_type& alloc)
    : name{alloc}, value{alloc}, prev{nullptr}, next{nullptr}, location{_location} {}

TiXmlAttribute::TiXmlAttribute(std::string_view _name, std::string_view _value,
                               const allocator_type& alloc)
    : name{_name, alloc}, value{_value, alloc}, prev{nullptr}, next{nullptr}, location{} {}

TiXmlAttribute::~TiXmlAttribute() = default;

const TiXmlAttribute* TiXmlAttribute::Next() const {
    // We are using knowledge of the sentinel. The sentinel must have a value or name.
    if (next->value.empty() && next->name.empty()) return nullptr;
    return next;
}

const TiXmlAttribute* TiXmlAttribute::Previous() const {
    // We are using knowledge of the sentinel. The sentinel must have a value or name.
    if (prev->value.empty() && prev->name.empty()) return nullptr;
    return prev;
}

TiXmlAttribute* TiXmlAttribute::Next() {
    // We are using knowledge of the sentinel. The sentinel have a value or name.
    if (next->value.empty() && next->name.empty()) return nullptr;
    return next;
}

TiXmlAttribute* TiXmlAttribute::Previous() {
    // We are using knowledge of the sentinel. The sentinel have a value or name.
    if (prev->value.empty() && prev->name.empty()) return nullptr;
    return prev;
}

void TiXmlAttribute::Print(FILE* file) const {
    if (!file) return;
    std::string n;
    std::string v;

    TiXmlBase::EncodeString(name, &n);
    TiXmlBase::EncodeString(value, &v);

    if (value.find('\"') == std::string::npos) {
        fmt::fprintf(file, "%s=\"%s\"", n, v);
    } else {
        fmt::fprintf(file, "%s='%s'", n, v);
    }
}

void TiXmlAttribute::Print(std::string* str) const {
    if (!str) return;

    if (value.find('\"') == std::string::npos) {
        TiXmlBase::EncodeString(name, str);
        *str += "=\"";
        TiXmlBase::EncodeString(value, str);
        *str += "\"";
    } else {
        TiXmlBase::EncodeString(name, str);
        *str += "='";
        TiXmlBase::EncodeString(value, str);
        *str += "'";
    }
}

TiXmlAttributeSet::TiXmlAttributeSet(const allocator_type& alloc) : sentinel{alloc} {
    sentinel.next = &sentinel;
    sentinel.prev = &sentinel;
}

TiXmlAttributeSet::~TiXmlAttributeSet() { Clear(); }

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

const TiXmlAttribute* TiXmlAttributeSet::Find(std::string_view name) const {
    for (const TiXmlAttribute* attribute = sentinel.next; attribute != &sentinel;
         attribute = attribute->next) {
        if (attribute->name == name) return attribute;
    }
    return nullptr;
}

template <typename T>
struct PMRDeleter {
    void operator()(T* item) { alloc.delete_object(item); }
    std::pmr::polymorphic_allocator<> alloc;
};

template <class T, class... Args>
std::unique_ptr<T, PMRDeleter<T>> pmr_make_unique(std::pmr::polymorphic_allocator<> alloc,
                                                  Args&&... args) {
    return std::unique_ptr<T, PMRDeleter<T>>(alloc.new_object<T>(std::forward<Args>(args)...),
                                             PMRDeleter<T>{alloc});
}

const char* TiXmlAttributeSet::Parse(const char* p, TiXmlParsingData* data) {
    p = TiXmlBase::SkipWhiteSpace(p);
    if (!p || !*p) return nullptr;

    TiXmlCursor location;
    if (data) {
        data->Stamp(p);
        location = data->Cursor();
    }

    auto alloc = sentinel.value.get_allocator();
    auto attribute = pmr_make_unique<TiXmlAttribute>(alloc, location);

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
