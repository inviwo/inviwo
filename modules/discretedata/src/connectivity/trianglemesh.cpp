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

#include <modules/discretedata/connectivity/trianglemesh.h>
#include <modules/discretedata/connectivity/elementiterator.h>

namespace inviwo {
namespace discretedata {

TriangleMesh::TriangleMesh(ind numVertices, std::shared_ptr<const DataChannel<ind, 3>> triangleList)
    : ExplicitConnectivity(GridPrimitive::Face)
    , numVertices_(numVertices)
    , triangleList_(triangleList) {}

TriangleMesh::TriangleMesh(ind numVertices, const std::vector<ind>& triangleList)
    : ExplicitConnectivity(GridPrimitive::Face)
    , numVertices_(numVertices)
    , triangleList_(
          new BufferChannel<ind, 3>(triangleList, "Triangle Indices", GridPrimitive::Face)) {}

void TriangleMesh::setBaseMap() {
    ivwAssert(triangleList_->getGridPrimitiveType() == GridPrimitive::Face,
              "Assuming triangle indices to be defined on faces.");

    ind numTriangles = triangleList_->size();

    ConnectionMap map;
    map->data_.resize(numTriangles * 3);
    triangleList_->fill(*reinterpret_cast<glm::vec<3, ind>*>(map->data_.data()), 0, numTriangles);

    map->prefixSum_.resize(numTriangles + 1);
    for (ind i = 0; i < numTriangles + 1; ++i) map->prefixSum_[i] = i * 3;

    connectionMaps_[{GridPrimitive::Face, GridPrimitive::Vertex}] = map;
}

// ConnectionMap TriangleMesh::createConnectionMap(GridPrimitive from, GridPrimitive to) const {
//     // Convert existing triangle list into a connection map.
//     if (from == GridPrimitive::Face && to == GridPrimitive::Vertex) {
//         ind numTriangles = triangleList_->size();

//         ConnectionMap map;
//         map->data_.resize(numTriangles * 3);
//         triangleList_->fill(*reinterpret_cast<glm::vec<3, ind>*>(map->data_.data()), 0,
//                             numTriangles);

//         map->prefixSum_.resize(numTriangles + 1);
//         for (ind i = 0; i < numTriangles + 1; ++i) map->prefixSum_[i] = i * 3;

//         return map;
//     }

//     // Create map from vertices to triangles.
//     if (from == GridPrimitive::Vertex && to == GridPrimitive::Face) {
//     }
// }

}  // namespace discretedata
}  // namespace inviwo
