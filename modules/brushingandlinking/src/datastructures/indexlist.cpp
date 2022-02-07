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

#include <modules/brushingandlinking/datastructures/indexlist.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

bool IndexList::empty() const { return indices_.empty(); }

size_t IndexList::size() const { return indices_.size(); }

void IndexList::clear() {
    indices_.clear();
    indicesBySource_.clear();
    indicesDirty_ = false;
}

const BitSet& IndexList::getIndices() const {
    update();
    return indices_;
}

bool IndexList::set(std::string_view src, const BitSet& indices) {
    const std::string source(src);
    auto it = indicesBySource_.find(source);

    if (it == indicesBySource_.end()) {
        if (indices.empty()) return false;

        indicesBySource_.emplace(std::make_pair(source, indices));
    } else {
        if (it->second == indices) return false;

        if (indices.empty()) {
            indicesBySource_.erase(it);
        } else {
            it->second = indices;
        }
    }
    indicesDirty_ = true;
    return true;
}

bool IndexList::contains(uint32_t idx) const {
    update();
    return indices_.contains(idx);
}

bool IndexList::removeSources(const std::vector<std::string>& sources) {
    size_t modified = 0u;
    for (auto& source : sources) {
        modified += indicesBySource_.erase(source);
    }
    indicesDirty_ |= (modified != 0);
    return (modified != 0);
}

void IndexList::update() const {
    if (!indicesDirty_) return;

    using T = std::unordered_map<std::string, BitSet>::value_type;
    util::map_erase_remove_if(indicesBySource_, [](const T& p) { return p.second.empty(); });

    auto bitsets =
        util::transform(indicesBySource_, [](auto& p) -> const BitSet* { return &p.second; });
    indices_ = BitSet::fastUnion(bitsets);

    indicesDirty_ = false;
}

void IndexList::serialize(Serializer& s) const {
    s.serialize("source", indicesBySource_, "indices");
}

void IndexList::deserialize(Deserializer& d) {
    indicesBySource_.clear();
    d.deserialize("source", indicesBySource_, "indices");
    indicesDirty_ = true;
}

}  // namespace inviwo
