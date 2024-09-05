/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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
#include <inviwo/core/util/glmvec.h>                      // for size3_t
#include <modules/opengl/inviwoopengl.h>                  // for GLenum, GL_CLAMP_TO_EDGE, GLint
#include <modules/opengl/texture/texture.h>               // for Texture
#include <inviwo/core/resourcemanager/resource.h>

#include <array>    // for array
#include <cstddef>  // for size_t

namespace inviwo {
struct GLFormat;

class IVW_MODULE_OPENGL_API Texture2DArray : public Texture {
public:
    Texture2DArray(size3_t dimensions, GLFormat glFormat, GLenum filtering,
                   const SwizzleMask& swizzleMask = swizzlemasks::rgba,
                   const std::array<GLenum, 2>& wrapping = {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE},
                   GLint level = 0);
    Texture2DArray(size3_t dimensions, GLint format, GLint internalformat, GLenum dataType,
                   GLenum filtering, const SwizzleMask& swizzleMask = swizzlemasks::rgba,
                   const std::array<GLenum, 2>& wrapping = {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE},
                   GLint level = 0);
    Texture2DArray(const Texture2DArray& other);
    Texture2DArray(Texture2DArray&& other);
    Texture2DArray& operator=(const Texture2DArray& other);
    Texture2DArray& operator=(Texture2DArray&& other);
    virtual ~Texture2DArray();

    Texture2DArray* clone() const;

    void initialize(const void* data);

    size_t getNumberOfValues() const;

    void upload(const void* data);

    void uploadAndResize(const void* data, const size3_t& dim);

    const size3_t& getDimensions() const { return dimensions_; }

    void setWrapping(const std::array<GLenum, 2>& wrapping);
    std::array<GLenum, 2> getWrapping() const;

private:
    size3_t dimensions_;
};

}  // namespace inviwo
