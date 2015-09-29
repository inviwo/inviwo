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

SimpleMesh::SimpleMesh(DrawType dt, ConnectivityType ct) : Mesh(dt, ct) {
    addBuffer(BufferType::POSITION_ATTRIB, std::make_shared<Buffer<vec3>>());  // pos 0
    addBuffer(BufferType::TEXCOORD_ATTRIB, std::make_shared<Buffer<vec3>>());  // pos 1
    addBuffer(BufferType::COLOR_ATTRIB, std::make_shared<Buffer<vec4>>());     // pos 2
    addIndicies(Mesh::MeshInfo(dt, ct), std::make_shared<IndexBuffer>());
}

SimpleMesh* SimpleMesh::clone() const { return new SimpleMesh(*this); }

unsigned int SimpleMesh::addVertex(vec3 pos, vec3 texCoord, vec4 color) {
    auto posBuffer =
        static_cast<Vec3BufferRAM*>(buffers_[0].second->getEditableRepresentation<BufferRAM>());
    posBuffer->add(pos);
    static_cast<Vec3BufferRAM*>(buffers_[1].second->getEditableRepresentation<BufferRAM>())
        ->add(texCoord);
    static_cast<Vec4BufferRAM*>(buffers_[2].second->getEditableRepresentation<BufferRAM>())
        ->add(color);
    return static_cast<unsigned int>(posBuffer->getSize() - 1);
}

void SimpleMesh::addIndex(unsigned int idx) {
    static_cast<IndexBufferRAM*>(indices_[0].second->getEditableRepresentation<BufferRAM>())
        ->add(idx);
}

void SimpleMesh::setIndicesInfo(DrawType dt, ConnectivityType ct) {
    indices_[0].first = Mesh::MeshInfo(dt, ct);
}

const Buffer<vec3>* SimpleMesh::getVertexList() const {
    return static_cast<const Buffer<vec3>*>(buffers_[0].second.get());
}

const Buffer<vec3>* SimpleMesh::getTexCoordList() const {
    return static_cast<const Buffer<vec3>*>(buffers_[1].second.get());
}

const Buffer<vec4>* SimpleMesh::getColorList() const {
    return static_cast<const Buffer<vec4>*>(buffers_[2].second.get());
}

const IndexBuffer* SimpleMesh::getIndexList() const { return indices_[0].second.get(); }

}  // namespace
