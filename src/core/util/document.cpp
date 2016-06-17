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

Document::operator std::string() const {
    std::stringstream ss;

    visitElements(
        [&](Element* elem, std::vector<Element*>& stack) {
            ss << std::string(stack.size() * 4, ' ') << "<" << elem->type << " id='" << elem->name
               << "'>\n";
        },
        [&](Element* elem, std::vector<Element*>& stack) {
            ss << std::string(stack.size() * 4, ' ') << "</" << elem->type << ">\n";
        });

    return ss.str();
}

Document::Element* Document::findElement(const std::vector<Path>& paths) const {
    Element* elem = root_.get();
    for (const auto& path : paths) {
        auto it = elements_.find(elem);
        if (it != elements_.end()) {
            elem = path(it->second);
            if (!elem) return nullptr;
        } else {
            return nullptr;
        }
    }
    return elem;
}

void Document::insert(const std::vector<Path>& path, const std::string& type,
                      const std::string& name) {
    if (auto elem = findElement(path)) {
        elements_[elem].push_back(util::make_unique<Element>(type, name));
    }
}

Document::Path::Path(std::function<Element*(const std::vector<std::unique_ptr<Element>>&)> func)
    : func_(func) {}

Document::Path::Path(const std::string& name)
    : func_{[name](const std::vector<std::unique_ptr<Element>>& elements) -> Element* {
        auto it = util::find_if(elements,
                                [&](const std::unique_ptr<Element>& e) { return e->name == name; });
        if (it != elements.end()) {
            return it->get();
        } else {
            return nullptr;
        }
    }} {}

Document::Path::Path(int index)
    : func_{[index](const std::vector<std::unique_ptr<Element>>& elements) -> Element* {
        size_t i = index < 0 ? elements.size() + index : static_cast<size_t>(index);
        if (i < elements.size())
            return elements[i].get();
        else
            return nullptr;
    }} {}

Document::Path Document::Path::first() {
    return Path([](const std::vector<std::unique_ptr<Element>>& elements) -> Element* {
        if (!elements.empty())
            return elements.front().get();
        else
            return nullptr;
    });
}
Document::Path Document::Path::last() {
    return Path([](const std::vector<std::unique_ptr<Element>>& elements) -> Element* {
        if (!elements.empty())
            return elements.back().get();
        else
            return nullptr;
    });
}

Document::Element* Document::Path::operator() (
    const std::vector<std::unique_ptr<Element>>& elements) const {
    return func_(elements);
}

}  // namespace
