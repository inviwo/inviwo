/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/base/algorithm/convexhullmesh.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>

#include <algorithm>

namespace inviwo {

namespace util {

std::shared_ptr<Mesh> convertHullToMesh(const std::vector<vec2> &hull, bool useIndices) {
    auto mesh = std::make_shared<Mesh>(DrawType::Lines, ConnectivityType::Loop);

    if (hull.empty()) {
        return mesh;
    }
    auto vertices = std::make_shared<Buffer<vec2>>();
    auto vBuffer = vertices->getEditableRAMRepresentation();

    vBuffer->append(&hull);

    mesh->addBuffer(BufferType::PositionAttrib, vertices);

    if (useIndices) {
        auto indices = std::make_shared<IndexBuffer>();
        auto indexBuffer = indices->getEditableRAMRepresentation();

        std::vector<uint32_t> seq(hull.size());
        // create a sequence of increasing numbers
        std::iota(seq.begin(), seq.end(), 0);

        indexBuffer->append(&seq);

        mesh->addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::Loop), indices);
    }

    return mesh;
}

}  // namespace util

}  // namespace inviwo
