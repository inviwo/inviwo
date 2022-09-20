/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <modules/opengl/texture/texture3d.h>

#include <inviwo/core/datastructures/image/imagetypes.h>  // for SwizzleMask
#include <inviwo/core/util/glmvec.h>                      // for size3_t
#include <modules/opengl/glformats.h>                     // for GLFormat
#include <modules/opengl/inviwoopengl.h>                  // for GLenum, GLsizei, glPixelStorei
#include <modules/opengl/openglcapabilities.h>            // for OpenGLCapabilities
#include <modules/opengl/texture/texture.h>               // for Texture
#include <modules/opengl/texture/textureobserver.h>       // for TextureObserver

#include <mutex>                                          // for scoped_lock
#include <vector>                                         // for vector

#include <glm/vec3.hpp>                                   // for vec<>::(anonymous), operator!=
#include <tcb/span.hpp>                                   // for span

namespace inviwo {

Texture3D::Texture3D(size3_t dimensions, GLFormat glFormat, GLenum filtering,
                     const SwizzleMask& swizzleMask, const std::array<GLenum, 3>& wrapping,
                     GLint level)
    : Texture(GL_TEXTURE_3D, glFormat, filtering, swizzleMask, util::span(wrapping), level)
    , dimensions_(dimensions) {}

Texture3D::Texture3D(size3_t dimensions, GLint format, GLint internalformat, GLenum dataType,
                     GLenum filtering, const SwizzleMask& swizzleMask,
                     const std::array<GLenum, 3>& wrapping, GLint level)
    : Texture(GL_TEXTURE_3D, format, internalformat, dataType, filtering, swizzleMask,
              util::span(wrapping), level)
    , dimensions_(dimensions) {}

Texture3D::Texture3D(const Texture3D& rhs) : Texture(rhs), dimensions_(rhs.dimensions_) {
    initialize(nullptr);
    if (OpenGLCapabilities::getOpenGLVersion() >= 430) {  // GPU memcpy
        std::scoped_lock lock{syncMutex};
        if (syncObj != 0) {
            glDeleteSync(syncObj);
            syncObj = 0;
        }

        glCopyImageSubData(rhs.getID(), rhs.getTarget(), 0, 0, 0, 0, getID(), target_, 0, 0, 0, 0,
                           static_cast<GLsizei>(dimensions_.x), static_cast<GLsizei>(dimensions_.y),
                           static_cast<GLsizei>(dimensions_.z));

        syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    } else {  // Copy data through PBO
        loadFromPBO(&rhs);
    }
}

Texture3D::Texture3D(Texture3D&& rhs) = default;

Texture3D& Texture3D::operator=(const Texture3D& rhs) {
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
                               static_cast<GLsizei>(rhs.dimensions_.y),
                               static_cast<GLsizei>(rhs.dimensions_.z));

            syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        } else {  // Copy data through PBO
            loadFromPBO(&rhs);
        }
    }

    return *this;
}

Texture3D& Texture3D::operator=(Texture3D&& rhs) = default;

Texture3D* Texture3D::clone() const { return new Texture3D(*this); }

void Texture3D::initialize(const void* data) {
    // Notify observers
    forEachObserver([](TextureObserver* o) { o->notifyBeforeTextureInitialization(); });

    // Allocate data
    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage3D(GL_TEXTURE_3D, level_, internalformat_, static_cast<GLsizei>(dimensions_.x),
                 static_cast<GLsizei>(dimensions_.y), static_cast<GLsizei>(dimensions_.z), 0,
                 format_, dataType_, data);
    LGL_ERROR;
    forEachObserver([](TextureObserver* o) { o->notifyAfterTextureInitialization(); });
}

size_t Texture3D::getNumberOfValues() const {
    return dimensions_.x * dimensions_.y * dimensions_.z;
}

void Texture3D::upload(const void* data) {
    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, static_cast<GLsizei>(dimensions_.x),
                    static_cast<GLsizei>(dimensions_.y), static_cast<GLsizei>(dimensions_.z),
                    format_, dataType_, data);
    LGL_ERROR;
}

void Texture3D::uploadAndResize(const void* data, const size3_t& dim) {
    if (dimensions_ != dim) {
        dimensions_ = dim;
        setPBOAsInvalid();
        initialize(data);
    }
}

void Texture3D::setWrapping(const std::array<GLenum, 3>& wrapping) {
    Texture::setWrapping(util::span(wrapping));
}

std::array<GLenum, 3> Texture3D::getWrapping() const {
    std::array<GLenum, 3> wrapping{};
    Texture::getWrapping(util::span(wrapping));
    return wrapping;
}

}  // namespace inviwo
