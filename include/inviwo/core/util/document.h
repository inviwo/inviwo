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

#ifndef IVW_DOCUMENT_H
#define IVW_DOCUMENT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>

#include <inviwo/core/properties/valuewrapper.h>
#include <inviwo/core/util/stdextensions.h>

#include <unordered_map>
#include <memory>
#include <functional>
#include <string>
#include <algorithm>

namespace inviwo {

/**
 * \class Document
 * \brief A helper class to represent a document
 */

class IVW_CORE_API Document {
public:
    class Element;
    class DocumentHandle;

    using ElemVec = std::vector<std::unique_ptr<Element>>;

    enum class ElementType { Node, Text };

    class IVW_CORE_API Element {
    public:
        friend Document;
        friend DocumentHandle;

        Element(const Element&) = delete;
        Element& operator=(const Element&) = delete;
        Element(Element&&) = default;
        Element& operator=(Element&&) = default;

        Element(ElementType type, const std::string& content);
        Element(const std::string& name, const std::string& content = "",
                const std::unordered_map<std::string, std::string>& attributes = {});

        const std::string& name() const;
        const std::unordered_map<std::string, std::string>& attributes() const;
        const std::string& content() const;

        ElementType type() const;
        bool isText() const;
        bool isNode() const;

        std::string& name();
        std::unordered_map<std::string, std::string>& attributes();
        std::string& content();

        bool emptyTag() const;
        bool noIndent() const;

    private:
        ElementType type_;
        std::vector<std::unique_ptr<Element>> children_;
        std::string data_;
        std::unordered_map<std::string, std::string> attributes_;
        static const std::vector<std::string> emptyTags_;
        static const std::vector<std::string> noIndentTags_;
    };

    class IVW_CORE_API PathComponent {
    public:
        PathComponent(const std::string strrep,
                      std::function<ElemVec::const_iterator(const ElemVec&)> matcher);

        PathComponent(int index);
        PathComponent(const std::string name);
        PathComponent(const std::unordered_map<std::string, std::string>& attributes);

        PathComponent(const std::string name,
                      const std::unordered_map<std::string, std::string>& attributes);

        static PathComponent first();

        static PathComponent last();

        static PathComponent end();

        ElemVec::const_iterator operator()(const ElemVec& elements) const;

        template <class Elem, class Traits>
        friend std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                                            const Document::PathComponent& path) {
            ss << path.strrep_;
            return ss;
        }

    private:
        std::string strrep_;
        std::function<ElemVec::const_iterator(const ElemVec&)> matcher_;
    };

    class IVW_CORE_API DocumentHandle {
    public:
        friend Document;

        DocumentHandle(const DocumentHandle&) = default;
        DocumentHandle& operator=(const DocumentHandle&) = default;
        DocumentHandle(DocumentHandle&&) = default;
        DocumentHandle& operator=(DocumentHandle&&) = default;

        DocumentHandle get(const std::vector<PathComponent>& path);
        DocumentHandle insert(PathComponent pos, const std::string& name,
                              const std::string content = "",
                              const std::unordered_map<std::string, std::string>& attributes = {});

        DocumentHandle append(const std::string& name, const std::string content = "",
                              const std::unordered_map<std::string, std::string>& attributes = {});

        DocumentHandle insert(PathComponent pos, Document doc);
        DocumentHandle append(Document doc);

        const Element& element() const;
        Element& element();

        operator bool() const;
        DocumentHandle& operator+=(const std::string& content);

    private:
        DocumentHandle(const Document* doc, Element* elem);

        const Document* doc_;
        Element* elem_;
    };

    Document();
    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;
    Document(Document&&) = default;
    Document& operator=(Document&&) = default;

    DocumentHandle handle() const;

    DocumentHandle get(const std::vector<PathComponent>& path);
    DocumentHandle insert(PathComponent pos, const std::string& name,
                          const std::string content = "",
                          const std::unordered_map<std::string, std::string>& attributes = {});

    DocumentHandle append(const std::string& name, const std::string content = "",
                          const std::unordered_map<std::string, std::string>& attributes = {});

    DocumentHandle insert(PathComponent pos, Document doc);
    DocumentHandle append(Document doc);

    template <typename BeforVisitor, typename AfterVisitor>
    void visit(BeforVisitor before, AfterVisitor after) const {
        const std::function<void(Element*, std::vector<Element*>&)> traverser =
            [&](Element* elem, std::vector<Element*>& stack) {
                before(elem, stack);
                stack.push_back(elem);

                for (const auto& e : elem->children_) traverser(e.get(), stack);

                stack.pop_back();
                after(elem, stack);
            };
        std::vector<Element*> stack;
        for (const auto& e : root_->children_) traverser(e.get(), stack);
    }

    template <class Elem, class Traits>
    friend std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                                        const Document& doc) {
        doc.visit(
            [&](Element* elem, std::vector<Element*>& stack) {
                if (elem->isNode()) {
                    ss << std::string(stack.size() * 4, ' ') << "<" << elem->name();
                    for (const auto& item : elem->attributes()) {
                        ss << " " << item.first << "='" << item.second << "'";
                    }
                    ss << ">\n";
                } else if (elem->isText() && !elem->content().empty()) {
                    if (!stack.empty() && !stack.back()->noIndent()) {
                        ss << std::string((stack.size()) * 4, ' ');
                    }
                    ss << elem->content() << "\n";
                }
            },
            [&](Element* elem, std::vector<Element*>& stack) {
                if (elem->isNode() && !elem->emptyTag()) {
                    ss << std::string(stack.size() * 4, ' ') << "</" << elem->name() << ">\n";
                }
            });

        return ss;
    }

    operator std::string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

private:
    std::unique_ptr<Element> root_;
};

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
std::string convert(T&& /*val*/) {
    return "???";
}
}  // namespace detail

class IVW_CORE_API TableBuilder {
public:
    struct IVW_CORE_API Wrapper {
        std::string data_;

    protected:
        template <typename T>
        Wrapper(T&& data) : data_(detail::convert(std::forward<T>(data))) {}
        Wrapper(const std::string& data);
        Wrapper(const char* const data);
    };

    struct IVW_CORE_API ArrributeWrapper : Wrapper {
        template <typename T>
        ArrributeWrapper(const std::unordered_map<std::string, std::string>& attributes, T&& data)
            : Wrapper(std::forward<T>(data)), attributes_(attributes) {}
        std::unordered_map<std::string, std::string> attributes_;
    };
    struct IVW_CORE_API Header : Wrapper {
        template <typename T>
        Header(T&& data) : Wrapper(std::forward<T>(data)) {}
    };

    struct Span_t {};

    TableBuilder(Document::DocumentHandle handle, Document::PathComponent pos,
                 const std::unordered_map<std::string, std::string>& attributes = {});

    TableBuilder(Document::DocumentHandle table);

    template <typename... Args>
    Document::DocumentHandle operator()(Document::PathComponent pos, Args&&... args) {
        auto row = table_.insert(pos, "tr");
        tablerow(row, std::forward<Args>(args)...);
        return row;
    }

    template <typename... Args>
    Document::DocumentHandle operator()(Args&&... args) {
        return operator()(Document::PathComponent::end(), std::forward<Args>(args)...);
    }

private:
    template <typename T, typename... Args>
    void tablerow(Document::DocumentHandle& w, T&& val, Args&&... args) {
        tabledata(w, std::forward<T>(val));
        tablerow(w, std::forward<Args>(args)...);
    }

    template <typename T>
    void tablerow(Document::DocumentHandle& w, T&& val) {
        tabledata(w, std::forward<T>(val));
    }

    void tabledata(Document::DocumentHandle& row, const std::string& val);
    void tabledata(Document::DocumentHandle& row, const char* const val);

    template <typename T, typename std::enable_if<!std::is_base_of<Wrapper, std::decay_t<T>>::value,
                                                  int>::type = 0>
    void tabledata(Document::DocumentHandle& row, T&& val) {
        row.insert(Document::PathComponent::end(), "td", detail::convert(val));
    }
    void tabledata(Document::DocumentHandle& row, Span_t val);

    void tabledata(Document::DocumentHandle& row, const ArrributeWrapper& val);
    void tabledata(Document::DocumentHandle& row, const Header& val);

    Document::DocumentHandle table_;
};

}  // namespace utildoc

}  // namespace inviwo

#endif  // IVW_DOCUMENT_H
