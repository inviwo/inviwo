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

#pragma once

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglexception.h>

#include <concepts>
#include <cstddef>
#include <functional>
#include <span>

namespace inviwo {

/**
 * @brief A pixel unpack buffer object (PBO) used to upload pixel data to textures.
 *
 * The buffer can be mapped into the address space of the application, allowing data producers,
 * e.g. file readers, to write directly into memory that OpenGL can transfer to a texture. This
 * avoids an intermediate copy of the data in application memory.
 *
 * While the buffer is bound, the `data` pointer arguments of glTexImage and glTexSubImage are
 * interpreted as byte offsets into the buffer. @see utilgl::mapAndUploadToTexture
 */
class IVW_MODULE_OPENGL_API PixelUnpackBuffer {
public:
    /**
     * Create and allocate a buffer of @p sizeInBytes bytes with GL_STREAM_DRAW usage. The buffer
     * is left unbound.
     */
    explicit PixelUnpackBuffer(size_t sizeInBytes);
    PixelUnpackBuffer(const PixelUnpackBuffer&) = delete;
    PixelUnpackBuffer& operator=(const PixelUnpackBuffer&) = delete;
    PixelUnpackBuffer(PixelUnpackBuffer&& rhs) noexcept;
    PixelUnpackBuffer& operator=(PixelUnpackBuffer&& rhs) noexcept;
    ~PixelUnpackBuffer();

    GLuint getId() const;
    size_t getSizeInBytes() const;

    /// Bind the buffer to GL_PIXEL_UNPACK_BUFFER
    void bind() const;
    /// Unbind any buffer from GL_PIXEL_UNPACK_BUFFER
    void unbind() const;

    /**
     * Map the whole buffer for writing, discarding any previous content. The buffer is bound on
     * return and must not be unbound before calling unmap().
     * @throws OpenGLException if the buffer could not be mapped
     */
    std::span<std::byte> map();

    /**
     * Unmap the buffer. The buffer stays bound.
     * @return false if the data store became corrupt while mapped (glUnmapBuffer returned
     *         GL_FALSE), in which case the contents must be considered undefined.
     */
    bool unmap();

    bool isMapped() const;

private:
    GLuint id_{0};
    size_t sizeInBytes_{0};
    bool mapped_{false};
};

namespace utilgl {

/**
 * Upload data to @p texture through a mapped pixel unpack buffer, avoiding an intermediate copy
 * of the data in application memory.
 *
 * @p fill is invoked with a writable span of GPU accessible memory of
 * `texture.getNumberOfValues() * texture.getSizeInBytes()` bytes, which it must fill completely
 * with data in the client format of the texture (see Texture::getDataFormat). Afterwards the
 * texture is either initialized (@p initialize is true), or its content is replaced.
 *
 * @throws OpenGLException if the buffer could not be mapped, or if the data store was corrupted
 * while mapped. Exceptions thrown by @p fill are propagated, leaving the texture content undefined.
 */
template <typename Tex, std::invocable<std::span<std::byte>> Fill>
void mapAndUploadToTexture(Tex& texture, bool initialize, Fill&& fill) {
    PixelUnpackBuffer pbo{texture.getNumberOfValues() * texture.getSizeInBytes()};

    std::invoke(std::forward<Fill>(fill), pbo.map());

    if (!pbo.unmap()) {
        throw OpenGLException{SourceContext{},
                              "Pixel buffer data store was corrupted during texture upload"};
    }

    if (initialize) {
        texture.initialize(nullptr);
    } else {
        texture.upload(nullptr);
    }
    pbo.unbind();
}

}  // namespace utilgl

}  // namespace inviwo
