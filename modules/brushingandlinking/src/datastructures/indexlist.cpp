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

namespace inviwo {

size_t IndexList::size() const { return indices_.size(); }

void IndexList::set(const BrushingAndLinkingInport* src, const BitSet& indices) {
    indicesBySource_[src] = indices;
    update();
}

void IndexList::remove(const BrushingAndLinkingInport* src) {
    indicesBySource_.erase(src);
    update();
}

std::shared_ptr<std::function<void()>> IndexList::onChange(std::function<void()> V) {
    return onUpdate_.add(V);
}

void IndexList::update() {
    indices_.clear();

    using T = PortIndexMap::value_type;
    util::map_erase_remove_if(indicesBySource_, [](const T& p) {
        // remove if port is disconnected or if the set is empty
        return !p.first->isConnected() || p.second.empty();
    });

    auto bitsets =
        util::transform(indicesBySource_, [](auto& p) -> const BitSet* { return &p.second; });
    indices_ = BitSet::fastUnion(bitsets);

    onUpdate_.invoke();
}

void IndexList::clear() {
    indices_.clear();
    indicesBySource_.clear();
    onUpdate_.invoke();
}

void IndexList::serialize(Serializer& s) const {
    s.serialize(
        "ports", indicesBySource_, "indices", {},
        [](const BrushingAndLinkingInport* p) { return p->getPath(); }, util::identity());
}

void IndexList::deserialize(Deserializer& d, const BrushingAndLinkingOutport& port) {
    indicesBySource_.clear();

    std::unordered_map<std::string, BitSet> map;
    d.deserialize("ports", map, "indices");

    const auto connectedPorts = port.getConnectedInports();
    for (auto&& [key, indices] : map) {
        auto it =
            util::find_if(connectedPorts, [path = key](Inport* p) { return p->getPath() == path; });
        if (it != connectedPorts.end()) {
            indicesBySource_[static_cast<BrushingAndLinkingInport*>(*it)] = std::move(indices);
        }
    }
    update();
}

}  // namespace inviwo
