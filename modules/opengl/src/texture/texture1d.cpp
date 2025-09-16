/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <modules/opengl/texture/texture1d.h>

#include <inviwo/core/datastructures/image/imagetypes.h>  // for SwizzleMask
#include <modules/opengl/glformats.h>                     // for GLFormat
#include <modules/opengl/inviwoopengl.h>                  // for GLenum, GLsizei, glPixelStorei
#include <modules/opengl/openglcapabilities.h>            // for OpenGLCapabilities
#include <modules/opengl/texture/texture.h>               // for Texture
#include <modules/opengl/texture/textureobserver.h>       // for TextureObserver
#include <inviwo/core/resourcemanager/resource.h>

#include <mutex>   // for scoped_lock
#include <vector>  // for vector

#include <span>  // for span

namespace inviwo {

Texture1D::Texture1D(size_t width, GLFormat glFormat, GLenum filtering,
                     const SwizzleMask& swizzleMask, GLenum wrapping, GLint level)
    : Texture(GL_TEXTURE_1D, glFormat, filtering, swizzleMask,
              std::span<const GLenum, 1>(&wrapping, 1), level)
    , width_(width) {}

Texture1D::Texture1D(size_t width, GLint format, GLint internalformat, GLenum dataType,
                     GLenum filtering, const SwizzleMask& swizzleMask, GLenum wrapping, GLint level)
    : Texture(GL_TEXTURE_1D, format, internalformat, dataType, filtering, swizzleMask,
              std::span<const GLenum, 1>(&wrapping, 1), level)
    , width_(width) {}

Texture1D::Texture1D(const Texture1D& rhs) : Texture(rhs), width_(rhs.width_) {
    initialize(nullptr);
    if (OpenGLCapabilities::getOpenGLVersion() >= 430) {  // GPU memcpy
        std::scoped_lock lock{syncMutex};
        if (syncObj != 0) {
            glDeleteSync(syncObj);
            syncObj = 0;
        }

        glCopyImageSubData(rhs.getID(), rhs.getTarget(), 0, 0, 0, 0, getID(), target_, 0, 0, 0, 0,
                           static_cast<GLsizei>(width_), 1, 1);
        syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    } else {  // Copy data through PBO
        loadFromPBO(&rhs);
    }
}

Texture1D::Texture1D(Texture1D&& rhs) = default;

Texture1D& Texture1D::operator=(const Texture1D& rhs) {
    if (this != &rhs) {
        Texture::operator=(rhs);
        width_ = rhs.width_;
        initialize(nullptr);

        if (OpenGLCapabilities::getOpenGLVersion() >= 430) {  // GPU memcpy
            std::scoped_lock lock{syncMutex};
            if (syncObj != 0) {
                glDeleteSync(syncObj);
                syncObj = 0;
            }

            glCopyImageSubData(rhs.getID(), rhs.getTarget(), 0, 0, 0, 0, getID(), target_, 0, 0, 0,
                               0, static_cast<GLsizei>(rhs.width_), 1, 1);
            syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        } else {  // Copy data through PBO
            loadFromPBO(&rhs);
        }
    }

    return *this;
}

Texture1D& Texture1D::operator=(Texture1D&& rhs) = default;

Texture1D::~Texture1D() { resource::remove(resource::GL{id_}); }

Texture1D* Texture1D::clone() const { return new Texture1D(*this); }

void Texture1D::initialize(const void* data) {
    // Notify observers
    forEachObserver([](TextureObserver* o) { o->notifyBeforeTextureInitialization(); });

    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage1D(GL_TEXTURE_1D, level_, internalformat_, static_cast<GLsizei>(width_), 0, format_,
                 dataType_, data);
    LGL_ERROR;
    forEachObserver([](TextureObserver* o) { o->notifyAfterTextureInitialization(); });

    auto old = resource::remove(resource::GL{id_});
    resource::add(resource::GL{id_}, Resource{.dims = glm::size4_t{width_, 0, 0, 0},
                                              .format = getDataFormat()->getId(),
                                              .desc = "Texture1D",
                                              .meta = resource::getMeta(old)});
}

size_t Texture1D::getNumberOfValues() const { return width_; }

void Texture1D::upload(const void* data) {
    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, static_cast<GLsizei>(width_), format_, dataType_, data);
    LGL_ERROR_CLASS;
}

void Texture1D::setWrapping(GLenum wrapping) {
    Texture::setWrapping(std::span<const GLenum, 1>(&wrapping, 1));
}

GLenum Texture1D::getWrapping() const {
    GLenum wrapping;
    Texture::getWrapping(std::span<GLenum, 1>(&wrapping, 1));
    return wrapping;
}

void Texture1D::resize(size_t width) {
    if (width_ != width) {
        width_ = width;
        setPBOAsInvalid();
        initialize(nullptr);
    }
}

}  // namespace inviwo
