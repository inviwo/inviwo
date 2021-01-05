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

#include <modules/opengl/texture/texture.h>

#include <modules/opengl/openglutils.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

Texture::Texture(GLenum target, GLFormat glFormat, GLenum filtering, const SwizzleMask& swizzleMask,
                 util::span<const GLenum> wrapping, GLint level)
    : Observable<TextureObserver>()
    , target_(target)
    , format_(glFormat.format)
    , internalformat_(glFormat.internalFormat)
    , dataType_(glFormat.type)
    , level_(level)
    , pboBackIsSetup_(false)
    , pboBackHasData_(false) {

    glGenTextures(1, &id_);

    glGenBuffers(1, &pboBack_);

    glBindTexture(target_, id_);
    auto swizzleMaskGL = utilgl::convertSwizzleMaskToGL(swizzleMask);
    glTexParameteriv(target_, GL_TEXTURE_SWIZZLE_RGBA, swizzleMaskGL.data());
    glTexParameterIuiv(target_, GL_TEXTURE_MIN_FILTER, &filtering);
    glTexParameterIuiv(target_, GL_TEXTURE_MAG_FILTER, &filtering);

    for (auto [i, wrap] : util::enumerate(wrapping)) {
        glTexParameteri(target_, wrapNames[i], wrap);
    }

    glBindTexture(target_, 0);
}

Texture::Texture(GLenum target, GLint format, GLint internalformat, GLenum dataType,
                 GLenum filtering, const SwizzleMask& swizzleMask,
                 util::span<const GLenum> wrapping, GLint level)
    : Observable<TextureObserver>()
    , target_(target)
    , format_(format)
    , internalformat_(internalformat)
    , dataType_(dataType)
    , level_(level)
    , pboBackIsSetup_(false)
    , pboBackHasData_(false) {

    glGenTextures(1, &id_);

    glGenBuffers(1, &pboBack_);

    glBindTexture(target_, id_);
    auto swizzleMaskGL = utilgl::convertSwizzleMaskToGL(swizzleMask);
    glTexParameteriv(target_, GL_TEXTURE_SWIZZLE_RGBA, swizzleMaskGL.data());
    glTexParameterIuiv(target_, GL_TEXTURE_MIN_FILTER, &filtering);
    glTexParameterIuiv(target_, GL_TEXTURE_MAG_FILTER, &filtering);
    for (auto [i, wrap] : util::enumerate(wrapping)) {
        glTexParameteri(target_, wrapNames[i], wrap);
    }
    glBindTexture(target_, 0);
}

Texture::Texture(const Texture& other)
    : Observable<TextureObserver>()
    , target_(other.target_)
    , format_(other.format_)
    , internalformat_(other.internalformat_)
    , dataType_(other.dataType_)
    , level_(other.level_)
    , pboBackIsSetup_(false)
    , pboBackHasData_(false) {

    glGenTextures(1, &id_);
    glGenBuffers(1, &pboBack_);

    std::array<GLint, 4> swizzleMaskGL;
    GLenum minfiltering;
    GLenum magfiltering;

    std::array<GLenum, 3> wrapping;
    const auto dims = targetDims(target_);

    glBindTexture(other.target_, other.id_);
    glGetTexParameteriv(other.target_, GL_TEXTURE_SWIZZLE_RGBA, swizzleMaskGL.data());
    glGetTexParameterIuiv(other.target_, GL_TEXTURE_MIN_FILTER, &minfiltering);
    glGetTexParameterIuiv(other.target_, GL_TEXTURE_MAG_FILTER, &magfiltering);
    for (size_t i = 0; i < dims; ++i) {
        glGetTexParameterIuiv(target_, wrapNames[i], &(wrapping[i]));
    }

    glBindTexture(target_, id_);
    glTexParameteriv(target_, GL_TEXTURE_SWIZZLE_RGBA, swizzleMaskGL.data());
    glTexParameterIuiv(target_, GL_TEXTURE_MIN_FILTER, &minfiltering);
    glTexParameterIuiv(target_, GL_TEXTURE_MAG_FILTER, &minfiltering);
    for (size_t i = 0; i < dims; ++i) {
        glTexParameterIuiv(target_, wrapNames[i], &(wrapping[i]));
    }
    glBindTexture(target_, 0);
}

Texture::Texture(Texture&& other)
    : Observable<TextureObserver>(std::move(other))
    , target_(other.target_)
    , format_(other.format_)
    , internalformat_(other.internalformat_)
    , dataType_(other.dataType_)
    , level_(other.level_)
    , id_(other.id_)  // Steal texture
    , pboBack_(other.pboBack_)
    , pboBackIsSetup_(false)
    , pboBackHasData_(false) {

    // Free resources from other
    other.id_ = 0;
    other.pboBack_ = 0;
}

Texture& Texture::operator=(const Texture& rhs) {
    if (this != &rhs) {
        target_ = rhs.target_;
        format_ = rhs.format_;
        internalformat_ = rhs.internalformat_;
        dataType_ = rhs.dataType_;

        std::array<GLint, 4> swizzleMaskGL;
        GLenum minfiltering;
        GLenum magfiltering;

        std::array<GLenum, 3> wrapping;
        const auto dims = targetDims(target_);

        glBindTexture(rhs.target_, rhs.id_);
        glGetTexParameteriv(rhs.target_, GL_TEXTURE_SWIZZLE_RGBA, swizzleMaskGL.data());
        glGetTexParameterIuiv(rhs.target_, GL_TEXTURE_MIN_FILTER, &minfiltering);
        glGetTexParameterIuiv(rhs.target_, GL_TEXTURE_MAG_FILTER, &magfiltering);
        for (size_t i = 0; i < dims; ++i) {
            glGetTexParameterIuiv(target_, wrapNames[i], &(wrapping[i]));
        }
        glBindTexture(target_, id_);
        glTexParameteriv(target_, GL_TEXTURE_SWIZZLE_RGBA, swizzleMaskGL.data());
        glTexParameterIuiv(target_, GL_TEXTURE_MIN_FILTER, &minfiltering);
        glTexParameterIuiv(target_, GL_TEXTURE_MAG_FILTER, &minfiltering);
        for (size_t i = 0; i < dims; ++i) {
            glTexParameterIuiv(target_, wrapNames[i], &(wrapping[i]));
        }
        glBindTexture(target_, 0);
    }

    return *this;
}

Texture& Texture::operator=(Texture&& rhs) {
    if (this != &rhs) {
        // Free existing resources
        glDeleteTextures(1, &id_);
        glDeleteBuffers(1, &pboBack_);

        // Steal resources
        Observable<TextureObserver>::operator=(std::move(rhs));
        target_ = rhs.target_;
        format_ = rhs.format_;
        internalformat_ = rhs.internalformat_;
        dataType_ = rhs.dataType_;

        id_ = rhs.id_;
        pboBack_ = rhs.pboBack_;

        // Release resources from source object
        rhs.id_ = 0;
        rhs.pboBack_ = 0;
    }

    return *this;
}

Texture::~Texture() {
    if (id_ != 0 || pboBack_ != 0) {
        // These functions silently ignores zeros,
        // which happens when move operations has been used
        glDeleteTextures(1, &id_);
        glDeleteBuffers(1, &pboBack_);
    }
    if (syncObj != 0) {
        glDeleteSync(syncObj);
    }
}

size_t Texture::targetDims(GLenum target) {
    const auto it = std::find_if(targetToDim.begin(), targetToDim.end(),
                                 [&](auto& item) { return item.first == target; });
    return it != targetToDim.end() ? it->second : size_t{0};
}

GLuint Texture::getID() const { return id_; }

GLenum Texture::getTarget() const { return target_; }

GLenum Texture::getFormat() const { return format_; }

GLenum Texture::getInternalFormat() const { return internalformat_; }

GLenum Texture::getDataType() const { return dataType_; }

const DataFormatBase* Texture::getDataFormat() const {
    return DataFormatBase::get(GLFormats::getDataFormat(dataType_, channels(format_)));
}

GLenum Texture::getFiltering() const {
    GLenum filtering;
    bind();
    glGetTexParameterIuiv(target_, GL_TEXTURE_MIN_FILTER, &filtering);
    unbind();
    return filtering;
}

GLint Texture::getLevel() const { return level_; }

GLuint Texture::getNChannels() const { return channels(format_); }

GLuint Texture::getSizeInBytes() const {
    return static_cast<GLuint>(channels(format_) * dataTypeSize(dataType_));
}

void Texture::bind() const { glBindTexture(target_, id_); }

void Texture::unbind() const { glBindTexture(target_, 0); }

void Texture::setSwizzleMask(SwizzleMask mask) {
    auto swizzleMaskGL = utilgl::convertSwizzleMaskToGL(mask);
    bind();
    glTexParameteriv(target_, GL_TEXTURE_SWIZZLE_RGBA, swizzleMaskGL.data());
    unbind();
}

SwizzleMask Texture::getSwizzleMask() const {
    std::array<GLint, 4> swizzleMaskGL;
    bind();
    glGetTexParameteriv(target_, GL_TEXTURE_SWIZZLE_RGBA, swizzleMaskGL.data());
    unbind();

    return utilgl::convertSwizzleMaskFromGL(swizzleMaskGL);
}

void Texture::setInterpolation(InterpolationType interpolation) {
    auto filtering = utilgl::convertInterpolationToGL(interpolation);
    bind();
    glTexParameterIuiv(target_, GL_TEXTURE_MIN_FILTER, &filtering);
    glTexParameterIuiv(target_, GL_TEXTURE_MAG_FILTER, &filtering);
    unbind();
}

InterpolationType Texture::getInterpolation() const {
    GLenum filtering;
    bind();
    glGetTexParameterIuiv(target_, GL_TEXTURE_MIN_FILTER, &filtering);
    unbind();
    return utilgl::convertInterpolationFromGL(filtering);
}

void Texture::setWrapping(util::span<const GLenum> wrapping) {
    bind();
    for (auto [i, wrap] : util::enumerate(wrapping)) {
        glTexParameteri(target_, wrapNames[i], wrap);
    }
    unbind();
}

void Texture::getWrapping(util::span<GLenum> wrapping) const {
    bind();
    for (auto&& [i, wrap] : util::enumerate(wrapping)) {
        glGetTexParameterIuiv(target_, wrapNames[i], &wrap);
    }
    unbind();
}

void Texture::bindFromPBO() const {
    if (!pboBackHasData_) {
        downloadToPBO();
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pboBack_);
}

void Texture::bindToPBO() const { glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBack_); }

void Texture::unbindFromPBO() const {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    // Invalidate PBO, as error might occur in download() otherwise.
    pboBackHasData_ = false;
}

void Texture::unbindToPBO() const {
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
    LGL_ERROR_CLASS;
}

void Texture::download(void* data) const {
    bool downloadComplete = false;

    {
        std::scoped_lock lock{syncMutex};
        if (syncObj != 0) {
            glWaitSync(syncObj, 0, GL_TIMEOUT_IGNORED);
        }
    }

    if (pboBackHasData_) {
        // Copy from PBO
        bindToPBO();
        void* mem = glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);
        if (mem) {
            memcpy(data, mem, getNumberOfValues() * getSizeInBytes());
            downloadComplete = true;
        }
        // Release PBO data
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
        unbindToPBO();
        pboBackHasData_ = false;
    }

    if (!downloadComplete) {
        bind();
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glGetTexImage(target_, 0, format_, dataType_, data);
        unbind();
    }

    LGL_ERROR;
}

void Texture::downloadToPBO() const {
    if (!pboBackIsSetup_) {
        setupAsyncReadBackPBO();
        pboBackIsSetup_ = true;
    }

    {
        std::scoped_lock lock{syncMutex};
        if (syncObj != 0) {
            glWaitSync(syncObj, 0, GL_TIMEOUT_IGNORED);
        }
    }

    bind();
    bindToPBO();
    // copy texture contents into PBO
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(target_, 0, format_, dataType_, nullptr);
    unbindToPBO();
    pboBackHasData_ = true;
}

void Texture::loadFromPBO(const Texture* src) {
    setupAsyncReadBackPBO();
    src->bindFromPBO();
    upload(nullptr);
    unbind();
    src->unbindFromPBO();
    LGL_ERROR;
}

void Texture::setupAsyncReadBackPBO() const {
    bind();
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBack_);
    glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, getNumberOfValues() * getSizeInBytes(), nullptr,
                    GL_STREAM_READ_ARB);
    glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
    unbind();
    pboBackHasData_ = false;
    LGL_ERROR;
}

void Texture::setPBOAsInvalid() { pboBackIsSetup_ = false; }

GLuint Texture::channels(GLenum format) {
    switch (format) {
        case GL_DEPTH_COMPONENT:
        case GL_STENCIL_INDEX:
        case GL_DEPTH_STENCIL:
        case GL_RED:
        case GL_RED_INTEGER:
        case GL_GREEN:
        case GL_GREEN_INTEGER:
        case GL_BLUE:
        case GL_BLUE_INTEGER:
        case GL_ALPHA:
            return 1;

        case GL_RG:
        case GL_RG_INTEGER:
            return 2;

        case GL_RGB:
        case GL_BGR:
        case GL_RGB_INTEGER:
        case GL_BGR_INTEGER:
            return 3;

        case GL_RGBA:
        case GL_BGRA:
        case GL_RGBA_INTEGER:
        case GL_BGRA_INTEGER:
            return 4;
        default:
            throw OpenGLException("Invalid format specified", IVW_CONTEXT_CUSTOM("Texture"));
    }
}

size_t Texture::dataTypeSize(GLenum dataType) {
    switch (dataType) {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
            return 1;

        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case GL_HALF_FLOAT:
            return 2;

        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT:
            return 4;

        default:
            throw OpenGLException("Invalid format specified", IVW_CONTEXT_CUSTOM("Texture"));
    }
}

}  // namespace inviwo
