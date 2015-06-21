/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "texture.h"

namespace inviwo {

Texture::Texture(GLenum target, GLFormats::GLFormat glFormat, GLenum filtering, GLint level)
    : Observable<TextureObserver>(), ReferenceCounter()
    , target_(target)
    , format_(glFormat.format)
    , internalformat_(glFormat.internalFormat)
    , dataType_(glFormat.type)
    , filtering_(filtering)
    , level_(level)
    , texParameterCallback_(new TextureCallback())
    , pboBackIsSetup_(false)
    , pboBackHasData_(false)
{
    glGenTextures(1, &id_);
    numChannels_ = glFormat.channels;
    byteSize_ = numChannels_*glFormat.typeSize;
    glGenBuffers(1, &pboBack_);
    LGL_ERROR_SUPPRESS;
}

Texture::Texture(GLenum target, GLint format, GLint internalformat, GLenum dataType, GLenum filtering, GLint level)
    : Observable<TextureObserver>(), ReferenceCounter()
    , target_(target)
    , format_(format)
    , internalformat_(internalformat)
    , dataType_(dataType)
    , filtering_(filtering)
    , level_(level)
    , texParameterCallback_(new TextureCallback())
    , pboBackIsSetup_(false)
    , pboBackHasData_(false)
{
    glGenTextures(1, &id_);
    setNChannels();
    setSizeInBytes();
    glGenBuffers(1, &pboBack_);
    LGL_ERROR_SUPPRESS;
}

Texture::Texture(const Texture& other)
    : Observable<TextureObserver>(), ReferenceCounter()
    , target_(other.target_)
    , format_(other.format_)
    , internalformat_(other.internalformat_)
    , dataType_(other.dataType_)
    , filtering_(other.filtering_)
    , level_(other.level_)
    , texParameterCallback_(new TextureCallback())
    , byteSize_(other.byteSize_)
    , numChannels_(other.numChannels_)
    , pboBackIsSetup_(false)
    , pboBackHasData_(false)
{
    glGenTextures(1, &id_);
    glGenBuffers(1, &pboBack_);
    LGL_ERROR_SUPPRESS;
}

Texture& Texture::operator=(const Texture& rhs) {
    if (this != &rhs) {
        // Check if this object is shared with OpenCL/CUDA/DirectX
        if (getRefCount() > 1) {
            LogError("This object is shared and cannot changed (size/format etc.) until the shared object has been released");
        }

        target_ = rhs.target_;
        format_ = rhs.format_;
        internalformat_ = rhs.internalformat_;
        dataType_ = rhs.dataType_;
        filtering_ = rhs.filtering_;
        numChannels_ = rhs.numChannels_;
        byteSize_ = rhs.byteSize_;
    }

    return *this;
}

Texture::~Texture() {
    glDeleteTextures(1, &id_);
    glDeleteBuffers(1, &pboBack_);
    delete texParameterCallback_;
}

GLuint Texture::getID() const {
    return id_;
}

GLenum Texture::getTarget() const {
    return target_;
}

GLenum Texture::getFormat() const {
    return format_;
}

GLenum Texture::getInternalFormat() const {
    return internalformat_;
}

GLenum Texture::getDataType() const {
    return dataType_;
}

GLenum Texture::getFiltering() const {
    return filtering_;
}

GLint Texture::getLevel() const {
    return level_;
}

GLuint Texture::getNChannels() const {
    return numChannels_;
}

GLuint Texture::getSizeInBytes() const {
    return byteSize_;
}

void Texture::bind() const {
    glBindTexture(target_, id_);
    LGL_ERROR;
}

void Texture::unbind() const {
    glBindTexture(target_, 0);
    LGL_ERROR;
}

void Texture::bindFromPBO() const {
    if (!pboBackHasData_) {
        downloadToPBO();
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pboBack_);
    LGL_ERROR;
}

void Texture::bindToPBO() const {
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBack_);
    LGL_ERROR;
}

void Texture::unbindFromPBO() const {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    //Invalidate PBO, as error might occur in download() otherwise.
    pboBackHasData_ = false;
    LGL_ERROR;
}

void Texture::unbindToPBO() const {
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
    LGL_ERROR_SUPPRESS;
}

void Texture::download(void* data) const {
    bool downloadComplete = false;
    if (pboBackHasData_) {
        // Copy from PBO
        bindToPBO();
        void* mem = glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);
        if (mem){
            memcpy(data, mem, getNumberOfValues()*getSizeInBytes());
            downloadComplete = true;
        }
        //Release PBO data
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
    if (!pboBackIsSetup_){
        setupAsyncReadBackPBO();
        pboBackIsSetup_ = true;
    }

    bind();
    bindToPBO();
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

void Texture::setPBOAsInvalid() {
     pboBackIsSetup_ = false;
}

void Texture::setNChannels() {
    switch (format_) {
        case GL_STENCIL_INDEX:
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
        case GL_ALPHA:
            numChannels_ = 1;
            break;

        case GL_RGB:
        case GL_BGR:
            numChannels_ = 3;
            break;

        case GL_RGBA:
        case GL_BGRA:
            numChannels_ = 4;
            break;

        default:
            numChannels_ = 0;
            LogError("Invalid format: " << format_);
    }
}

void Texture::setSizeInBytes() {
    GLuint dataTypeSize;

    switch (dataType_) {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
            dataTypeSize = 1;
            break;

        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case GL_HALF_FLOAT:
            dataTypeSize = 2;
            break;

        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT:
            dataTypeSize = 4;
            break;

        default:
            dataTypeSize = 0;
            LogError("Invalid data type: " << dataTypeSize);
    }

    byteSize_ = numChannels_*dataTypeSize;
}





} // namespace
