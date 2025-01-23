/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/transparentmaps.h>

#include <functional>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

namespace inviwo {

class Serializer;
class Deserializer;

/**
 * @brief A class to represent a structured document, usually some html
 */
class IVW_CORE_API Document {
public:
    class DocumentHandle;

    enum class ElementType { Node, Text };

    class IVW_CORE_API Element {
    public:
        friend Document;
        friend DocumentHandle;

        Element(const Element&);
        Element& operator=(const Element&);
        Element(Element&&) noexcept = default;
        Element& operator=(Element&&) = default;

        Element(ElementType type, std::string_view content);
        Element(std::string_view name, std::string_view content = "",
                const UnorderedStringMap<std::string>& attributes = {});

        const std::string& name() const;
        std::string& name();

        const UnorderedStringMap<std::string>& attributes() const;
        UnorderedStringMap<std::string>& attributes();

        const std::string& content() const;
        std::string& content();

        ElementType type() const;
        bool isText() const;
        bool isNode() const;

        bool emptyTag() const;
        bool noIndent() const;

        friend void swap(Element& lhs, Element& rhs) {
            std::swap(lhs.type_, rhs.type_);
            std::swap(lhs.children_, rhs.children_);
            std::swap(lhs.data_, rhs.data_);
            std::swap(lhs.attributes_, rhs.attributes_);
        }

        void serialize(Serializer& s) const;
        void deserialize(Deserializer& d);

    private:
        ElementType type_;
        std::vector<std::unique_ptr<Element>> children_;
        std::string data_;
        UnorderedStringMap<std::string> attributes_;
        static const std::vector<std::string> emptyTags_;
        static const std::vector<std::string> noIndentTags_;
    };

    using ElemVec = std::vector<std::unique_ptr<Element>>;

    class IVW_CORE_API PathComponent {
    public:
        PathComponent(std::string_view strrep,
                      std::function<ElemVec::const_iterator(const ElemVec&)> matcher);

        explicit PathComponent(int index);
        explicit PathComponent(std::string_view name);
        explicit PathComponent(const UnorderedStringMap<std::string>& attributes);

        PathComponent(std::string_view name, const UnorderedStringMap<std::string>& attributes);

        static PathComponent first();

        static PathComponent last();

        static PathComponent end();

        ElemVec::const_iterator operator()(const ElemVec& elements) const;

        IVW_CORE_API friend std::ostream& operator<<(std::ostream& ss,
                                                     const Document::PathComponent& path);

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
        DocumentHandle insert(PathComponent pos, std::string_view name,
                              std::string_view content = "",
                              const UnorderedStringMap<std::string>& attributes = {});

        DocumentHandle append(std::string_view name, std::string_view content = "",
                              const UnorderedStringMap<std::string>& attributes = {});
        DocumentHandle insert(PathComponent pos, Document doc);

        DocumentHandle insertText(PathComponent pos, std::string_view text);
        DocumentHandle appendText(std::string_view text);

        DocumentHandle append(Document doc);

        const Element& element() const;
        Element& element();

        operator bool() const;
        DocumentHandle& operator+=(std::string_view content);

    private:
        DocumentHandle(const Document* doc, Element* elem);

        const Document* doc_;
        Element* elem_;
    };

    Document();
    Document(std::string_view text);
    Document(const Document&);
    Document& operator=(const Document&);
    Document(Document&&) = default;
    Document& operator=(Document&&) = default;
    virtual ~Document() = default;

    bool empty() const { return root_->children_.empty(); };

    DocumentHandle handle() const;

    DocumentHandle get(const std::vector<PathComponent>& path);
    DocumentHandle insert(PathComponent pos, std::string_view name, std::string_view content = "",
                          const UnorderedStringMap<std::string>& attributes = {});

    DocumentHandle append(std::string_view name, std::string_view content = "",
                          const UnorderedStringMap<std::string>& attributes = {});

    DocumentHandle insertText(PathComponent pos, std::string_view text);
    DocumentHandle appendText(std::string_view text);

    DocumentHandle insert(PathComponent pos, Document doc);
    DocumentHandle append(Document doc);

    template <typename BeforVisitor, typename AfterVisitor>
    void visit(BeforVisitor before, AfterVisitor after) const {
        const auto traverser = [&](auto& self, Element* elem,
                                   std::vector<Element*>& stack) -> void {
            before(elem, stack);
            stack.push_back(elem);

            for (const auto& child : elem->children_) {
                self(self, child.get(), stack);
            }

            stack.pop_back();
            after(elem, stack);
        };

        std::vector<Element*> stack;
        for (const auto& e : root_->children_) {
            traverser(traverser, e.get(), stack);
        }
    }

    IVW_CORE_API friend std::ostream& operator<<(std::ostream& ss, const Document& doc);

    std::string str() const;
    operator std::string() const;

    friend void swap(Document& lhs, Document& rhs) { std::swap(lhs.root_, rhs.root_); }

    void serialize(Serializer& s) const;
    void deserialize(Deserializer& d);

private:
    std::unique_ptr<Element> root_;
};

}  // namespace inviwo
