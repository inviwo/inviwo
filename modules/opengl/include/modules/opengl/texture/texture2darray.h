/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#ifndef IVW_TEXTURE2DARRAY_H
#define IVW_TEXTURE2DARRAY_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/texture/texture.h>

namespace inviwo {

class IVW_MODULE_OPENGL_API Texture2DArray : public Texture {
public:
    Texture2DArray(size3_t dimensions, GLFormats::GLFormat glFormat, GLenum filtering,
                   GLint level = 0);
    Texture2DArray(size3_t dimensions, GLint format, GLint internalformat, GLenum dataType,
                   GLenum filtering, GLint level = 0);
    Texture2DArray(const Texture2DArray& other);
    Texture2DArray& operator=(const Texture2DArray& other);
    virtual ~Texture2DArray() = default;

    Texture2DArray* clone() const;

    void initialize(const void* data);

    size_t getNumberOfValues() const;

    void upload(const void* data);

    void uploadAndResize(const void* data, const size3_t& dim);

    const size3_t& getDimensions() const { return dimensions_; }

protected:
    static void default2DArrayTextureParameterFunction(Texture*);

private:
    size3_t dimensions_;
};

}  // namespace inviwo

#endif  // IVW_TEXTURE2DARRAY_H
