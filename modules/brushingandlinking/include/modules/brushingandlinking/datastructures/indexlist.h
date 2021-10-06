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

#pragma once

#include <modules/brushingandlinking/brushingandlinkingmoduledefine.h>
#include <inviwo/core/datastructures/bitset.h>
#include <inviwo/core/util/dispatcher.h>

#include <unordered_map>
#include <unordered_set>

namespace inviwo {
class BrushingAndLinkingInport;
class BrushingAndLinkingOutport;
class BrushingAndLinkingManager;
class Serializer;
class Deserializer;

class IVW_MODULE_BRUSHINGANDLINKING_API IndexList {
public:
    IndexList() = default;

    size_t size() const;
    bool contains(uint32_t idx) const;

    void set(const BrushingAndLinkingInport* src, const BitSet& incices);
    void remove(const BrushingAndLinkingInport* src);

    std::shared_ptr<std::function<void()>> onChange(std::function<void()> V);

    void update();
    void clear();
    const BitSet& getIndices() const { return indices_; }

    void serialize(Serializer& s) const;
    void deserialize(Deserializer& d, const BrushingAndLinkingOutport& port);

private:
    using PortIndexMap = std::unordered_map<const BrushingAndLinkingInport*, BitSet>;
    PortIndexMap indicesBySource_;
    BitSet indices_;
    Dispatcher<void()> onUpdate_;
};

inline bool IndexList::contains(uint32_t idx) const { return indices_.contains(idx); }

}  // namespace inviwo
