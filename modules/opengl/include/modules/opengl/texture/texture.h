/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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
#include <modules/opengl/glformats.h>
#include <modules/opengl/texture/textureobserver.h>

#include <inviwo/core/datastructures/image/imagetypes.h>
#include <tcb/span.hpp>
#include <array>
#include <mutex>

namespace inviwo {

class IVW_MODULE_OPENGL_API Texture : public Observable<TextureObserver> {
public:
    Texture(GLenum target, GLFormat glFormat, GLenum filtering, const SwizzleMask& swizzleMask,
            util::span<const GLenum> wrapping, GLint level);
    Texture(GLenum target, GLint format, GLint internalformat, GLenum dataType, GLenum filtering,
            const SwizzleMask& swizzleMask, util::span<const GLenum> wrapping, GLint level);
    Texture(const Texture& other);
    Texture(Texture&& other);
    Texture& operator=(const Texture& other);
    Texture& operator=(Texture&& other);

    virtual ~Texture();

    virtual Texture* clone() const = 0;

    virtual size_t getNumberOfValues() const = 0;

    virtual void upload(const void* data) = 0;

    GLuint getID() const;

    GLenum getTarget() const;
    GLenum getFormat() const;
    GLenum getInternalFormat() const;
    GLenum getDataType() const;
    const DataFormatBase* getDataFormat() const;
    GLenum getFiltering() const;
    GLint getLevel() const;

    GLuint getNChannels() const;
    GLuint getSizeInBytes() const;

    void bind() const;
    void unbind() const;

    void setSwizzleMask(SwizzleMask mask);
    SwizzleMask getSwizzleMask() const;

    void setInterpolation(InterpolationType interpolation);
    InterpolationType getInterpolation() const;

    void setWrapping(util::span<const GLenum> wrapping);
    void getWrapping(util::span<GLenum> wrapping) const;

    void download(void* data) const;
    void downloadToPBO() const;
    void loadFromPBO(const Texture*);

protected:
    void bindFromPBO() const;
    void bindToPBO() const;
    void unbindFromPBO() const;
    void unbindToPBO() const;

    void setupAsyncReadBackPBO() const;
    void setPBOAsInvalid();

    GLenum target_;
    GLenum format_;
    GLenum internalformat_;
    GLenum dataType_;
    GLint level_;

    static constexpr std::array<GLenum, 3> wrapNames{GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T,
                                                     GL_TEXTURE_WRAP_R};

    static constexpr std::array<std::pair<GLenum, size_t>, 5> targetToDim{
        {{GL_TEXTURE_1D, 1},
         {GL_TEXTURE_2D, 2},
         {GL_TEXTURE_3D, 3},
         {GL_TEXTURE_1D_ARRAY, 1},
         {GL_TEXTURE_2D_ARRAY, 2}}};

    static size_t targetDims(GLenum target);
    static GLuint channels(GLenum format);
    static size_t dataTypeSize(GLenum dataType);

    mutable std::mutex syncMutex;
    mutable GLsync syncObj = 0;

private:
    GLuint id_;
    GLuint pboBack_;  // For asynchronous readback to CPU

    mutable bool pboBackIsSetup_;
    mutable bool pboBackHasData_;
};

}  // namespace inviwo
