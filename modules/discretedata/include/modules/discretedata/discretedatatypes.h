/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2021 Inviwo Foundation
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

#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <unordered_map>
#include <string>

namespace inviwo {
namespace discretedata {

//! Discretedata index type
using ind = signed long long;

/**
 * Mapping structure name to respective dimension.
 * Assign channels to any dimensions this way.
 * If these do not suffice, cast the respective ind.
 */
enum class GridPrimitive : int {
    Undef = -1,
    Vertex = 0,
    Edge = 1,
    Face = 2,
    Volume = 3,
    HyperVolume = 4
};

inline std::string primitiveName(GridPrimitive primitive) {
    switch (primitive) {
        case GridPrimitive::Vertex:
            return "Vertex";
        case GridPrimitive::Edge:
            return "Edge";
        case GridPrimitive::Face:
            return "Face";
        case GridPrimitive::Volume:
            return "Volume";
        case GridPrimitive::HyperVolume:
            return "HyperVolume";
        default:
            std::string nD = std::to_string(static_cast<ind>(primitive));
            nD += 'D';
            return nD;
    }
}

}  // namespace discretedata

template <>
struct InviwoDefaults<discretedata::ind> {
    static InviwoDefaultData<discretedata::ind> get() {
        return {"ind",
                uvec2(1, 1),
                discretedata::ind(1),
                discretedata::ind(-100),
                discretedata::ind(100),
                discretedata::ind(1)};
    }
};

template <>
struct InviwoDefaults<discretedata::GridPrimitive> {
    static InviwoDefaultData<discretedata::GridPrimitive> get() {
        return {"GridPrimitive",
                uvec2(1, 1),
                discretedata::GridPrimitive::Vertex,
                discretedata::GridPrimitive::Vertex,
                discretedata::GridPrimitive::HyperVolume,
                static_cast<discretedata::GridPrimitive>(1)};
    }
};

}  // namespace inviwo
