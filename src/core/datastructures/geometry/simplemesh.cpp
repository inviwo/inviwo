/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/simplemesh.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

namespace inviwo {

SimpleMesh::SimpleMesh(GeometryEnums::DrawType dt, GeometryEnums::ConnectivityType ct)
    : Mesh(dt, ct) {

    addAttribute(new Position3dBuffer()); // pos 0
    addAttribute(new TexCoord3dBuffer()); // pos 1
    addAttribute(new ColorBuffer());      // pos 2
    addIndicies(Mesh::AttributesInfo(dt,ct),  new IndexBuffer());
}

SimpleMesh::SimpleMesh(const SimpleMesh& rhs) : Mesh(rhs) {}

SimpleMesh& SimpleMesh::operator=(const SimpleMesh& that) {
    if (this != &that) {
        Mesh::operator=(that);
    }
    return *this;
}

SimpleMesh* SimpleMesh::clone() const {
    return new SimpleMesh(*this);
}

SimpleMesh::~SimpleMesh() {
    deinitialize();
}

void SimpleMesh::addVertex(vec3 pos, vec3 texCoord, vec4 color) {
    static_cast<Position3dBuffer*>(attributes_[0])->getEditableRepresentation<Position3dBufferRAM>()->add(pos);
    static_cast<TexCoord3dBuffer*>(attributes_[1])->getEditableRepresentation<TexCoord3dBufferRAM>()->add(texCoord);
    static_cast<ColorBuffer*>(attributes_[2])->getEditableRepresentation<ColorBufferRAM>()->add(color);
}

void SimpleMesh::addIndex(unsigned int idx) {
    indexAttributes_[0].second->getEditableRepresentation<IndexBufferRAM>()->add(idx);
}

void SimpleMesh::setIndicesInfo(GeometryEnums::DrawType dt, GeometryEnums::ConnectivityType ct) {
    indexAttributes_[0].first = Mesh::AttributesInfo(dt, ct);
}

const Position3dBuffer* SimpleMesh::getVertexList() const {
    return static_cast<const Position3dBuffer*>(attributes_[0]);
}

const TexCoord3dBuffer* SimpleMesh::getTexCoordList() const {
    return static_cast<const TexCoord3dBuffer*>(attributes_[1]);
}

const ColorBuffer* SimpleMesh::getColorList() const {
    return static_cast<const ColorBuffer*>(attributes_[2]);
}

const IndexBuffer* SimpleMesh::getIndexList() const {
    return indexAttributes_[0].second;
}



} // namespace

