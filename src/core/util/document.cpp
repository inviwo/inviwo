/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <sstream>

namespace inviwo {

std::unordered_map<std::string, std::string>& Document::Element::attributes() {
    return attributes_;
}

const std::unordered_map<std::string, std::string>& Document::Element::attributes() const {
    return attributes_;
}

std::string& Document::Element::content() { return data_; }

const std::string& Document::Element::content() const { return data_; }

bool Document::Element::emptyTag() const { return util::contains(emptyTags_, data_); }

bool Document::Element::noIndent() const { return util::contains(noIndentTags_, data_); }

const std::vector<std::string> Document::Element::noIndentTags_ = {"pre"};

const std::vector<std::string> Document::Element::emptyTags_ = {
    "area",   "base", "br",   "col",   "embed",  "hr",    "img", "input",
    "keygen", "link", "meta", "param", "source", "track", "wbr"};

Document::ElementType Document::Element::type() const { return type_; }

bool Document::Element::isText() const { return type_ == ElementType::Text; }

bool Document::Element::isNode() const { return type_ == ElementType::Node; }

Document::Element::Element(ElementType type, const std::string& content)
    : type_{type}, data_{content} {}
Document::Element::Element(const std::string& name, const std::string& content,
                           const std::unordered_map<std::string, std::string>& attributes)
    : type_{ElementType::Node}, data_{name}, attributes_{attributes} {
    if (!content.empty()) {
        children_.push_back(std::make_unique<Element>(ElementType::Text, content));
    }
}

std::string& Document::Element::name() { return data_; }

const std::string& Document::Element::name() const { return data_; }

Document::PathComponent Document::PathComponent::first() {
    return PathComponent("<first>", [](const ElemVec& elements) -> ElemVec::const_iterator {
        if (!elements.empty())
            return elements.begin();
        else
            return elements.end();
    });
}

Document::PathComponent::PathComponent(
    const std::string strrep, std::function<ElemVec::const_iterator(const ElemVec&)> matcher)
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

Document::PathComponent::PathComponent(const std::string name)
    : strrep_(name), matcher_{[name](const ElemVec& elements) -> ElemVec::const_iterator {
        return std::find_if(elements.begin(), elements.end(),
                            [name](const auto& e) { return e->isNode() && e->name() == name; });
    }} {}

Document::PathComponent::PathComponent(
    const std::unordered_map<std::string, std::string>& attributes)
    : strrep_("attr"), matcher_{[attributes](const ElemVec& elements) -> ElemVec::const_iterator {
        return std::find_if(elements.begin(), elements.end(), [attributes](const auto& e) {
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
    const std::string name, const std::unordered_map<std::string, std::string>& attributes)
    : strrep_("attr")
    , matcher_{[name, attributes](const ElemVec& elements) -> ElemVec::const_iterator {
        return std::find_if(elements.begin(), elements.end(), [name, attributes](const auto& e) {
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
    PathComponent pos, const std::string& name, const std::string content,
    const std::unordered_map<std::string, std::string>& attributes) {
    auto iter = pos(elem_->children_);

    auto it = elem_->children_.insert(iter, std::make_unique<Element>(name, content, attributes));

    return DocumentHandle(doc_, it->get());
}

Document::DocumentHandle Document::DocumentHandle::append(
    const std::string& name, const std::string content,
    const std::unordered_map<std::string, std::string>& attributes) {
    return insert(PathComponent::end(), name, content, attributes);
}

Document::DocumentHandle Document::DocumentHandle::insert(PathComponent pos, Document doc) {
    auto iter = pos(elem_->children_);
    auto it = elem_->children_.insert(iter, std::move(doc.root_));
    return DocumentHandle(doc_, it->get());
}
Document::DocumentHandle Document::DocumentHandle::append(Document doc) {
    return insert(PathComponent::end(), std::move(doc));
}

Document::Element& Document::DocumentHandle::element() { return *elem_; }

const Document::Element& Document::DocumentHandle::element() const { return *elem_; }

Document::DocumentHandle::operator bool() const { return elem_ != nullptr; }

Document::DocumentHandle& Document::DocumentHandle::operator+=(const std::string& content) {
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

Document::DocumentHandle Document::handle() const { return DocumentHandle(this, root_.get()); }

Document::DocumentHandle Document::get(const std::vector<PathComponent>& path) {
    return handle().get(path);
}

Document::DocumentHandle Document::insert(
    PathComponent pos, const std::string& name, const std::string content,
    const std::unordered_map<std::string, std::string>& attributes) {
    return handle().insert(pos, name, content, attributes);
}

Document::DocumentHandle Document::append(
    const std::string& name, const std::string content,
    const std::unordered_map<std::string, std::string>& attributes) {
    return handle().append(name, content, attributes);
}

Document::DocumentHandle Document::insert(PathComponent pos, Document doc) {
    return handle().insert(pos, std::move(doc));
}
Document::DocumentHandle Document::append(Document doc) { return handle().append(std::move(doc)); }

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

utildoc::TableBuilder::Wrapper::Wrapper(const std::string& data) : data_(data) {}

}  // namespace inviwo
