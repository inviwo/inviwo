/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/discretedata/connectivity/connectivity.h>
#include <modules/discretedata/connectivity/elementiterator.h>

namespace inviwo {
namespace discretedata {

ind Connectivity::getNumElements(GridPrimitive elementType) const {
    assert((ind)numGridPrimitives_.size() == (ind)gridDimension_ + 1 &&
           "GridPrimitive count vector has the wrong size.");
    assert(numGridPrimitives_[(int)elementType] != -1 && "No size stored.");
    return numGridPrimitives_[(int)elementType];
}

ElementRange Connectivity::all(GridPrimitive dim) const { return ElementRange(dim, this); }

CellType Connectivity::getCellType(GridPrimitive dim, ind) const {
    switch (dim) {
        case GridPrimitive::Vertex:
            return CellType::Vertex;
        case GridPrimitive::Edge:
            return CellType::Line;
        case GridPrimitive::Face:
            return CellType::Quad;
        case GridPrimitive::Volume:
            return CellType::Hexahedron;
        default:
            return CellType::HigherOrderHexahedron;
    }
}

CellType Connectivity::getCellType(ElementIterator& element) const {
    return getCellType(element.getType(), element.getIndex());
}

}  // namespace discretedata
}  // namespace inviwo
