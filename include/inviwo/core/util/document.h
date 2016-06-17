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

#include <unordered_map>

namespace inviwo {

/**
 * \class Document
 * \brief A helper class to represent a simple document
 */
class IVW_CORE_API Document {
    struct Element {
        Element(std::string t, std::string n) : type(t), name(n) {}
        std::string type;
        std::string name;
        std::unordered_map<std::string, std::string> attributes;
        std::string contents;
    };

public:
    struct Path {
        Path(std::function<Element*(const std::vector<std::unique_ptr<Element>>&)> func);
        Path(const std::string&);
        Path(int index);
        static Path first();
        static Path last();
        Element* operator()(const std::vector<std::unique_ptr<Element>>& elements) const;

    private:
        std::function<Element*(const std::vector<std::unique_ptr<Element>>&)> func_;
    };

    Document();
    virtual ~Document() = default;

    void insert(const std::vector<Path>& path, const std::string& type,
                const std::string& name);

    operator std::string() const;

private:
    Element* findElement(const std::vector<Path>& path) const;

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

}  // namespace

#endif  // IVW_DOCUMENT_H
