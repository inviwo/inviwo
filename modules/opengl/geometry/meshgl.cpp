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

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <modules/opengl/buffer/elementbuffergl.h>
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

const BufferGL* MeshGL::getBufferGL(size_t idx) const { return bufferGLs_[idx]; }

// save all buffers and to lazy attachment in enable.
void MeshGL::update(bool editable) {
    bufferGLs_.clear();
    Mesh* owner = this->getOwner();

    if (editable) {
        for (auto buf : owner->getBuffers()) {
            auto bufGL = buf.second->getEditableRepresentation<BufferGL>();
            bufferGLs_.push_back(bufGL);
            bufferArray_.attachBufferObject(bufGL->getBufferObject().get(),
                                            static_cast<GLuint>(buf.first));
        }
    } else {
        for (auto buf : owner->getBuffers()) {
            auto bufGL = buf.second->getRepresentation<BufferGL>();
            bufferGLs_.push_back(bufGL);
            bufferArray_.attachBufferObject(bufGL->getBufferObject().get(),
                                            static_cast<GLuint>(buf.first));
        }
    }
}

Mesh* MeshGL::getOwner() { return static_cast<Mesh*>(DataRepresentation::getOwner()); }

const Mesh* MeshGL::getOwner() const {
    return static_cast<const Mesh*>(DataRepresentation::getOwner());
}

std::type_index MeshGL::getTypeIndex() const { return std::type_index(typeid(MeshGL)); }

}  // namespace
