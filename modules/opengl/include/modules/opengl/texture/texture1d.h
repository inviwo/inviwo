/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025-2026 Inviwo Foundation
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

#include <modules/opengl/openglmoduledefine.h>  // for IVW_MODULE_OPENGL_API

#include <inviwo/core/datastructures/image/imagetypes.h>  // for rgba, SwizzleMask
#include <modules/opengl/inviwoopengl.h>                  // for GLenum, GL_CLAMP_TO_EDGE, GLint
#include <modules/opengl/texture/texture.h>               // for Texture

#include <array>    // for array
#include <cstddef>  // for size_t

namespace inviwo {
struct GLFormat;

class IVW_MODULE_OPENGL_API Texture1D : public Texture {
public:
    Texture1D(size_t width, GLFormat glFormat, GLenum filtering,
              const SwizzleMask& swizzleMask = swizzlemasks::rgba,
              GLenum wrapping = GL_CLAMP_TO_EDGE,
              GLint level = 0);
    Texture1D(size_t width, GLint format, GLint internalFormat, GLenum dataType,
              GLenum filtering, const SwizzleMask& swizzleMask = swizzlemasks::rgba,
              GLenum wrapping = GL_CLAMP_TO_EDGE,
              GLint level = 0);

    Texture1D(const Texture1D& other);
    Texture1D(Texture1D&& other);
    Texture1D& operator=(const Texture1D& other);
    Texture1D& operator=(Texture1D&& other);
    virtual ~Texture1D();

    Texture1D* clone() const;

    void initialize(const void* data);
    void upload(const void* data);

    size_t getNumberOfValues() const;

    const size_t& getDimensions() const { return width_; }
    size_t getWidth() const { return width_; }

    void setWrapping(GLenum wrapping);
    GLenum getWrapping() const;

    void resize(size_t width);

private:
    size_t width_;
};

}  // namespace inviwo
