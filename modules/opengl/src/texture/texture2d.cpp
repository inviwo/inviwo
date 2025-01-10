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

#include <modules/opengl/texture/texture2d.h>

#include <inviwo/core/datastructures/image/imagetypes.h>  // for SwizzleMask
#include <inviwo/core/util/glmvec.h>                      // for size2_t
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

Texture2D::Texture2D(size2_t dimensions, GLFormat glFormat, GLenum filtering,
                     const SwizzleMask& swizzleMask, const std::array<GLenum, 2>& wrapping,
                     GLint level)
    : Texture(GL_TEXTURE_2D, glFormat, filtering, swizzleMask, std::span(wrapping), level)
    , dimensions_(dimensions) {}

Texture2D::Texture2D(size2_t dimensions, GLint format, GLint internalformat, GLenum dataType,
                     GLenum filtering, const SwizzleMask& swizzleMask,
                     const std::array<GLenum, 2>& wrapping, GLint level)
    : Texture(GL_TEXTURE_2D, format, internalformat, dataType, filtering, swizzleMask,
              std::span(wrapping), level)
    , dimensions_(dimensions) {}

Texture2D::Texture2D(const Texture2D& rhs) : Texture(rhs), dimensions_(rhs.dimensions_) {
    initialize(nullptr);
    if (OpenGLCapabilities::getOpenGLVersion() >= 430) {  // GPU memcpy
        std::scoped_lock lock{syncMutex};
        if (syncObj != 0) {
            glDeleteSync(syncObj);
            syncObj = 0;
        }

        glCopyImageSubData(rhs.getID(), rhs.getTarget(), 0, 0, 0, 0, getID(), target_, 0, 0, 0, 0,
                           static_cast<GLsizei>(dimensions_.x), static_cast<GLsizei>(dimensions_.y),
                           1);
        syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    } else {  // Copy data through PBO
        loadFromPBO(&rhs);
    }
}

Texture2D::Texture2D(Texture2D&& rhs) = default;

Texture2D& Texture2D::operator=(const Texture2D& rhs) {
    if (this != &rhs) {
        Texture::operator=(rhs);
        dimensions_ = rhs.dimensions_;
        initialize(nullptr);

        if (OpenGLCapabilities::getOpenGLVersion() >= 430) {  // GPU memcpy
            std::scoped_lock lock{syncMutex};
            if (syncObj != 0) {
                glDeleteSync(syncObj);
                syncObj = 0;
            }

            glCopyImageSubData(rhs.getID(), rhs.getTarget(), 0, 0, 0, 0, getID(), target_, 0, 0, 0,
                               0, static_cast<GLsizei>(rhs.dimensions_.x),
                               static_cast<GLsizei>(rhs.dimensions_.y), 1);
            syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        } else {  // Copy data through PBO
            loadFromPBO(&rhs);
        }
    }

    return *this;
}

Texture2D& Texture2D::operator=(Texture2D&& rhs) = default;

Texture2D::~Texture2D() { resource::remove(resource::GL{id_}); }

Texture2D* Texture2D::clone() const { return new Texture2D(*this); }

void Texture2D::initialize(const void* data) {
    // Notify observers
    forEachObserver([](TextureObserver* o) { o->notifyBeforeTextureInitialization(); });

    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, level_, internalformat_, static_cast<GLsizei>(dimensions_.x),
                 static_cast<GLsizei>(dimensions_.y), 0, format_, dataType_, data);
    LGL_ERROR;
    forEachObserver([](TextureObserver* o) { o->notifyAfterTextureInitialization(); });

    auto old = resource::remove(resource::GL{id_});
    resource::add(resource::GL{id_}, Resource{.dims = glm::size4_t{dimensions_, 0, 0},
                                              .format = getDataFormat()->getId(),
                                              .desc = "Texture2D",
                                              .meta = resource::getMeta(old)});
}

size_t Texture2D::getNumberOfValues() const { return dimensions_.x * dimensions_.y; }

void Texture2D::upload(const void* data) {
    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, static_cast<GLsizei>(dimensions_.x),
                    static_cast<GLsizei>(dimensions_.y), format_, dataType_, data);
    LGL_ERROR_CLASS;
}

void Texture2D::setWrapping(const std::array<GLenum, 2>& wrapping) {
    Texture::setWrapping(std::span(wrapping));
}

std::array<GLenum, 2> Texture2D::getWrapping() const {
    std::array<GLenum, 2> wrapping{};
    Texture::getWrapping(std::span(wrapping));
    return wrapping;
}

void Texture2D::resize(size2_t dimensions) {
    if (dimensions_ != dimensions) {
        dimensions_ = dimensions;
        setPBOAsInvalid();
        initialize(nullptr);
    }
}

}  // namespace inviwo
