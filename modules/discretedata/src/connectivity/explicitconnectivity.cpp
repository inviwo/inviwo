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

#include <modules/discretedata/connectivity/explicitconnectivity.h>
#include <modules/discretedata/util/util.h>
#include <unordered_set>
#ifndef __clang__
#include <omp.h>
#endif

namespace inviwo {
namespace discretedata {

ind ExplicitConnectivity::getNumElements(GridPrimitive elementType) const {
    assert((ind)numGridPrimitives_.size() == (ind)gridDimension_ + 1 &&
           "GridPrimitive count vector has the wrong size.");
    if (numGridPrimitives_[(int)elementType] == -1) {
        // Build the map from the queried primitve type to vertex.
        auto key = std::make_pair(elementType, GridPrimitive::Vertex);
        auto map = createConnectionMap(elementType, GridPrimitive::Vertex);
        connectionMaps_[key] = map;
        numGridPrimitives_[(int)elementType] = map->size();
    }
    return numGridPrimitives_[(int)elementType];
}

void ExplicitConnectivity::createConnectionMapToAndFromVertex(GridPrimitive primitive) const {
    dd_util::PrefixSumVector<ind> baseVertToPrim;

    // Go from highest level primitive to selected primitive.
    auto maxToVert = connectionMaps_[{gridDimension_, GridPrimitive::Vertex}];
#pragma omp parallel for
    for (ind idx = 0; idx < maxToVert->size(); ++idx) {
        std::unordered_set<std::vector<ind>, dd_util::VectorHash<ind>, dd_util::VectorCompare<ind>>
            verts;

        // HERE HERE
        // find & insert
    }

    // std::unordered_set<std::vector<ind>>
}

void ExplicitConnectivity::createInverseConnectionMap(GridPrimitive from, GridPrimitive to) const {
    auto fromTo = connectionMaps_[{from, to}];
    std::vector<std::vector<ind>> result(fromTo->size());

    for (ind i = 0; i < fromTo->size(); ++i)
        for (ind j = fromTo->prefixSum_[i]; j < fromTo->prefixSum_[i + 1]; ++j)
            result[fromTo->data_[j]].push_back(i);

    ConnectionMap map = std::make_shared<dd_util::PrefixSumVector<ind>>(result);
    connectionMaps_[{to, from}] = map;
}

ConnectionMap ExplicitConnectivity::createConnectionMap(GridPrimitive from,
                                                        GridPrimitive to) const {
    ivwAssert((connectionMaps_[{gridDimension_, GridPrimitive::Vertex}]),
              "Map from cells to vertices required.");
    ivwAssert(!(connectionMaps_[{from, to}]), "Map already exists.");

    // Need to have map from 'from' to vertex to assign indices.
    auto& fromToVert = connectionMaps_[{from, GridPrimitive::Vertex}];
    if (!fromToVert) createConnectionMapToAndFromVertex(from);
    // HERE, TODO
    auto& cellByVert = connectionMaps_[{GridPrimitive::Vertex, gridDimension_}];
}

void ExplicitConnectivity::getConnections(std::vector<ind>& result, ind index, GridPrimitive from,
                                          GridPrimitive to, bool isPosition) const {
    // auto primPair = std::make_pair(from, to);
    auto map = connectionMaps_[{from, to}];

    // First time asking for a connection of this kind.
    if (!map) {
        map = createConnectionMap(from, to);
        // If we never accessed this kind of primitive before,
        // we get the count of it as a by-product.
        if (!numGridPrimitives_[(ind)from]) numGridPrimitives_[(ind)from] = map->size();
    }

    map->getBlock(index, result);
}  // namespace discretedata

}  // namespace discretedata
}  // namespace inviwo
