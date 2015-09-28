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
    addAttribute(std::make_shared<BufferVec3Float32>(BufferType::POSITION_ATTRIB));  // pos 0
    addAttribute(std::make_shared<BufferVec3Float32>(BufferType::TEXCOORD_ATTRIB));  // pos 1
    addAttribute(std::make_shared<BufferVec4Float32>(BufferType::COLOR_ATTRIB));     // pos 2
    addIndicies(Mesh::AttributesInfo(dt, ct),
                std::make_shared<BufferUInt32>(BufferType::INDEX_ATTRIB));
}

SimpleMesh* SimpleMesh::clone() const { return new SimpleMesh(*this); }

unsigned int SimpleMesh::addVertex(vec3 pos, vec3 texCoord, vec4 color) {
    auto posBuffer =
        static_cast<Vec3BufferRAM*>(attributes_[0]->getEditableRepresentation<BufferRAM>());
    posBuffer->add(pos);
    static_cast<Vec3BufferRAM*>(attributes_[1]->getEditableRepresentation<BufferRAM>())
        ->add(texCoord);
    static_cast<Vec4BufferRAM*>(attributes_[2]->getEditableRepresentation<BufferRAM>())
        ->add(color);
    return static_cast<unsigned int>(posBuffer->getSize() - 1);
}

void SimpleMesh::addIndex(unsigned int idx) {
    static_cast<UInt32BufferRAM*>(indexAttributes_[0].second->getEditableRepresentation<BufferRAM>())
        ->add(idx);
}

void SimpleMesh::setIndicesInfo(DrawType dt, ConnectivityType ct) {
    indexAttributes_[0].first = Mesh::AttributesInfo(dt, ct);
}

const BufferVec3Float32* SimpleMesh::getVertexList() const {
    return static_cast<const BufferVec3Float32*>(attributes_[0].get());
}

const BufferVec3Float32* SimpleMesh::getTexCoordList() const {
    return static_cast<const BufferVec3Float32*>(attributes_[1].get());
}

const BufferVec4Float32* SimpleMesh::getColorList() const {
    return static_cast<const BufferVec4Float32*>(attributes_[2].get());
}

const BufferUInt32* SimpleMesh::getIndexList() const { return indexAttributes_[0].second.get(); }

}  // namespace
