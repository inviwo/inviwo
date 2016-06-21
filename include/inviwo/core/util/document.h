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

#ifndef IVW_DOCUMENT_H
#define IVW_DOCUMENT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>

#include <inviwo/core/properties/valuewrapper.h>
#include <inviwo/core/util/introspection.h>

#include <unordered_map>

namespace inviwo {

/**
 * \class Document
 * \brief A helper class to represent a simple document
 */
class IVW_CORE_API Document {
public:
    class Element {
    public:
        Element(const std::string& type, const std::string& name);
        Element(const Element&) = default;
        Element& operator=(const Element&) = default;
        
        Element& setName(const std::string& name);
        Element& setType(const std::string& type);
        Element& setContent(const std::string& contents);
        Element& addAttribute(const std::string& key, const std::string& value);
        
        const std::string& name() const;
        const std::string& type() const;
        const std::string& contents() const;
        std::unordered_map<std::string, std::string>& attributes();
        
    private:
        std::string type_;
        std::string name_;
        std::unordered_map<std::string, std::string> attributes_;
        std::string contents_;
    };
    
    class Path {
    public:
        using C = std::vector<std::unique_ptr<Element>>;
        Path(const std::string strrep, std::function<C::const_iterator(const C&)> func);
        Path(const std::string&);
        Path(int index);
        static Path first();
        static Path last();
        C::const_iterator operator()(const C& elements) const;
        operator const std::string&() const;
        friend std::ostream& operator<<(std::ostream &out, const Path& path);
    private:
        std::string strrep_;
        std::function<C::const_iterator(const C&)> func_;
    };

    class ElementWrapper {
    public:
        ElementWrapper(Document* doc, Element* element, const std::vector<Path>& path);

        Element& element();
        const std::vector<Path>& path();
        
        ElementWrapper getElement(const std::vector<Path>& path);

        ElementWrapper addElementIn(const std::string& type, const std::string& name);
        ElementWrapper addElementAfter(const std::string& type, const std::string& name);
        ElementWrapper addElementBefore(const std::string& type, const std::string& name);

        ElementWrapper& setName(const std::string& name);
        ElementWrapper& setType(const std::string& type);
        ElementWrapper& setContent(const std::string& contents);
        ElementWrapper& addAttribute(const std::string& key, const std::string& value);

        size_t numberOfChildren() const;

    private:
        Document* doc_;
        Element* element_;
        std::vector<Path> path_;
    };

    Document();
    Document(const Document& rhs);
    Document(Document&& rhs);
    Document& operator=(Document rhs);
    
    virtual ~Document() = default;

    ElementWrapper getElement(const std::vector<Path>& path);
    ElementWrapper addElementIn(const std::vector<Path>& path, const std::string& type,
                const std::string& name);
    ElementWrapper addElementAfter(const std::vector<Path>& path, const std::string& type,
                     const std::string& name);
    ElementWrapper addElementBefore(const std::vector<Path>& path, const std::string& type,
                      const std::string& name);
    
    size_t numberOfChildren(const std::vector<Path>& path) const;

    operator std::string() const;
    friend std::ostream& operator<<(std::ostream &out, const Document& doc);

private:
    Element* getElement(std::vector<Path>::const_iterator b,
                        std::vector<Path>::const_iterator e) const;

    template <typename BeforVisitor, typename AfterVisitor>
    void visitElements(BeforVisitor before, AfterVisitor after) const {
        const std::function<void(Element*, std::vector<Element*>&)> traverser = [&](
            Element* elem, std::vector<Element*>& stack) {
            before(elem, stack);
            stack.push_back(elem);
            auto it = elements_.find(elem);
            if (it != elements_.end()) {
                for (const auto& e : it->second) traverser(e.get(), stack);
            }
            stack.pop_back();
            after(elem, stack);
        };
        auto it = elements_.find(root_.get());
        if (it != elements_.end()) {
            std::vector<Element*> stack;
            for (const auto& e : it->second) traverser(e.get(), stack);
        }
    }

    std::unique_ptr<Element> root_;
    std::unordered_map<Element*, std::vector<std::unique_ptr<Element>>> elements_;
};

IVW_CORE_API std::ostream& operator<<(std::ostream &out, const Document::Path& path);
IVW_CORE_API std::ostream& operator<<(std::ostream &out, const Document& doc);



namespace utildoc {

namespace detail {

template <typename T,
          typename std::enable_if<util::is_stream_insertable<T>::value, std::size_t>::type = 0>
std::string convert(T&& val) {
    std::stringstream value;
    value << std::boolalpha << std::forward<T>(val);
    return value.str();
}
template <typename T,
          typename std::enable_if<!util::is_stream_insertable<T>::value, std::size_t>::type = 0>
std::string convert(T&& val) {
    return "???";
}

}


struct TableBuilder {
    struct ColSpan {};
    struct Header {};
    struct Text {};

    struct Append {};

    TableBuilder(Document::ElementWrapper r, const std::string& id)
        : t_{r.addElementIn("table", id)
                 .addAttribute("border", "0")
                 .addAttribute("cellspacing", "0")
                 .addAttribute("cellpadding", "0")} {}

    TableBuilder(Append, Document::ElementWrapper t, const std::string& id)
        : t_{t} { }

    void operator()(const std::string& id, const std::string& val) {
        auto tr = t_.addElementIn("tr", "row" + toString(t_.numberOfChildren()));
        State s;
        buildrow(tr, s, Header{}, id, Text{}, val);
    }

    template <typename... Args>
    void operator()(Args&&... args) {
        auto tr = t_.addElementIn("tr", "row" + toString(t_.numberOfChildren()));
        State s;
        buildrow(tr, s, std::forward<Args>(args)...);
    }

    template <typename T>
    void operator()(const ValueWrapper<T>& v) {
        std::string name{v.name};
        name[0] = std::toupper(name[0]);
        auto tr = t_.addElementIn("tr", "row" + toString(t_.numberOfChildren()));
        State s;
        buildrow(tr, s, Header{}, name, Text{}, v.value);
    }

private:
    enum class Format { Header, Text };
    struct State {
        Format format = Format::Text;
        size_t col = 0;
    };

    template <typename T>
    void processItem(Document::ElementWrapper& w, State& s, T&& val) {
        auto td = w.addElementIn("td", "col" + toString(s.col))
                      .setContent(detail::convert(std::forward<T>(val)));
        switch (s.format) {
            case Format::Header:
                td.addAttribute("style", "color:#bbb;padding-right:8px;");
                break;
            case Format::Text:
                td.addAttribute("style", "padding-right:8px;");
                break;
        }
        s.col++;
    }
    void processItem(Document::ElementWrapper& w, State& s, ColSpan val) {
        auto l = w.getElement({Document::Path::last()});
        auto it = l.element().attributes().find("colspan");
        if(it != l.element().attributes().end()) {
            std::stringstream ss;
            ss << it->second;
            int count{0};
            ss >> count;
            l.addAttribute("colspan", toString(count + 1));
        } else {
            l.addAttribute("colspan", "1");
        }
    }
    
    void processItem(Document::ElementWrapper& w,  State& s, Header val) {
        s.format = Format::Header;

    }
    void processItem(Document::ElementWrapper& w,  State& s, Text val) {
        s.format = Format::Text;
    }

    template <typename T, typename... Args>
    void buildrow(Document::ElementWrapper& w, State& s, T&& val, Args&&... args) {
        processItem(w, s, val);
        buildrow(w, s, std::forward<Args>(args)...);
    }

    template <typename T>
    void buildrow(Document::ElementWrapper& w,  State& s, T&& val) {
        processItem(w, s, val);
    }
    
    Document::ElementWrapper t_;
};

}

}  // namespace

#endif  // IVW_DOCUMENT_H
