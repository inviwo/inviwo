/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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
#pragma once

#include <modules/opengl/openglmoduledefine.h>

#include <inviwo/core/util/exception.h>

#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/buffer/bufferobject.h>
#include <modules/opengl/texture/texture.h>

#include <span>

namespace inviwo {

/**
 * \brief A texture backed by buffer storage
 */
template <typename T = std::uint8_t, GLenum InternalFormat = GL_R8UI>
class BufferTexture : public TextureBase {
public:
    BufferTexture(std::uint32_t size)
        : TextureBase(GL_TEXTURE_BUFFER)
        , storage{size * sizeof(T), GLFormats::get(DataFormat<T>::id()), GL_DYNAMIC_DRAW,
                  GL_ARRAY_BUFFER} {

        bind();
        glTexBuffer(GL_TEXTURE_BUFFER, InternalFormat, storage.getId());
        unbind();
    }

    std::span<T> map(GLenum access) {
        storage.bind();
        void* data = glMapBuffer(storage.getTarget(), access);
        if (!data) {
            throw Exception(IVW_CONTEXT, "Unable to map OpenGL buffer '{}'", storage.getId());
        }
        return std::span<T>{reinterpret_cast<T*>(data), getSize()};
    }

    void unmap() {
        if (!glUnmapBuffer(storage.getTarget())) {
            throw Exception(IVW_CONTEXT, "Unable to unmap OpenGL buffer '{}'", storage.getId());
        }
        storage.unbind();
    }

    void setSize(std::uint32_t size) { storage.setSizeInBytes(size * sizeof(T)); }

    std::uint32_t getSize() const {
        return static_cast<std::uint32_t>(storage.getSizeInBytes() / sizeof(T));
    }

    BufferTexture(const BufferTexture&) = delete;
    BufferTexture(BufferTexture&&) = default;
    BufferTexture& operator=(const BufferTexture&) = delete;
    BufferTexture& operator=(BufferTexture&&) = default;
    virtual ~BufferTexture() = default;

    BufferObject storage;
};

}  // namespace inviwo
