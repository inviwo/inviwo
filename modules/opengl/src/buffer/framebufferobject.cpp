/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/util/assertion.h>

#define IS_ACTIVE_CHECK_ATTACH IVW_ASSERT(isActive(), "FBO not active when attaching texture")
#define IS_ACTIVE_CHECK_DETACH IVW_ASSERT(isActive(), "FBO not active when detaching texture")

namespace inviwo {

const std::array<GLenum, 16> FrameBufferObject::colorAttachmentEnums_ = {
    GL_COLOR_ATTACHMENT0,  GL_COLOR_ATTACHMENT1,  GL_COLOR_ATTACHMENT2,  GL_COLOR_ATTACHMENT3,
    GL_COLOR_ATTACHMENT4,  GL_COLOR_ATTACHMENT5,  GL_COLOR_ATTACHMENT6,  GL_COLOR_ATTACHMENT7,
    GL_COLOR_ATTACHMENT8,  GL_COLOR_ATTACHMENT9,  GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11,
    GL_COLOR_ATTACHMENT12, GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14, GL_COLOR_ATTACHMENT15};

FrameBufferObject::FrameBufferObject()
    : id_(0u)
    , hasDepthAttachment_(false)
    , hasStencilAttachment_(false)
    , maxColorattachments_(0)
    , prevFbo_(0u)
    , prevDrawFbo_(0u)
    , prevReadFbo_(0u) {

    glGenFramebuffers(1, &id_);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorattachments_);

    drawBuffers_.reserve(maxColorattachments_);
    buffersInUse_.resize(maxColorattachments_, false);
}

FrameBufferObject::~FrameBufferObject() {
    deactivate();
    glDeleteFramebuffers(1, &id_);
}

void FrameBufferObject::activate() {
    // store currently bound FBO
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo_);

    glBindFramebuffer(GL_FRAMEBUFFER, id_);
    LGL_ERROR;
}

void FrameBufferObject::defineDrawBuffers() {
    // TODO: how to handle empty drawBuffers_ ? Do nothing or activate GL_COLOR_ATTACHMENT0 ?
    if (drawBuffers_.empty()) return;
    glDrawBuffers(static_cast<GLsizei>(drawBuffers_.size()), &drawBuffers_[0]);
    LGL_ERROR;
}

void FrameBufferObject::deactivate() {
    if (isActive()) {
        glBindFramebuffer(GL_FRAMEBUFFER, prevFbo_);
        LGL_ERROR;
    }
}

bool FrameBufferObject::isActive() const {
    GLint currentFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);
    return (static_cast<GLint>(id_) == currentFbo);
}

void FrameBufferObject::deactivateFBO() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

/******************************* 2D Texture *****************************************/

void FrameBufferObject::attachTexture(Texture2D* texture, GLenum attachmentID) {
    IS_ACTIVE_CHECK_ATTACH;
    performAttachTexture(attachmentID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentID, GL_TEXTURE_2D, texture->getID(), 0);
}

GLenum FrameBufferObject::attachColorTexture(Texture2D* texture) {
    IS_ACTIVE_CHECK_ATTACH;
    GLenum attachmentID;
    if (performAttachColorTexture(attachmentID)) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentID, GL_TEXTURE_2D, texture->getID(), 0);
    }
    return attachmentID;
}

GLenum FrameBufferObject::attachColorTexture(Texture2D* texture, int attachmentNumber,
                                             bool attachFromRear, int forcedLocation) {
    IS_ACTIVE_CHECK_ATTACH;
    GLenum attachmentID;
    if (performAttachColorTexture(attachmentID, attachmentNumber, attachFromRear, forcedLocation)) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentID, GL_TEXTURE_2D, texture->getID(), 0);
    }
    return attachmentID;
}

/******************************* 2D Array Texture *****************************************/

void FrameBufferObject::attachTexture(Texture2DArray* texture, GLenum attachmentID) {
    IS_ACTIVE_CHECK_ATTACH;
    performAttachTexture(attachmentID);
    glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0);
}

GLenum FrameBufferObject::attachColorTexture(Texture2DArray* texture) {
    IS_ACTIVE_CHECK_ATTACH;
    GLenum attachmentID;
    if (performAttachColorTexture(attachmentID)) {
        glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0);
    }
    return attachmentID;
}

GLenum FrameBufferObject::attachColorTexture(Texture2DArray* texture, int attachmentNumber,
                                             bool attachFromRear, int forcedLocation) {
    IS_ACTIVE_CHECK_ATTACH;
    GLenum attachmentID;
    if (performAttachColorTexture(attachmentID, attachmentNumber, attachFromRear, forcedLocation)) {
        glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0);
    }
    return attachmentID;
}

void FrameBufferObject::attachTextureLayer(Texture2DArray* texture, GLenum attachmentID, int layer,
                                           int level) {
    IS_ACTIVE_CHECK_ATTACH;
    performAttachTexture(attachmentID);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentID, texture->getID(), level, layer);
}

GLenum FrameBufferObject::attachColorTextureLayer(Texture2DArray* texture, int layer) {
    IS_ACTIVE_CHECK_ATTACH;
    GLenum attachmentID;
    if (performAttachColorTexture(attachmentID)) {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0, layer);
    }
    return attachmentID;
}

GLenum FrameBufferObject::attachColorTextureLayer(Texture2DArray* texture, int attachmentNumber,
                                                  int layer, bool attachFromRear,
                                                  int forcedLocation) {
    IS_ACTIVE_CHECK_ATTACH;
    GLenum attachmentID;
    if (performAttachColorTexture(attachmentID, attachmentNumber, attachFromRear, forcedLocation)) {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0, layer);
    }
    return attachmentID;
}

/******************************* 3D Texture *****************************************/

void FrameBufferObject::attachTexture(Texture3D* texture, GLenum attachmentID) {
    IS_ACTIVE_CHECK_ATTACH;
    performAttachTexture(attachmentID);
    glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0);
}

GLenum FrameBufferObject::attachColorTexture(Texture3D* texture) {
    IS_ACTIVE_CHECK_ATTACH;
    GLenum attachmentID;
    if (performAttachColorTexture(attachmentID)) {
        glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0);
    }
    return attachmentID;
}

GLenum FrameBufferObject::attachColorTexture(Texture3D* texture, int attachmentNumber,
                                             bool attachFromRear, int forcedLocation) {
    IS_ACTIVE_CHECK_ATTACH;
    GLenum attachmentID;
    if (performAttachColorTexture(attachmentID, attachmentNumber, attachFromRear, forcedLocation)) {
        glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, texture->getID(), 0);
    }
    return attachmentID;
}

void FrameBufferObject::attachTextureLayer(Texture3D* texture, GLenum attachmentID, int layer) {
    IS_ACTIVE_CHECK_ATTACH;
    performAttachTexture(attachmentID);
    glFramebufferTexture3D(GL_FRAMEBUFFER, attachmentID, GL_TEXTURE_3D, texture->getID(), 0, layer);
}

GLenum FrameBufferObject::attachColorTextureLayer(Texture3D* texture, int layer) {
    IS_ACTIVE_CHECK_ATTACH;
    GLenum attachmentID;
    if (performAttachColorTexture(attachmentID)) {
        glFramebufferTexture3D(GL_FRAMEBUFFER, attachmentID, GL_TEXTURE_3D, texture->getID(), 0,
                               layer);
    }
    return attachmentID;
}

GLenum FrameBufferObject::attachColorTextureLayer(Texture3D* texture, int attachmentNumber,
                                                  int layer, bool attachFromRear,
                                                  int forcedLocation) {
    IS_ACTIVE_CHECK_ATTACH;
    GLenum attachmentID;
    if (performAttachColorTexture(attachmentID, attachmentNumber, attachFromRear, forcedLocation)) {
        glFramebufferTexture3D(GL_FRAMEBUFFER, attachmentID, GL_TEXTURE_3D, texture->getID(), 0,
                               layer);
    }
    return attachmentID;
}

void FrameBufferObject::detachTexture(GLenum attachmentID) {
    IS_ACTIVE_CHECK_DETACH;
    if (attachmentID == GL_DEPTH_ATTACHMENT) {
        hasDepthAttachment_ = false;
    } else if (attachmentID == GL_STENCIL_ATTACHMENT) {
        hasStencilAttachment_ = false;
    } else if (attachmentID == GL_DEPTH_STENCIL_ATTACHMENT) {
        hasDepthAttachment_ = false;
        hasStencilAttachment_ = false;
    } else {
        // check for valid attachmentID
        if ((attachmentID < colorAttachmentEnums_[0]) ||
            (attachmentID > colorAttachmentEnums_[0] + maxColorattachments_ - 1)) {
            LogError("Attachments ID " << attachmentID
                                       << " exceeds maximum amount of color attachments");
            return;
        }
        // check whether given ID is already attached

        // keep internal state consistent and remove color attachment from draw buffers
        util::erase_remove(drawBuffers_, attachmentID);

        // set attachment state to unused
        buffersInUse_[attachmentID - colorAttachmentEnums_[0]] = false;
    }

    glFramebufferTexture(GL_FRAMEBUFFER, attachmentID, 0, 0);
}

void FrameBufferObject::detachAllTextures() {
    IS_ACTIVE_CHECK_DETACH;
    detachTexture(GL_DEPTH_ATTACHMENT);
    detachTexture(GL_STENCIL_ATTACHMENT);

    for (const auto& buffer : drawBuffers_) {
        glFramebufferTexture(GL_FRAMEBUFFER, buffer, 0, 0);
    }
    drawBuffers_.clear();
    std::fill(buffersInUse_.begin(), buffersInUse_.end(), false);
}

unsigned int FrameBufferObject::getID() const { return id_; }

const GLenum* FrameBufferObject::getDrawBuffersDeprecated() const { return &drawBuffers_[0]; }

int FrameBufferObject::getMaxColorAttachments() const { return maxColorattachments_; }

bool FrameBufferObject::hasColorAttachment() const { return !drawBuffers_.empty(); }

bool FrameBufferObject::hasDepthAttachment() const { return hasDepthAttachment_; }

bool FrameBufferObject::hasStencilAttachment() const { return hasStencilAttachment_; }

void FrameBufferObject::checkStatus() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:  // All OK
            break;

        case GL_FRAMEBUFFER_UNDEFINED:
            LogWarn("GL_FRAMEBUFFER_UNDEFINED");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            LogWarn("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            LogWarn("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            LogWarn("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            LogWarn("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
            break;

        case GL_FRAMEBUFFER_UNSUPPORTED:
            LogWarn("GL_FRAMEBUFFER_UNSUPPORTED");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            LogWarn("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            LogWarn("GL_FRAMEBUFFER_INCOMPLETE_FORMATS");
            break;

        default:
            LogWarn("Unknown error " << status);
            break;
    }
}

void FrameBufferObject::setRead_Blit(bool set) const {
    if (set) {  // store currently bound draw FBO
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFbo_);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
    } else {
        GLint currentReadFbo;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &currentReadFbo);
        if (currentReadFbo == prevReadFbo_) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, prevFbo_);
        }
    }
}

void FrameBufferObject::setDraw_Blit(bool set) {
    if (set) {  // store currently bound draw FBO
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFbo_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id_);
    } else {
        GLint currentDrawFbo;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &currentDrawFbo);
        if (currentDrawFbo == prevDrawFbo_) {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevFbo_);
        }
    }
}

void FrameBufferObject::performAttachTexture(GLenum attachmentID) {
    if (attachmentID == GL_DEPTH_ATTACHMENT) {
        hasDepthAttachment_ = true;
    } else if (attachmentID == GL_STENCIL_ATTACHMENT) {
        hasStencilAttachment_ = true;
    } else if (attachmentID == GL_DEPTH_STENCIL_ATTACHMENT) {
        hasDepthAttachment_ = true;
        hasStencilAttachment_ = true;
    } else {
        // check for valid attachmentID
        if ((attachmentID < colorAttachmentEnums_[0]) ||
            (attachmentID > colorAttachmentEnums_[0] + maxColorattachments_ - 1)) {
            LogError("Attachments ID " << attachmentID
                                       << " exceeds maximum amount of color attachments");
            return;
        }
        // check whether given ID is already attached
        if (!buffersInUse_[attachmentID - colorAttachmentEnums_[0]]) {
            drawBuffers_.push_back(attachmentID);
            buffersInUse_[attachmentID - colorAttachmentEnums_[0]] = true;
        }
    }
}

bool FrameBufferObject::performAttachColorTexture(GLenum& outAttachNumber) {
    // identify first unused color attachment ID
    auto itUnUsed = util::find_if(buffersInUse_, [](const auto& used) { return !used; });

    if (itUnUsed == buffersInUse_.end()) {
        LogError("Maximum number of color attachments reached.");
        outAttachNumber = GL_NONE;
        return false;
    }
    GLenum target = static_cast<GLenum>(colorAttachmentEnums_[0] +
                                        std::distance(buffersInUse_.begin(), itUnUsed));
    drawBuffers_.push_back(target);
    *itUnUsed = true;
    outAttachNumber = target;
    return true;
}

bool FrameBufferObject::performAttachColorTexture(GLenum& outAttachNumber, int attachmentNumber,
                                                  bool attachFromRear, int forcedLocation) {
    if (static_cast<long>(drawBuffers_.size()) == maxColorattachments_) {
        LogError("Maximum number of color attachments reached.");
        outAttachNumber = GL_NONE;
        return false;
    } else if ((attachmentNumber < 0) || (attachmentNumber >= maxColorattachments_)) {
        LogError("Invalid attachment ID. " << attachmentNumber);
        outAttachNumber = GL_NONE;
        return false;
    }

    attachmentNumber =
        (attachFromRear ? maxColorattachments_ - attachmentNumber - 1 : attachmentNumber);
    GLenum attachmentID = static_cast<GLenum>(colorAttachmentEnums_[0] + attachmentNumber);
    if (!buffersInUse_[attachmentNumber]) {
        // new attachment, not registered before
        buffersInUse_[attachmentNumber] = true;
        if ((forcedLocation < 0) || (forcedLocation > static_cast<int>(drawBuffers_.size()))) {
            // no or invalid forced location
            drawBuffers_.push_back(attachmentID);
        } else {
            // forced location, position attachment at given position in drawBuffers_
            drawBuffers_.insert(drawBuffers_.begin() + forcedLocation, attachmentID);
        }
    } else if ((forcedLocation > -1) && (forcedLocation < static_cast<int>(drawBuffers_.size()))) {
        // attachment is already registered, but buffer location is forced.
        // adjust position within drawBuffers_ only if required
        if (drawBuffers_[forcedLocation] != attachmentID) {
            std::vector<GLenum>::iterator it = drawBuffers_.begin();
            while ((*it != attachmentID) && (it != drawBuffers_.end())) {
                ++it;
            }
            drawBuffers_.erase(it);
            drawBuffers_.insert(drawBuffers_.begin() + forcedLocation, attachmentID);
        }
    }

    outAttachNumber = attachmentID;
    return true;
}

int FrameBufferObject::getAttachmentLocation(GLenum attachmentID) const {
    if ((attachmentID == GL_DEPTH_ATTACHMENT) || (attachmentID == GL_STENCIL_ATTACHMENT) ||
        (attachmentID == GL_DEPTH_STENCIL_ATTACHMENT))
        return 0;

    auto it = util::find_if(drawBuffers_, [&](const auto& b) { return b == attachmentID; });
    if (it != drawBuffers_.end()) {
        return static_cast<int>(std::distance(drawBuffers_.begin(), it));
    } else {
        // given ID not attached
        return -1;
    }
}

std::string FrameBufferObject::printBuffers() const {
    std::stringstream str;
    if (drawBuffers_.empty()) {
        str << "none";
    } else {
        for (std::size_t i = 0; i < drawBuffers_.size(); ++i) {
            str << ((i != 0) ? ", " : "") << getAttachmentStr(drawBuffers_[i]);
        }
    }
    str << " / " << std::count(buffersInUse_.begin(), buffersInUse_.end(), true)
        << " buffers active";

    return str.str();
}

std::string FrameBufferObject::getAttachmentStr(GLenum attachmentID) {
    if ((attachmentID < GL_COLOR_ATTACHMENT0) || (attachmentID > GL_COLOR_ATTACHMENT15))
        return "GL_NONE";

    std::stringstream str;
    str << "GL_COLOR_ATTACHMENT" << (attachmentID - colorAttachmentEnums_[0]);
    return str.str();
}

}  // namespace inviwo
