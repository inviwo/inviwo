/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include <modules/opengl/buffer/framebufferobject.h>

#include <inviwo/core/util/assertion.h>             // for IVW_ASSERT, assertion, assertions
#include <inviwo/core/util/canvas.h>                // for Canvas, Canvas::ContextID
#include <inviwo/core/util/rendercontext.h>         // for RenderContext
#include <inviwo/core/util/sourcecontext.h>         // for IVW_SOURCE_LOCATION, SourceLocation
#include <inviwo/core/util/stdextensions.h>         // for any_of
#include <modules/opengl/openglexception.h>         // for OpenGLException
#include <modules/opengl/texture/texture2d.h>       // for Texture2D
#include <modules/opengl/texture/texture2darray.h>  // for Texture2DArray
#include <modules/opengl/texture/texture3d.h>       // for Texture3D

#include <algorithm>    // for find
#include <chrono>       // for literals
#include <iterator>     // for distance
#include <string>       // for basic_string
#include <string_view>  // for operator""sv, string_view
#include <utility>      // for move

#include <fmt/core.h>  // for format

#include <inviwo/tracy/tracy.h>

namespace inviwo {

using namespace std::literals;

inline void checkContext(std::string_view error, Canvas::ContextID org, SourceLocation loc) {
    if constexpr (cfg::assertions) {
        auto rc = RenderContext::getPtr();
        Canvas::ContextID curr = rc->activeContext();
        if (org != curr) {

            const auto message =
                fmt::format("{}: '{}' ({}) than it was created: '{}' ({})", error,
                            rc->getContextName(curr), curr, rc->getContextName(org), org);

            assertion(loc.getFile(), loc.getFunction(), loc.getLine(), message);
        }
    }
}

GLuint FrameBufferObject::maxColorAttachments() {
    static GLint max = []() {
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max);
        return static_cast<GLuint>(max);
    }();
    return max;
}

FrameBufferObject::FrameBufferObject()
    : id_(0u), attachedDepthId_(0), attachedStencilId_(0), attachedColorIds_{} {

    creationContext_ = RenderContext::getPtr()->activeContext();
    IVW_ASSERT(creationContext_, "An OpenGL Context has to be active");

    glGenFramebuffers(1, &id_);

    attachedColorIds_.resize(maxColorAttachments(), 0);
}

FrameBufferObject::FrameBufferObject(FrameBufferObject&& rhs) noexcept
    : id_(rhs.id_)
    , attachedDepthId_{rhs.attachedDepthId_}
    , attachedStencilId_{rhs.attachedStencilId_}
    , attachedColorIds_{std::move(rhs.attachedColorIds_)}
    , creationContext_{rhs.creationContext_} {

    rhs.id_ = 0;
}

FrameBufferObject& FrameBufferObject::operator=(FrameBufferObject&& rhs) noexcept {
    if (this != &rhs) {
        if (id_ != 0) {
            checkContext("FBO deleted in a different context"sv, creationContext_,
                         IVW_SOURCE_LOCATION);
            glDeleteFramebuffers(1, &id_);
        }

        id_ = rhs.id_;
        attachedDepthId_ = rhs.attachedDepthId_;
        attachedStencilId_ = rhs.attachedStencilId_;
        attachedColorIds_ = std::move(rhs.attachedColorIds_);
        creationContext_ = rhs.creationContext_;

        rhs.id_ = 0;
    }
    return *this;
}

FrameBufferObject::~FrameBufferObject() {
    if (id_ != 0) {
        checkContext("FBO deleted in a different context"sv, creationContext_, IVW_SOURCE_LOCATION);
        glDeleteFramebuffers(1, &id_);
    }
}

void FrameBufferObject::activate() {
    checkContext("FBO activated in a different context"sv, creationContext_, IVW_SOURCE_LOCATION);
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
}

void FrameBufferObject::deactivate() { deactivateFBO(); }

void FrameBufferObject::deactivateFBO() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

bool FrameBufferObject::isActive() const {
    checkContext("FBO used in a different context"sv, creationContext_, IVW_SOURCE_LOCATION);
    GLint currentFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);
    return (static_cast<GLint>(id_) == currentFbo);
}

GLenum FrameBufferObject::status() const {
    IVW_ASSERT(isActive(), "FBO not active when checking status");
    return glCheckFramebufferStatus(GL_FRAMEBUFFER);
}

/******************************* Attachments *****************************************/

void FrameBufferObject::registerAttachment(GLenum attachmentID, GLuint texId) {
    if (attachmentID == GL_DEPTH_ATTACHMENT) {
        attachedDepthId_ = texId;
    } else if (attachmentID == GL_STENCIL_ATTACHMENT) {
        attachedStencilId_ = texId;
    } else if (attachmentID == GL_DEPTH_STENCIL_ATTACHMENT) {
        attachedDepthId_ = texId;
        attachedStencilId_ = texId;
    } else {
        const auto num = enumToNumber(attachmentID);
        if (num >= maxColorAttachments()) {
            throw OpenGLException(IVW_CONTEXT, "Invalid attachment id: {}", attachmentID);
        }
        attachedColorIds_[num] = texId;
    }
}

void FrameBufferObject::deregisterAttachment(GLenum attachmentID) {
    registerAttachment(attachmentID, 0);
}

GLenum FrameBufferObject::firstFreeAttachmentID() const {
    const auto it = std::find(attachedColorIds_.begin(), attachedColorIds_.end(), GLenum{0});
    if (it == attachedColorIds_.end()) {
        throw OpenGLException("Maximum number of color attachments reached.", IVW_CONTEXT);
    }
    return numberToEnum(static_cast<GLuint>(std::distance(attachedColorIds_.begin(), it)));
}

/******************************* 2D Texture *****************************************/

void FrameBufferObject::attachTexture(Texture2D* texture, GLenum attachmentID) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentID, GL_TEXTURE_2D, texture->getID(), 0);
}

GLenum FrameBufferObject::attachColorTexture(Texture2D* texture) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    const auto attachmentID = firstFreeAttachmentID();
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentID, GL_TEXTURE_2D, texture->getID(), 0);
    return attachmentID;
}

GLenum FrameBufferObject::attachColorTexture(Texture2D* texture, int attachmentNumber) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    GLenum attachmentID = numberToEnum(attachmentNumber);
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentID, GL_TEXTURE_2D, texture->getID(), 0);
    return attachmentID;
}

/******************************* 2D Array Texture *****************************************/

void FrameBufferObject::attachTexture(Texture2DArray* texture, GLenum attachmentID) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0);
}

GLenum FrameBufferObject::attachColorTexture(Texture2DArray* texture) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    const auto attachmentID = firstFreeAttachmentID();
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0);
    return attachmentID;
}

GLenum FrameBufferObject::attachColorTexture(Texture2DArray* texture, int attachmentNumber) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    GLenum attachmentID = numberToEnum(attachmentNumber);
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0);
    return attachmentID;
}

void FrameBufferObject::attachTextureLayer(Texture2DArray* texture, GLenum attachmentID, int layer,
                                           int level) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentID, texture->getID(), level, layer);
}

GLenum FrameBufferObject::attachColorTextureLayer(Texture2DArray* texture, int layer) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    const auto attachmentID = firstFreeAttachmentID();
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0, layer);
    return attachmentID;
}

GLenum FrameBufferObject::attachColorTextureLayer(Texture2DArray* texture, int attachmentNumber,
                                                  int layer) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    GLenum attachmentID = numberToEnum(attachmentNumber);
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0, layer);

    return attachmentID;
}

/******************************* 3D Texture *****************************************/

void FrameBufferObject::attachTexture(Texture3D* texture, GLenum attachmentID) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0);
}

GLenum FrameBufferObject::attachColorTexture(Texture3D* texture) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    const auto attachmentID = firstFreeAttachmentID();
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0);
    return attachmentID;
}

GLenum FrameBufferObject::attachColorTexture(Texture3D* texture, int attachmentNumber) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    GLenum attachmentID = numberToEnum(attachmentNumber);
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0);

    return attachmentID;
}

void FrameBufferObject::attachTextureLayer(Texture3D* texture, GLenum attachmentID, int layer) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0, layer);
}

GLenum FrameBufferObject::attachColorTextureLayer(Texture3D* texture, int layer) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    const auto attachmentID = firstFreeAttachmentID();
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0, layer);
    return attachmentID;
}

GLenum FrameBufferObject::attachColorTextureLayer(Texture3D* texture, int attachmentNumber,
                                                  int layer) {
    IVW_ASSERT(isActive(), "FBO not active when attaching texture");
    GLenum attachmentID = numberToEnum(attachmentNumber);
    registerAttachment(attachmentID, texture->getID());
    glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0, layer);
    return attachmentID;
}

void FrameBufferObject::detachTexture(GLenum attachmentID) {
    IVW_ASSERT(isActive(), "FBO not active when detaching texture");
    deregisterAttachment(attachmentID);
    glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, 0, 0);
}

void FrameBufferObject::detachAllTextures() {
    IVW_ASSERT(isActive(), "FBO not active when detaching texture");

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0);
    attachedDepthId_ = 0;
    attachedStencilId_ = 0;

    for (GLuint num = 0; num < attachedColorIds_.size(); ++num) {
        if (attachedColorIds_[num] != 0) {
            glFramebufferTexture(GL_FRAMEBUFFER, numberToEnum(num), 0, 0);
            attachedColorIds_[num] = 0;
        }
    }
}

unsigned int FrameBufferObject::getID() const { return id_; }

bool FrameBufferObject::hasColorAttachment() const {
    return util::any_of(attachedColorIds_, [](auto id) { return id != 0; });
}

bool FrameBufferObject::hasDepthAttachment() const { return attachedDepthId_ != 0; }

bool FrameBufferObject::hasStencilAttachment() const { return attachedStencilId_ != 0; }

void FrameBufferObject::setReadBlit(bool set) const {
    if (set) {  // store currently bound draw FBO
        checkContext("FBO activated in a different context"sv, creationContext_,
                     IVW_SOURCE_LOCATION);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
    } else {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }
}

void FrameBufferObject::setDrawBlit(bool set) {
    if (set) {  // store currently bound draw FBO
        checkContext("FBO activated in a different context"sv, creationContext_,
                     IVW_SOURCE_LOCATION);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id_);
    } else {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
}

std::string_view utilgl::framebufferStatusToString(GLenum status) {
    using namespace std::literals;
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            return "Complete"sv;
        case GL_FRAMEBUFFER_UNDEFINED:
            return "Undefined"sv;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "Incomplete Attachment"sv;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "Incomplete Missing Attachment"sv;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            return "Incomplete Draw Buffer"sv;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            return "Incomplete Read Buffer"sv;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "Unsupported"sv;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "Incomplete Multisample"sv;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            return "Incomplete Formats"sv;
        default:
            return "Unknown Error"sv;
    }
}

}  // namespace inviwo
