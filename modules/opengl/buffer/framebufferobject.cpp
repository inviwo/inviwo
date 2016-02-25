/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include "framebufferobject.h"

namespace inviwo {

const GLenum FrameBufferObject::colorAttachmentEnums_[] = {
    GL_COLOR_ATTACHMENT0_EXT,  GL_COLOR_ATTACHMENT1_EXT,  GL_COLOR_ATTACHMENT2_EXT,
    GL_COLOR_ATTACHMENT3_EXT,  GL_COLOR_ATTACHMENT4_EXT,  GL_COLOR_ATTACHMENT5_EXT,
    GL_COLOR_ATTACHMENT6_EXT,  GL_COLOR_ATTACHMENT7_EXT,  GL_COLOR_ATTACHMENT8_EXT,
    GL_COLOR_ATTACHMENT9_EXT,  GL_COLOR_ATTACHMENT10_EXT, GL_COLOR_ATTACHMENT11_EXT,
    GL_COLOR_ATTACHMENT12_EXT, GL_COLOR_ATTACHMENT13_EXT, GL_COLOR_ATTACHMENT14_EXT,
    GL_COLOR_ATTACHMENT15_EXT};

FrameBufferObject::FrameBufferObject()
    : id_(0u)
    , hasDepthAttachment_(false)
    , hasStencilAttachment_(false)
    , maxColorattachments_(0)
    , prevFbo_(0u)
    , prevDrawFbo_(0u)
    , prevReadFbo_(0u) {
    glGenFramebuffersEXT(1, &id_);
    
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &maxColorattachments_);

    drawBuffers_.reserve(maxColorattachments_);
    buffersInUse_.resize(maxColorattachments_, false);
}

FrameBufferObject::~FrameBufferObject() {
    deactivate();
    glDeleteFramebuffersEXT(1, &id_);
}

void FrameBufferObject::activate() {
    // store currently bound FBO
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &prevFbo_);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id_);
    LGL_ERROR;
}

void FrameBufferObject::defineDrawBuffers() {
    // TODO: how to handle empty drawBuffers_ ? Do nothing or activate GL_COLOR_ATTACHMENT0 ?
    if (drawBuffers_.empty()) return;
    glDrawBuffers(static_cast<GLsizei>(drawBuffers_.size()), &drawBuffers_[0]);
    LGL_ERROR;
}

void FrameBufferObject::deactivate() {
    GLint currentFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFbo);
    if (currentFbo == static_cast<GLint>(id_)) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, prevFbo_);
        LGL_ERROR;
    }
}

void FrameBufferObject::deactivateFBO() { glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); }

/******************************* 2D Texture *****************************************/

void FrameBufferObject::attachTexture(Texture2D* texture, GLenum attachmentID) {
    performAttachTexture(attachmentID);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachmentID, GL_TEXTURE_2D, texture->getID(), 0);
}

GLenum FrameBufferObject::attachColorTexture(Texture2D* texture) {
    GLenum attachmentID;

    if (performAttachColorTexture(attachmentID)) {
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachmentID, GL_TEXTURE_2D, texture->getID(),
                                  0);
    }

    return attachmentID;
}

GLenum FrameBufferObject::attachColorTexture(Texture2D* texture, int attachmentNumber,
                                             bool attachFromRear, int forcedLocation) {
    GLenum attachmentID;

    if (performAttachColorTexture(attachmentID, attachmentNumber, attachFromRear, forcedLocation)) {
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachmentID, GL_TEXTURE_2D, texture->getID(),
                                  0);
    }

    return attachmentID;
}

/******************************* 2D Array Texture *****************************************/

void FrameBufferObject::attachTexture(Texture2DArray* texture, GLenum attachmentID) {
    performAttachTexture(attachmentID);
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, attachmentID, texture->getID(), 0);
}

GLenum FrameBufferObject::attachColorTexture(Texture2DArray* texture) {
    GLenum attachmentID;

    if (performAttachColorTexture(attachmentID)) {
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, attachmentID, texture->getID(), 0);
    }

    return attachmentID;
}

GLenum FrameBufferObject::attachColorTexture(Texture2DArray* texture, int attachmentNumber,
                                             bool attachFromRear, int forcedLocation) {
    GLenum attachmentID;

    if (performAttachColorTexture(attachmentID, attachmentNumber, attachFromRear, forcedLocation)) {
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, attachmentID, texture->getID(), 0);
    }

    return attachmentID;
}

void FrameBufferObject::attachTextureLayer(Texture2DArray* texture, GLenum attachmentID,
                                           int layer, int level) {
    performAttachTexture(attachmentID);
    glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, attachmentID, texture->getID(), level, layer);
}

GLenum FrameBufferObject::attachColorTextureLayer(Texture2DArray* texture, int layer) {
    GLenum attachmentID;

    if (performAttachColorTexture(attachmentID)) {
        glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, attachmentID, texture->getID(), 0, layer);
    }

    return attachmentID;
}

GLenum FrameBufferObject::attachColorTextureLayer(Texture2DArray* texture, int attachmentNumber,
                                                  int layer, bool attachFromRear,
                                                  int forcedLocation) {
    GLenum attachmentID;

    if (performAttachColorTexture(attachmentID, attachmentNumber, attachFromRear, forcedLocation)) {
        glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, attachmentID, texture->getID(), 0, layer);
    }

    return attachmentID;
}

/******************************* 3D Texture *****************************************/

void FrameBufferObject::attachTexture(Texture3D* texture, GLenum attachmentID) {
    performAttachTexture(attachmentID);
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, attachmentID, texture->getID(), 0);
}

GLenum FrameBufferObject::attachColorTexture(Texture3D* texture) {
    GLenum attachmentID;

    if (performAttachColorTexture(attachmentID)) {
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, attachmentID, texture->getID(), 0);
    }

    return attachmentID;
}

GLenum FrameBufferObject::attachColorTexture(Texture3D* texture, int attachmentNumber,
                                             bool attachFromRear, int forcedLocation) {
    GLenum attachmentID;

    if (performAttachColorTexture(attachmentID, attachmentNumber, attachFromRear, forcedLocation)) {
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, attachmentID, texture->getID(), 0);
    }

    return attachmentID;
}

void FrameBufferObject::attachTextureLayer(Texture3D* texture, GLenum attachmentID, int layer) {
    performAttachTexture(attachmentID);
    glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, attachmentID, GL_TEXTURE_3D, texture->getID(), 0,
                              layer);
}

GLenum FrameBufferObject::attachColorTextureLayer(Texture3D* texture, int layer) {
    GLenum attachmentID;

    if (performAttachColorTexture(attachmentID)) {
        glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, attachmentID, GL_TEXTURE_3D, texture->getID(),
                                  0, layer);
    }

    return attachmentID;
}

GLenum FrameBufferObject::attachColorTextureLayer(Texture3D* texture, int attachmentNumber,
                                                  int layer, bool attachFromRear,
                                                  int forcedLocation) {
    GLenum attachmentID;

    if (performAttachColorTexture(attachmentID, attachmentNumber, attachFromRear, forcedLocation)) {
        glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, attachmentID, GL_TEXTURE_3D, texture->getID(),
                                  0, layer);
    }

    return attachmentID;
}

void FrameBufferObject::detachTexture(GLenum attachmentID) {
    if (attachmentID == GL_DEPTH_ATTACHMENT)
        hasDepthAttachment_ = false;
    else if (attachmentID == GL_STENCIL_ATTACHMENT)
        hasStencilAttachment_ = false;
    else {
        std::vector<GLenum>::iterator it = drawBuffers_.begin();
        while (it != drawBuffers_.end()) {
            if (*it == attachmentID) break;
            ++it;
        }
        if (it == drawBuffers_.end()) {
            LogError("Could not detach " << attachmentID << " from framebuffer");
            return;
        }
        drawBuffers_.erase(it);
    }

    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, attachmentID, 0, 0);
}

void FrameBufferObject::detachAllTextures() {
    detachTexture(GL_DEPTH_ATTACHMENT);
    detachTexture(GL_STENCIL_ATTACHMENT);

    std::vector<GLenum>::iterator it = drawBuffers_.begin();
    while (it != drawBuffers_.end()) {
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, *it, 0, 0);
        ++it;
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
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

    switch (status) {
        case GL_FRAMEBUFFER_UNDEFINED:
            LogWarn("GL_FRAMEBUFFER_UNDEFINED");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            LogWarn("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            LogWarn("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            LogWarn("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            LogWarn("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
            break;

        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            LogWarn("GL_FRAMEBUFFER_UNSUPPORTED");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            LogWarn("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            LogWarn("GL_FRAMEBUFFER_INCOMPLETE_FORMATS");
            break;

        default:
            break;
    }
}

void FrameBufferObject::setRead_Blit(bool set) const {
    if (set) {
        // store currently bound draw FBO
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &prevReadFbo_);
        glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, id_);
    } else {
        GLint currentReadFbo;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &currentReadFbo);
        if (currentReadFbo == prevReadFbo_) {
            glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, prevFbo_);
        }
    }
}

void FrameBufferObject::setDraw_Blit(bool set) {
    if (set) {
        // store currently bound draw FBO
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &prevDrawFbo_);
        glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, id_);
    } else {
        GLint currentDrawFbo;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &currentDrawFbo);
        if (currentDrawFbo == prevDrawFbo_) {
            glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, prevFbo_);
        }
    }
}

void FrameBufferObject::performAttachTexture(GLenum attachmentID) {
    if (attachmentID == GL_DEPTH_ATTACHMENT)
        hasDepthAttachment_ = true;
    else if (attachmentID == GL_STENCIL_ATTACHMENT)
        hasStencilAttachment_ = true;
    else {
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
    if (drawBuffers_.empty()) {
        // no draw buffers attached as of now, use first attachment
        outAttachNumber = colorAttachmentEnums_[0];
        drawBuffers_.push_back(colorAttachmentEnums_[0]);
        buffersInUse_[outAttachNumber - colorAttachmentEnums_[0]] = true;
        return true;
    } else if (static_cast<long>(drawBuffers_.size()) == maxColorattachments_) {
        // cannot attach texture, maximum number of color attachments reached
        LogError("Maximum number of color attachments reached.");
        outAttachNumber = GL_NONE;
        return false;
    }

    // identify first unused color attachment ID
    std::vector<bool>::iterator itUsed = buffersInUse_.begin();
    while (itUsed != buffersInUse_.end()) {
        if (!*itUsed) break;
        ++itUsed;
    }

    // check for (itUsed == buffersUsed.end()) unnecessary, since already
    // handled above (drawBuffers_.size() == maxColorattachments_)
    if (itUsed == buffersInUse_.end()) {
        LogError("invalid state? (FBO " << id_ << ")");
    }
    GLenum target = static_cast<GLenum>(colorAttachmentEnums_[0] +
                                        std::distance(buffersInUse_.begin(), itUsed));
    drawBuffers_.push_back(target);
    outAttachNumber = target;
    buffersInUse_[target - colorAttachmentEnums_[0]] = true;

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
    if ((attachmentID == GL_DEPTH_ATTACHMENT) || (attachmentID == GL_STENCIL_ATTACHMENT)) return 0;

    std::vector<GLenum>::const_iterator it = drawBuffers_.begin();
    while (it != drawBuffers_.end()) {
        if (*it == attachmentID) return static_cast<int>(std::distance(drawBuffers_.begin(), it));
        ++it;
    }
    // given ID not attached
    return -1;
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

}  // namespace
