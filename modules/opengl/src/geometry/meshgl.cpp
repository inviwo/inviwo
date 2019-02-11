/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/buffer/buffergl.h>

namespace inviwo {

MeshGL::MeshGL() : MeshRepresentation() {}

MeshGL::MeshGL(const MeshGL& rhs) : MeshRepresentation(rhs) { update(true); }

MeshGL& MeshGL::operator=(const MeshGL& that) {
    if (this != &that) {
        MeshRepresentation::operator=(that);
    }
    update(true);
    return *this;
}

MeshGL* MeshGL::clone() const { return new MeshGL(*this); }

void MeshGL::enable() const { bufferArray_.bind(); }

void MeshGL::disable() const { bufferArray_.unbind(); }

size_t MeshGL::size() const { return bufferGLs_.size(); }

bool MeshGL::empty() const { return bufferGLs_.empty(); }

const BufferGL* MeshGL::getBufferGL(size_t idx) const {
    ivwAssert(idx < bufferGLs_.size(), "MeshGL::getBufferGL(): index out of bounds");
    return bufferGLs_[idx];
}

const Mesh::MeshInfo& MeshGL::getMeshInfoForIndexBuffer(size_t idx) const {
    ivwAssert(idx < indexBuffers_.size(),
              "MeshGL::getMeshInfoForIndexBuffer(): index out of bounds");
    return indexBuffers_[idx].first;
}

const BufferGL* MeshGL::getIndexBuffer(size_t idx) const {
    ivwAssert(idx < indexBuffers_.size(), "MeshGL::getIndexBuffer(): index out of bounds");
    return indexBuffers_[idx].second;
}

size_t MeshGL::getIndexBufferCount() const { return indexBuffers_.size(); }

// save all buffers and to lazy attachment in enable.
void MeshGL::update(bool editable) {
    bufferGLs_.clear();
    indexBuffers_.clear();

    Mesh* owner = this->getOwner();
    // update all buffers except index buffers, i.e. position, color, normals, etc.)
    for (auto buf : owner->getBuffers()) {
        const BufferGL* bufGL = editable ? buf.second->getEditableRepresentation<BufferGL>()
                                         : buf.second->getRepresentation<BufferGL>();
        bufferGLs_.push_back(bufGL);
        bufferArray_.attachBufferObject(bufGL->getBufferObject().get(),
                                        static_cast<GLuint>(buf.first.location));
    }
    // update index buffers
    for (auto buf : owner->getIndexBuffers()) {
        const BufferGL* bufGL = editable ? buf.second->getEditableRepresentation<BufferGL>()
                                         : buf.second->getRepresentation<BufferGL>();
        indexBuffers_.push_back({buf.first, bufGL});
    }
}

std::type_index MeshGL::getTypeIndex() const { return std::type_index(typeid(MeshGL)); }

bool MeshGL::isValid() const {
    return util::all_of(bufferGLs_, [](const auto& b) { return b->isValid(); });
}

}  // namespace inviwo
