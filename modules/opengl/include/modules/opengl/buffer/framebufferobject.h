/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/texture2darray.h>
#include <modules/opengl/texture/texture3d.h>
#include <inviwo/core/util/rendercontext.h>
#include <vector>
#include <optional>

namespace inviwo {

/**
 * @brief Inviwo framebuffer wrapper.
 *
 * Handles creation and deletion of OpenGL framebuffer objects. Has functions for attachment and
 * detachment of textures to the framebuffer. It Also keeps track of all attached texture ids. The
 * wapper is a move only type.
 */
class IVW_MODULE_OPENGL_API FrameBufferObject {
public:
    /**
     * @brief Create a new framebuffer object
     */
    FrameBufferObject();
    FrameBufferObject(const FrameBufferObject&) = delete;
    FrameBufferObject(FrameBufferObject&& rhs) noexcept;
    FrameBufferObject& operator=(const FrameBufferObject&) = delete;
    FrameBufferObject& operator=(FrameBufferObject&& rhs) noexcept;
    ~FrameBufferObject();

    /**
     * @brief Get the framebuffer name
     */
    unsigned int getID() const;

    /**
     * @brief Binds the framebuffer
     */
    void activate();

    /**
     * @brief Unbind the framebuffer (binds id 0)
     */
    void deactivate();

    /**
     * @brief Unbind the framebuffer (binds id 0)
     */
    static void deactivateFBO();

    /**
     * @brief Check it this framebuffer is currently bound
     */
    bool isActive() const;

    /**
     * @brief Check the framebuffer status. Log a warning if it is not Complete.
     * @return true if the framebuffer is complete otherwise false.
     */
    bool checkStatus() const;

    /**
     * @brief Attach a 2D Texture to the framebuffer
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @param attachmentID attachment point to use (@c GL_COLOR_ATTACHMENT0, ... )
     */
    void attachTexture(Texture2D* texture, GLenum attachmentID);

    /**
     * @brief Attach a 2D Color Texture to the framebuffer.
     *
     * Will use the first available attachment point.
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @return The used attachment point (@c GL_COLOR_ATTACHMENT0, ... )
     */
    GLenum attachColorTexture(Texture2D* texture);

    /**
     * @brief Attach a 2D Color Texture to the framebuffer.
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @param attachmentNumber number of attachment point to use (0, 1, ...)
     * @return the attachment point used (GL_COLOR_ATTACHMENT0, ... )
     */
    GLenum attachColorTexture(Texture2D* texture, int attachmentNumber);

    /**
     * @brief Attach a 2D Texture Array to the framebuffer.
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @param attachmentID attachment point to use (@c GL_COLOR_ATTACHMENT0, ... )
     */
    void attachTexture(Texture2DArray* texture, GLenum attachmentID);

    /**
     * @brief Attach a 2D Color Texture Array to the framebuffer.
     *
     * Will use the first available attachment point.
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @return The used attachment point (@c GL_COLOR_ATTACHMENT0, ... )
     */
    GLenum attachColorTexture(Texture2DArray* texture);

    /**
     * @brief Attach a 2D Color Texture Array to the framebuffer.
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @param attachmentNumber number of attachment point to use (0, 1, ...)
     * @return the attachment point used (@c GL_COLOR_ATTACHMENT0, ... )
     */
    GLenum attachColorTexture(Texture2DArray* texture, int attachmentNumber);

    /**
     * @brief Attach a layer of 2D a Texture Array to the framebuffer.
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @param attachmentID attachment point to use (@c GL_COLOR_ATTACHMENT0, ... )
     * @param layer index of the layer to attach
     * @param level the mimpmap level to use defaults to 0
     */
    void attachTextureLayer(Texture2DArray* texture, GLenum attachmentID, int layer, int level = 0);

    /**
     * @brief Attach a layer of a 2D Color Texture Array to the framebuffer.
     *
     * Will use the first available attachment point.
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @param layer index of the layer to attach
     * @return The used attachment point (@c GL_COLOR_ATTACHMENT0, ... )
     */
    GLenum attachColorTextureLayer(Texture2DArray* texture, int layer);

    /**
     * @brief Attach a layer of a 2D Color Texture Array to the framebuffer.
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @param attachmentNumber number of attachment point to use (0, 1, ...)
     * @param layer index of the layer to attach
     * @return the attachment point used (@c GL_COLOR_ATTACHMENT0, ... )
     */
    GLenum attachColorTextureLayer(Texture2DArray* texture, int attachmentNumber, int layer);

    /**
     * @brief Attach a 3D Texture to the framebuffer
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @param attachmentID attachment point to use (@c GL_COLOR_ATTACHMENT0, ... )
     */
    void attachTexture(Texture3D* texture, GLenum attachmentID);

    /**
     * @brief Attach a 3D Color Texture to the framebuffer.
     *
     * Will use the first available attachment point.
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @return The used attachment point (@c GL_COLOR_ATTACHMENT0, ... )
     */
    GLenum attachColorTexture(Texture3D* texture);

    /**
     * @brief Attach a 3D Color Texture to the framebuffer.
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @param attachmentNumber number of attachment point to use (0, 1, ...)
     * @return the attachment point used (@c GL_COLOR_ATTACHMENT0, ... )
     */
    GLenum attachColorTexture(Texture3D* texture, int attachmentNumber);

    /**
     * @brief Attach a layer of a 3D Texture to the framebuffer.
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @param attachmentID attachment point to use (@c GL_COLOR_ATTACHMENT0, ... )
     * @param layer index of the layer to attach
     */
    void attachTextureLayer(Texture3D* texture, GLenum attachmentID, int layer);

    /**
     * @brief Attach a layer of a 3D Color Texture to the framebuffer.
     * Will use the first available attachment point.
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @param layer index of the layer to attach
     * @return The used attachment point (@c GL_COLOR_ATTACHMENT0, ... )
     */
    GLenum attachColorTextureLayer(Texture3D* texture, int layer);

    /**
     * @brief Attach a layer of a 3D Color Texture to the framebuffer.
     * @pre The framebuffer must be active.
     * @param texture to attach
     * @param attachmentNumber number of attachment point to use (0, 1, ...)
     * @param layer index of the layer to attach
     * @return the attachment point used (@c GL_COLOR_ATTACHMENT0, ... )
     */
    GLenum attachColorTextureLayer(Texture3D* texture, int attachmentNumber, int layer);

    /**
     * @brief Detach the texture at the given attachment point
     * @pre The framebuffer must be active.
     * @param attachmentID to detach (@c GL_COLOR_ATTACHMENT0, ... )
     */
    void detachTexture(GLenum attachmentID);

    /**
     * @brief Detach all the attached textures
     * @pre The framebuffer must be active.
     */
    void detachAllTextures();

    /**
     * @brief Get the maximal number of color attachments
     * Queries @c GL_MAX_COLOR_ATTACHMENTS
     */
    static int getMaxColorAttachments();

    /**
     * @brief Check if there is any color attachments
     */

    bool hasColorAttachment() const;
    /**
     * @brief Check if there is any depth attachment
     */
    bool hasDepthAttachment() const;

    /**
     * @brief Check if there is any stencil attachment
     */
    bool hasStencilAttachment() const;

    /**
     * @brief Get the current list of attached colors texture id.
     *
     * Returns a list of all color attachments points, for unused points the value will be 0, for
     * used points the value will be the attached texture id.
     * @return List of attached texture ids.
     */
    const std::vector<GLuint>& attachedColorTextureIds() const { return attachedColorIds_; }

    /**
     * @brief Returns the texture id of the attached depth texture id or 0 if no depth is attached.
     */
    GLuint attachedDepthTextureId() const { return attachedDepthId_; }
    /**
     * @brief Returns the texture id of the attached stencil texture id of 0 if no stencil is
     * attached.
     */
    GLuint attachedStencilTextureId() const { return attachedStencilId_; }

    /**
     * @brief Bind the framebuffer name to GL_READ_FRAMEBUFFER for "Blit" reading
     * @param set True will bind this buffer, False will bind id 0
     */
    void setReadBlit(bool set = true) const;

    /**
     * @brief Bind the framebuffer name to GL_DRAW_FRAMEBUFFER for "Blit" drawing
     * @param set True will bind this buffer, False will bind id 0
     */
    void setDrawBlit(bool set = true);

private:
    static GLuint enumToNumber(GLenum attachmentID) { return attachmentID - GL_COLOR_ATTACHMENT0; }
    static GLenum numberToEnum(GLuint number) { return GL_COLOR_ATTACHMENT0 + number; }

    void registerAttachment(GLenum attachmentID, GLuint texId);
    void deregisterAttachment(GLenum attachmentID);
    GLenum firstFreeAttachmentID() const;

    static GLuint maxColorattachments();

    unsigned int id_;
    GLuint attachedDepthId_;
    GLuint attachedStencilId_;
    std::vector<GLuint> attachedColorIds_;

    Canvas::ContextID creationContext_ = nullptr;
};

namespace utilgl {

/**
 * @brief Convert the return value of @c glCheckFramebufferStatus to a human readable string
 */
IVW_MODULE_OPENGL_API std::string_view framebufferStatusToString(GLenum status);

/**
 * @brief A FrameBufferObject activation RAII utility, will store the current FBO and restore that
 * in the destructor
 */
class IVW_MODULE_OPENGL_API ActivateFBO {
public:
    explicit ActivateFBO(FrameBufferObject& fbo) : prevFbo_{0} {
        GLint currentFbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);
        if (static_cast<GLint>(fbo.getID()) != currentFbo) {
            fbo.activate();
            prevFbo_ = currentFbo;
        }
    }
    ActivateFBO(const ActivateFBO&) = delete;
    ActivateFBO(ActivateFBO&& rhs) noexcept : prevFbo_{rhs.prevFbo_} { rhs.prevFbo_ = 0; };
    ActivateFBO& operator=(const ActivateFBO&) = delete;
    ActivateFBO& operator=(ActivateFBO&& that) = delete;

    ~ActivateFBO() {
        if (prevFbo_ != 0) {
            glBindFramebuffer(GL_FRAMEBUFFER, prevFbo_);
        }
    }

private:
    GLint prevFbo_;
};

}  // namespace utilgl

}  // namespace inviwo
