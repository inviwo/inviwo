/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/core/util/document.h>
#include <inviwo/core/io/serialization/serialization.h>
#include <algorithm>

namespace inviwo {

std::unordered_map<std::string, std::string>& Document::Element::attributes() {
    return attributes_;
}

const std::unordered_map<std::string, std::string>& Document::Element::attributes() const {
    return attributes_;
}

std::string& Document::Element::content() { return data_; }

const std::string& Document::Element::content() const { return data_; }

bool Document::Element::emptyTag() const {
    return std::find(emptyTags_.begin(), emptyTags_.end(), data_) != emptyTags_.end();
}

bool Document::Element::noIndent() const {
    return std::find(noIndentTags_.begin(), noIndentTags_.end(), data_) != noIndentTags_.end();
}

const std::vector<std::string> Document::Element::noIndentTags_ = {"pre"};

const std::vector<std::string> Document::Element::emptyTags_ = {
    "area",   "base", "br",   "col",   "embed",  "hr",    "img", "input",
    "keygen", "link", "meta", "param", "source", "track", "wbr"};

Document::ElementType Document::Element::type() const { return type_; }

bool Document::Element::isText() const { return type_ == ElementType::Text; }

bool Document::Element::isNode() const { return type_ == ElementType::Node; }

Document::Element::Element(const Element& rhs)
    : type_{rhs.type_}, children_{}, data_{rhs.data_}, attributes_{rhs.attributes_} {
    for (const auto& child : rhs.children_) {
        children_.push_back(std::make_unique<Element>(*child));
    }
}

Document::Element& Document::Element::operator=(const Element& that) {
    if (this != &that) {
        Element tmp(that);
        swap(*this, tmp);
    }
    return *this;
}

Document::Element::Element(ElementType type, std::string_view content)
    : type_{type}, data_{content} {}
Document::Element::Element(std::string_view name, std::string_view content,
                           const std::unordered_map<std::string, std::string>& attributes)
    : type_{ElementType::Node}, data_{name}, attributes_{attributes} {
    if (!content.empty()) {
        children_.push_back(std::make_unique<Element>(ElementType::Text, content));
    }
}

std::string& Document::Element::name() { return data_; }

const std::string& Document::Element::name() const { return data_; }

void Document::Element::serialize(Serializer& s) const {
    s.serialize("type", type_, SerializationTarget::Attribute);
    s.serialize("data", data_, SerializationTarget::Attribute);
    s.serialize("children", children_);
    s.serialize("attributes", attributes_);
}
void Document::Element::deserialize(Deserializer& d) {
    d.deserialize("type", type_, SerializationTarget::Attribute);
    d.deserialize("data", data_, SerializationTarget::Attribute);
    d.deserialize("children", children_);
    d.deserialize("attributes", attributes_);
}

Document::PathComponent Document::PathComponent::first() {
    return PathComponent("<first>", [](const ElemVec& elements) -> ElemVec::const_iterator {
        if (!elements.empty())
            return elements.begin();
        else
            return elements.end();
    });
}

Document::PathComponent::PathComponent(
    std::string_view strrep, std::function<ElemVec::const_iterator(const ElemVec&)> matcher)
    : strrep_(strrep), matcher_(matcher) {}

Document::PathComponent::PathComponent(int index)
    : strrep_("Index: " + std::to_string(index))
    , matcher_{[index](const ElemVec& elements) -> ElemVec::const_iterator {
        size_t i = index < 0 ? elements.size() + index : static_cast<size_t>(index);
        if (i < elements.size())
            return elements.begin() + i;
        else
            return elements.end();
    }} {}

Document::PathComponent::PathComponent(std::string_view name)
    : strrep_(name)
    , matcher_{[name = std::string(name)](const ElemVec& elements) -> ElemVec::const_iterator {
        return std::find_if(elements.begin(), elements.end(),
                            [&](const auto& e) { return e->isNode() && e->name() == name; });
    }} {}

Document::PathComponent::PathComponent(
    const std::unordered_map<std::string, std::string>& attributes)
    : strrep_("attr"), matcher_{[attributes](const ElemVec& elements) -> ElemVec::const_iterator {
        return std::find_if(elements.begin(), elements.end(), [&](const auto& e) {
            if (e->isText()) return false;
            for (auto& attr : attributes) {
                auto it = e->attributes().find(attr.first);
                if (it == e->attributes().end()) return false;
                if (it->second != attr.second) return false;
            }
            return true;
        });
    }} {}

Document::PathComponent::PathComponent(
    std::string_view name, const std::unordered_map<std::string, std::string>& attributes)
    : strrep_("attr")
    , matcher_{[name = std::string(name),
                attributes](const ElemVec& elements) -> ElemVec::const_iterator {
        return std::find_if(elements.begin(), elements.end(), [&](const auto& e) {
            if (e->isText()) return false;
            if (e->name() != name) return false;
            for (auto& attr : attributes) {
                auto it = e->attributes().find(attr.first);
                if (it == e->attributes().end()) return false;
                if (it->second != attr.second) return false;
            }
            return true;
        });
    }} {}

Document::PathComponent Document::PathComponent::last() {
    return PathComponent("<last>", [](const ElemVec& elements) -> ElemVec::const_iterator {
        if (!elements.empty())
            return --elements.end();
        else
            return elements.end();
    });
}

Document::PathComponent Document::PathComponent::end() {
    return PathComponent(
        "<end>", [](const ElemVec& elements) -> ElemVec::const_iterator { return elements.end(); });
}

Document::ElemVec::const_iterator Document::PathComponent::operator()(
    const ElemVec& elements) const {
    return matcher_(elements);
}

Document::DocumentHandle::DocumentHandle(const Document* doc, Element* elem)
    : doc_(doc), elem_(elem) {}

Document::DocumentHandle Document::DocumentHandle::get(const std::vector<PathComponent>& path) {
    Element* current = elem_;
    for (const auto& pc : path) {
        auto it = pc(current->children_);
        if (it != current->children_.end()) {
            current = (*it).get();
        } else {
            return DocumentHandle(doc_, nullptr);
        }
    }
    return DocumentHandle(doc_, current);
}

Document::DocumentHandle Document::DocumentHandle::insert(
    PathComponent pos, std::string_view name, std::string_view content,
    const std::unordered_map<std::string, std::string>& attributes) {
    auto iter = pos(elem_->children_);

    auto it = elem_->children_.insert(iter, std::make_unique<Element>(name, content, attributes));

    return DocumentHandle(doc_, it->get());
}

Document::DocumentHandle Document::DocumentHandle::append(
    std::string_view name, std::string_view content,
    const std::unordered_map<std::string, std::string>& attributes) {
    return insert(PathComponent::end(), name, content, attributes);
}

Document::DocumentHandle Document::DocumentHandle::insertText(PathComponent pos,
                                                              std::string_view text) {
    auto iter = pos(elem_->children_);
    auto it = elem_->children_.insert(iter, std::make_unique<Element>(ElementType::Text, text));
    return DocumentHandle(doc_, it->get());
}
Document::DocumentHandle Document::DocumentHandle::appendText(std::string_view text) {
    return insertText(PathComponent::end(), text);
}

Document::DocumentHandle Document::DocumentHandle::insert(PathComponent pos, Document doc) {
    auto iter = pos(elem_->children_);

    if (doc.empty()) {
        return {doc_, nullptr};
    }

    auto it = elem_->children_.insert(iter, std::move_iterator(doc.root_->children_.begin()),
                                      std::move_iterator(doc.root_->children_.end()));

    return DocumentHandle(doc_, it->get());
}
Document::DocumentHandle Document::DocumentHandle::append(Document doc) {
    return insert(PathComponent::end(), std::move(doc));
}

Document::Element& Document::DocumentHandle::element() { return *elem_; }

const Document::Element& Document::DocumentHandle::element() const { return *elem_; }

Document::DocumentHandle::operator bool() const { return elem_ != nullptr; }

Document::DocumentHandle& Document::DocumentHandle::operator+=(std::string_view content) {
    if (elem_->isText()) {
        elem_->content() += content;
    } else if (!elem_->children_.empty() && elem_->children_.back()->isText()) {
        elem_->children_.back()->content() += content;
    } else {
        elem_->children_.push_back(std::make_unique<Element>(ElementType::Text, content));
    }

    return *this;
}

Document::Document() : root_{std::make_unique<Element>("root")} {}

Document::Document(std::string_view text) : root_{std::make_unique<Element>("root")} {
    root_->children_.push_back(std::make_unique<Element>(ElementType::Text, text));
}

Document::Document(const Document& rhs) : root_{std::make_unique<Element>(*rhs.root_)} {}

Document& Document::operator=(const Document& that) {
    if (this != &that) {
        Document tmp(that);
        std::swap(*this, tmp);
    }
    return *this;
}

Document::DocumentHandle Document::handle() const { return DocumentHandle(this, root_.get()); }

Document::DocumentHandle Document::get(const std::vector<PathComponent>& path) {
    return handle().get(path);
}

Document::DocumentHandle Document::insert(
    PathComponent pos, std::string_view name, std::string_view content,
    const std::unordered_map<std::string, std::string>& attributes) {
    return handle().insert(pos, name, content, attributes);
}

Document::DocumentHandle Document::append(
    std::string_view name, std::string_view content,
    const std::unordered_map<std::string, std::string>& attributes) {
    return handle().append(name, content, attributes);
}

Document::DocumentHandle Document::insert(PathComponent pos, Document doc) {
    return handle().insert(pos, std::move(doc));
}

Document::DocumentHandle Document::insertText(PathComponent pos, std::string_view text) {
    return handle().insertText(pos, text);
}
Document::DocumentHandle Document::appendText(std::string_view text) {
    return handle().appendText(text);
}

Document::DocumentHandle Document::append(Document doc) { return handle().append(std::move(doc)); }

std::string Document::str() const {
    std::stringstream ss;
    ss << *this;
    return std::move(ss).str();
}

Document::operator std::string() const { return str(); }

void Document::serialize(Serializer& s) const { s.serialize("root", root_); }
void Document::deserialize(Deserializer& d) { d.deserialize("root", root_); }

std::ostream& operator<<(std::ostream& ss, const Document& doc) {
    using Element = Document::Element;
    doc.visit(
        [&](Element* elem, std::vector<Element*>& stack) {
            if (elem->isNode()) {
                ss << std::setw(stack.size() * 4) << ' ' << '<' << elem->name();
                for (const auto& item : elem->attributes()) {
                    ss << ' ' << item.first << "='" << item.second << '\'';
                }
                ss << '>';
                if (!elem->noIndent()) ss << '\n';
            } else if (elem->isText() && !elem->content().empty()) {
                if (!stack.empty() && !stack.back()->noIndent()) {
                    ss << std::setw(stack.size() * 4) << ' ' << elem->content() << '\n';
                } else if (!stack.empty() && stack.back()->noIndent()) {
                    ss << elem->content();
                } else {
                    ss << elem->content() << '\n';
                }
            }
        },
        [&](Element* elem, std::vector<Element*>& stack) {
            if (elem->isNode() && !elem->emptyTag()) {
                if (!elem->noIndent()) {
                    ss << std::setw(stack.size() * 4) << ' ' << "</" << elem->name() << ">\n";
                } else {
                    ss << "</" << elem->name() << ">\n";
                }
            }
        });

    return ss;
}

utildoc::TableBuilder::TableBuilder(Document::DocumentHandle handle, Document::PathComponent pos,
                                    const std::unordered_map<std::string, std::string>& attributes)
    : table_(handle.insert(pos, "table", "", attributes)) {}

utildoc::TableBuilder::TableBuilder(Document::DocumentHandle table) : table_(table) {}

void utildoc::TableBuilder::tabledata(Document::DocumentHandle& row, const ArrributeWrapper& val) {
    row.insert(Document::PathComponent::end(), "td", val.data_, val.attributes_);
}

void utildoc::TableBuilder::tabledata(Document::DocumentHandle& row, const Header& val) {
    row.insert(Document::PathComponent::end(), "th", val.data_, {{"align", "left"}});
}

void utildoc::TableBuilder::tabledata(Document::DocumentHandle& row, const char* const val) {
    row.insert(Document::PathComponent::end(), "td", std::string(val));
}

void utildoc::TableBuilder::tabledata(Document::DocumentHandle& row, Span_t) {
    auto l = row.get({Document::PathComponent::last()});
    if (l) {
        auto it = l.element().attributes().find("colspan");
        if (it != l.element().attributes().end()) {
            std::stringstream ss;
            ss << it->second;
            int count{0};
            ss >> count;
            l.element().attributes()["colspan"] = std::to_string(count + 1);
        } else {
            l.element().attributes()["colspan"] = std::to_string(1);
        }
    }
}

void utildoc::TableBuilder::tabledata(Document::DocumentHandle& row, const std::string& val) {
    row.insert(Document::PathComponent::end(), "td", val);
}

utildoc::TableBuilder::Wrapper::Wrapper(const char* const data) : data_(data) {}

utildoc::TableBuilder::Wrapper::Wrapper(std::string_view data) : data_{data} {}

utildoc::TableBuilder::Wrapper::Wrapper(const std::string& data) : data_(data) {}

}  // namespace inviwo
