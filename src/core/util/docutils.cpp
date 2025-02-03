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

#include <inviwo/core/util/docutils.h>

namespace inviwo::utildoc {

TableBuilder::TableBuilder(Document::DocumentHandle handle, Document::PathComponent pos,
                           const UnorderedStringMap<std::string>& attributes)
    : table_(handle.insert(std::move(pos), "table", "", attributes)) {}

TableBuilder::TableBuilder(Document::DocumentHandle table) : table_(table) {}

void TableBuilder::tabledata(Document::DocumentHandle& row, const ArrributeWrapper& val) {
    row.insert(Document::PathComponent::end(), "td", val.data_, val.attributes_);
}

void TableBuilder::tabledata(Document::DocumentHandle& row, const Header& val) {
    row.insert(Document::PathComponent::end(), "th", val.data_, {{"align", "left"}});
}

void TableBuilder::tabledata(Document::DocumentHandle& row, const char* const val) {
    row.insert(Document::PathComponent::end(), "td", std::string(val));
}

void TableBuilder::tabledata(Document::DocumentHandle& row, Span_t) {
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

void TableBuilder::tabledata(Document::DocumentHandle& row, const std::string& val) {
    row.insert(Document::PathComponent::end(), "td", val);
}

TableBuilder::Wrapper::Wrapper(const char* const data) : data_(data) {}

TableBuilder::Wrapper::Wrapper(std::string_view data) : data_{data} {}

TableBuilder::Wrapper::Wrapper(std::string data) : data_(std::move(data)) {}

}  // namespace inviwo::utildoc
