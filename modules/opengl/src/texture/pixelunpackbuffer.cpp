/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/opengl/texture/pixelunpackbuffer.h>

#include <inviwo/core/util/exception.h>

#include <utility>

namespace inviwo {

PixelUnpackBuffer::PixelUnpackBuffer(size_t sizeInBytes) : sizeInBytes_{sizeInBytes} {
    if (sizeInBytes_ == 0) {
        throw OpenGLException{SourceContext{}, "Pixel buffer size must be greater than 0"};
    }
    glGenBuffers(1, &id_);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, id_);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(sizeInBytes_), nullptr,
                 GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    LGL_ERROR_CLASS;
}

PixelUnpackBuffer::PixelUnpackBuffer(PixelUnpackBuffer&& rhs) noexcept
    : id_{std::exchange(rhs.id_, 0)}
    , sizeInBytes_{std::exchange(rhs.sizeInBytes_, 0)}
    , mapped_{std::exchange(rhs.mapped_, false)} {}

PixelUnpackBuffer& PixelUnpackBuffer::operator=(PixelUnpackBuffer&& rhs) noexcept {
    if (this != &rhs) {
        if (id_ != 0) {
            // glDeleteBuffers implicitly unmaps a mapped buffer
            glDeleteBuffers(1, &id_);
        }
        id_ = std::exchange(rhs.id_, 0);
        sizeInBytes_ = std::exchange(rhs.sizeInBytes_, 0);
        mapped_ = std::exchange(rhs.mapped_, false);
    }
    return *this;
}

PixelUnpackBuffer::~PixelUnpackBuffer() {
    if (id_ != 0) {
        // glDeleteBuffers implicitly unmaps a mapped buffer
        glDeleteBuffers(1, &id_);
    }
}

GLuint PixelUnpackBuffer::getId() const { return id_; }

size_t PixelUnpackBuffer::getSizeInBytes() const { return sizeInBytes_; }

void PixelUnpackBuffer::bind() const { glBindBuffer(GL_PIXEL_UNPACK_BUFFER, id_); }

void PixelUnpackBuffer::unbind() const { glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0); }

std::span<std::byte> PixelUnpackBuffer::map() {
    if (mapped_) {
        throw OpenGLException{SourceContext{}, "Pixel buffer is already mapped"};
    }
    bind();
    void* ptr = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, static_cast<GLsizeiptr>(sizeInBytes_),
                                 GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (!ptr) {
        LGL_ERROR_CLASS;
        unbind();
        throw OpenGLException{SourceContext{}, "Unable to map pixel buffer of {} bytes",
                              sizeInBytes_};
    }
    mapped_ = true;
    return std::span<std::byte>{static_cast<std::byte*>(ptr), sizeInBytes_};
}

bool PixelUnpackBuffer::unmap() {
    if (!mapped_) return true;
    bind();
    const auto result = glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    mapped_ = false;
    LGL_ERROR_CLASS;
    return result == GL_TRUE;
}

bool PixelUnpackBuffer::isMapped() const { return mapped_; }

}  // namespace inviwo
