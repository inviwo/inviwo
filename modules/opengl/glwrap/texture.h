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

#ifndef IVW_TEXTURE_H
#define IVW_TEXTURE_H

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/glwrap/texturecallback.h>
#include <inviwo/core/util/referencecounter.h>
#include <modules/opengl/textureobserver.h>

namespace inviwo {

class IVW_MODULE_OPENGL_API Texture: public Observable<TextureObserver>, public ReferenceCounter {

public:
    Texture(GLenum, GLFormats::GLFormat glFormat, GLenum filtering, GLint level = 0);
    Texture(GLenum, GLint format, GLint internalformat, GLenum dataType, GLenum filtering, GLint level = 0);
    Texture(const Texture& other);
    Texture& operator=(const Texture& other);
    virtual ~Texture();

    virtual Texture* clone() const = 0;

    virtual size_t getNumberOfValues() const = 0;

    virtual void upload(const void* data) = 0;

    GLuint getID() const;

    GLenum getTarget() const;
    GLenum getFormat() const;
    GLenum getInternalFormat() const;
    GLenum getDataType() const;
    GLenum getFiltering() const;
    GLint getLevel() const;

    GLuint getNChannels() const;
    GLuint getSizeInBytes() const;

    template <typename T>
    void setTextureParameterFunction(T* o, void (T::*m)(Texture*)) const {
        texParameterCallback_->addMemberFunction(o,m);
    }

    void bind() const;
    void unbind() const;

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

    void setNChannels();
    void setSizeInBytes();

    GLenum target_;
    GLenum format_;
    GLenum internalformat_;
    GLenum dataType_;
    GLenum filtering_;
    GLint level_;

    TextureCallback* texParameterCallback_;

private:
    GLuint id_;
    GLuint pboBack_; //For asynchronous readback to CPU

    GLuint byteSize_;
    GLuint numChannels_;

    mutable bool pboBackIsSetup_;
    mutable bool pboBackHasData_;
};

} // namespace

#endif // IVW_TEXTURE_H
