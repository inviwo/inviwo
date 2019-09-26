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

#include <modules/discretedata/connectivity/lineset.h>
#include <modules/discretedata/connectivity/elementiterator.h>

namespace inviwo {
namespace discretedata {

LineSet::LineSet(const std::vector<ind>& numVertsPerLine)
    : Connectivity(GridPrimitive::Edge)
    , numVerticesPerLine_(numVertsPerLine)
    , indexPrefixSum_(numVertsPerLine.size() + 1) {

    indexPrefixSum_[0] = 0;
    for (ind l = 0; l < getNumLines(); ++l)
        indexPrefixSum_[l + 1] = indexPrefixSum_[l] + numVerticesPerLine_[l];

    ind numVertices = indexPrefixSum_[getNumLines()];
    Connectivity::numGridPrimitives_[(ind)GridPrimitive::Vertex] = numVertices;
    Connectivity::numGridPrimitives_[(ind)GridPrimitive::Edge] = numVertices - getNumLines();
}

void LineSet::getConnections(std::vector<ind>& result, ind idx, GridPrimitive from,
                             GridPrimitive to, bool) const {
    result.clear();
    switch (from) {
        case GridPrimitive::Vertex: {
            IVW_ASSERT(idx >= 0 && idx < numGridPrimitives_[0], "Vertex index not valid.");

            // Find the line the index belongs to.
            bool toEdge = (to == GridPrimitive::Edge);
            auto nextIndex = std::lower_bound(indexPrefixSum_.begin(), indexPrefixSum_.end(), idx);
            ind lineIdx = nextIndex - indexPrefixSum_.begin() - (*nextIndex == idx ? 0 : 1);
            ind indexToEdgeOffset = toEdge ? -lineIdx : 0;

            // Is the index the first vertex of a line?
            if (*nextIndex - idx != 0) {
                result.push_back(idx - 1 + indexToEdgeOffset);
            } else
                nextIndex++;
            // Is the index the last vertex of a line?
            if (*nextIndex - idx != 1) result.push_back(idx + (toEdge ? 0 : 1) + indexToEdgeOffset);
            return;
        }
        case GridPrimitive::Edge: {
            IVW_ASSERT(idx >= 0 && idx < numGridPrimitives_[1], "Edge index not valid.");

            // Find close lower bound of the line the index belongs to.
            auto nextIndex = std::lower_bound(indexPrefixSum_.begin(), indexPrefixSum_.end(), idx);
            if (nextIndex != indexPrefixSum_.begin()) nextIndex--;

            // Step until correct edge is found: subtract one endpoint per line.
            while (*(nextIndex + 1) - (nextIndex - indexPrefixSum_.begin() + 1) <= idx) ++nextIndex;
            ind lineIdx = nextIndex - indexPrefixSum_.begin();

            // An edge always has two vertex neighbors.
            if (to == GridPrimitive::Vertex) {
                result.push_back(idx + lineIdx);
                result.push_back(idx + lineIdx + 1);
            } else {
                // Is the index the first edge of a line?
                if (*nextIndex - lineIdx != idx) result.push_back(idx - 1);
                // Is the index the last edge of a line?
                if (*(nextIndex + 1) - lineIdx - 2 != idx) result.push_back(idx + 1);
            }
            return;
        }
        default:
            IVW_ASSERT(false, "Only two dimensional primitives in line sets.");
    }
}

// CellType LineSet::getCellType(GridPrimitive dim, ind index) const {
//     return dim == GridPrimitive::Vertex ? CellType::Vertex : CellType::Line;
// }

}  // namespace discretedata
}  // namespace inviwo
