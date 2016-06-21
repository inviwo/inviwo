/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

Document::Document() : root_{util::make_unique<Element>("root", "root")} {
    elements_[root_.get()] = std::vector<std::unique_ptr<Element>>();
}

Document::Document(const Document& rhs) : root_{util::make_unique<Element>(*rhs.root_)} {
    elements_[root_.get()] = std::vector<std::unique_ptr<Element>>();
    std::vector<Element*> parent = {root_.get()};
    rhs.visitElements(
        [&](Element* elem, std::vector<Element*>& stack) {
            auto clone = util::make_unique<Element>(*elem);
            auto c = clone.get();
            elements_[parent.back()].push_back(std::move(clone));
            parent.push_back(c);
        },
        [&](Element* elem, std::vector<Element*>& stack) { parent.pop_back(); });
}
Document::Document(Document&& rhs)
    : root_{std::move(rhs.root_)}, elements_{std::move(rhs.elements_)} {}

Document& Document::operator=(Document that) {
    std::swap(root_, that.root_);
    std::swap(elements_, that.elements_);
    return *this;
}

Document::operator std::string() const {
    std::stringstream ss;

    visitElements(
        [&](Element* elem, std::vector<Element*>& stack) {
            ss << std::string(stack.size() * 4, ' ') << "<" << elem->type() << " id='"
               << elem->name() << "'";
            for (const auto& item : elem->attributes()) {
                ss << " " << item.first << "='" << item.second << "'";
            }
            ss << ">\n";
            if (!elem->contents().empty())
                ss << std::string((1+stack.size()) * 4, ' ') << elem->contents() << "\n";
        },
        [&](Element* elem, std::vector<Element*>& stack) {
            ss << std::string(stack.size() * 4, ' ') << "</" << elem->type() << ">\n";
        });

    return ss.str();
}

Document::ElementWrapper Document::getElement(const std::vector<Path>& paths) {
    return ElementWrapper(this, getElement(paths.begin(), paths.end()), paths);
}

Document::Element* Document::getElement(std::vector<Path>::const_iterator b,
                                        std::vector<Path>::const_iterator e) const {
    Element* elem = root_.get();
    for (auto path = b; path != e; ++path) {
        auto it = elements_.find(elem);
        if (it != elements_.end()) {
            auto eit = (*path)(it->second);
            if (eit != it->second.end()) elem = eit->get();
            else return nullptr;
        } else {
            return nullptr;
        }
    }
    return elem;
}

size_t Document::numberOfChildren(const std::vector<Path>& path) const {
    if(auto elem = getElement(path.begin(), path.end())) {
        auto it = elements_.find(elem);
        if (it != elements_.end()) return it->second.size();
    }
    return 0;
}

Document::ElementWrapper Document::addElementIn(const std::vector<Path>& path,
                                                const std::string& type, const std::string& name) {
    if (auto elem = getElement(path.begin(), path.end())) {
        auto e = util::make_unique<Element>(type, name);
        auto res = e.get();
        elements_[elem].push_back(std::move(e));
        auto newpath = path;
        newpath.push_back(Path(name));
        return ElementWrapper(this, res, newpath);
    }
    throw Exception("Invalid Path: " + joinString(path, "/"));
}

Document::ElementWrapper Document::addElementAfter(const std::vector<Path>& path, const std::string& type,
                      const std::string& name) {
    
    if (path.empty()) throw Exception("Empty path");
    
    if (auto elem = path.size() > 1 ? getElement(path.begin(), --path.end()) : root_.get()) {
        auto& vec = elements_[elem];
        auto it = (path.back())(vec);
        if (it != vec.end()) {
            auto e = util::make_unique<Element>(type, name);
            auto res = e.get();
            vec.insert(++it, std::move(e));
            auto newpath = path;
            newpath.pop_back();
            newpath.push_back(Path(name));
            return ElementWrapper(this, res, newpath);
        }
    }
    throw Exception("Invalid Path: " + joinString(path, "/"));
}

Document::ElementWrapper Document::addElementBefore(const std::vector<Path>& path, const std::string& type,
                      const std::string& name) {
    if (path.empty()) throw Exception("Empty path");
    
    if (auto elem = path.size() > 1 ? getElement(path.begin(), --path.end()) : root_.get()) {
        auto& vec = elements_[elem];
        auto it = (path.back())(vec);
        if (it != vec.end()) {
            auto e = util::make_unique<Element>(type, name);
            auto res = e.get();
            vec.insert(it, std::move(e));
            auto newpath = path;
            newpath.pop_back();
            newpath.push_back(Path(name));
            return ElementWrapper(this, res, newpath);
        }
    }
    throw Exception("Invalid Path: " + joinString(path, "/"));
}

Document::Element::Element(const std::string& type, const std::string& name)
    : type_(type), name_(name) {}

Document::Element& Document::Element::setName(const std::string& name) {
    name_ = name;
    return *this;
}
Document::Element& Document::Element::setType(const std::string& type) {
    type_ = type;
    return *this;
}
Document::Element& Document::Element::setContent(const std::string& contents) {
    contents_ = contents;
    return *this;
}
Document::Element& Document::Element::addAttribute(const std::string& key,
                                                   const std::string& value) {
    attributes_[key] = value;
    return *this;
}

const std::string& Document::Element::name() const { return name_; }
const std::string& Document::Element::type() const { return type_; }
const std::string& Document::Element::contents() const { return contents_; }
std::unordered_map<std::string, std::string>& Document::Element::attributes() {
    return attributes_;
}

Document::Path::Path(const std::string strrep, std::function<C::const_iterator(const C&)> func)
    : strrep_(strrep), func_(func) {}

Document::Path::Path(const std::string& name)
    : strrep_(name), func_{[name](const C& elements) -> C::const_iterator {
        return util::find_if(elements,
                             [&](const std::unique_ptr<Element>& e) { return e->name() == name; });
    }} {}

Document::Path::Path(int index)
    : strrep_(toString(index)), func_{[index](const C& elements) -> C::const_iterator {
        size_t i = index < 0 ? elements.size() + index : static_cast<size_t>(index);
        if (i < elements.size())
            return elements.begin() + i;
        else
            return elements.end();
    }} {}

Document::Path Document::Path::first() {
    return Path("<first>", [](const C& elements) -> C::const_iterator {
        if (!elements.empty())
            return elements.begin();
        else
            return elements.end();
    });
}
Document::Path Document::Path::last() {
    return Path("<last>", [](const C& elements) -> C::const_iterator {
        if (!elements.empty())
            return --elements.end();
        else
            return elements.end();
    });
}

Document::Path::C::const_iterator Document::Path::operator() (const C& elements) const {
    return func_(elements);
}
Document::Path::operator const std::string&() const {
    return strrep_;
}


Document::ElementWrapper::ElementWrapper(Document* doc, Element* element,
                                         const std::vector<Path>& path)
    : doc_{doc}, element_(element), path_(path) {}

Document::Element& Document::ElementWrapper::element() { return *element_; }
const std::vector<Document::Path>& Document::ElementWrapper::path() { return path_; }


Document::ElementWrapper Document::ElementWrapper::getElement(const std::vector<Path>& path){
    auto p = path_;
    std::copy(path.begin(),path.end(), std::back_inserter(p));
    return doc_->getElement(p);
}

Document::ElementWrapper Document::ElementWrapper::addElementIn(const std::string& type,
                                                                const std::string& name) {
    return doc_->addElementIn(path_, type, name);
}
Document::ElementWrapper Document::ElementWrapper::addElementAfter(const std::string& type,
                                                                   const std::string& name) {
    return doc_->addElementAfter(path_, type, name);
}
Document::ElementWrapper Document::ElementWrapper::addElementBefore(const std::string& type,
                                                                    const std::string& name) {
    return doc_->addElementBefore(path_, type, name);
}

Document::ElementWrapper& Document::ElementWrapper::setName(const std::string& name) {
    element_->setName(name);
    return *this;
}
Document::ElementWrapper& Document::ElementWrapper::setType(const std::string& type) {
    element_->setType(type);
    return *this;
}
Document::ElementWrapper& Document::ElementWrapper::setContent(const std::string& contents) {
    element_->setContent(contents);
    return *this;
}
Document::ElementWrapper& Document::ElementWrapper::addAttribute(const std::string& key,
                                                                 const std::string& value) {
    element_->addAttribute(key, value);
    return *this;
}

size_t Document::ElementWrapper::numberOfChildren() const {
    return doc_->numberOfChildren(path_);
}

std::ostream& operator<<(std::ostream &out, const Document::Path& path) {
    out << path.strrep_;
    return out;
}
std::ostream& operator<<(std::ostream &out, const Document& doc) {
    out << doc.operator std::string();
    return out;
}

}  // namespace
