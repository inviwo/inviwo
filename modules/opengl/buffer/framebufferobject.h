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

#ifndef IVW_FRAMEBUFFEROBJECT_H
#define IVW_FRAMEBUFFEROBJECT_H

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/texture2darray.h>
#include <modules/opengl/texture/texture3d.h>
#include <vector>

namespace inviwo {

class IVW_MODULE_OPENGL_API FrameBufferObject {

public:
    FrameBufferObject();
    ~FrameBufferObject();

    // activate this FBO and store the currently set FBO
    void activate();
    void defineDrawBuffers();
    // unbind FBO and restore previous one
    void deactivate();

    // use this function to unbind the FBO, without restoring the previous
    // after this call, no FBO is bound
    static void deactivateFBO();

    //For attaching a 2D Texture
    void attachTexture(Texture2D* texture, GLenum attachmentID);
    GLenum attachColorTexture(Texture2D* texture);
    // if forcedLocation is > -1, this will enforce to position the color attachment 
    // at the given location in the draw buffer list (as used for attrib location in shaders)
    // side effects: affects subsequent buffer locations of already attached color buffers
    GLenum attachColorTexture(Texture2D* texture, int attachmentNumber, bool attachFromRear = false, int forcedLocation=-1);
    
    //For attaching a 2D Array Texture
    void attachTexture(Texture2DArray* texture, GLenum attachmentID);
    GLenum attachColorTexture(Texture2DArray* texture);
    // if forcedLocation is > -1, this will enforce to position the color attachment 
    // at the given location in the draw buffer list (as used for attrib location in shaders)
    // side effects: affects subsequent buffer locations of already attached color buffers
    GLenum attachColorTexture(Texture2DArray* texture, int attachmentNumber, bool attachFromRear = false, int forcedLocation=-1);
    
    //For attaching a layer of a 2D Array Texture
    void attachTextureLayer(Texture2DArray* texture, GLenum attachmentID, int layer);
    GLenum attachColorTextureLayer(Texture2DArray* texture, int layer);
    // if forcedLocation is > -1, this will enforce to position the color attachment 
    // at the given location in the draw buffer list (as used for attrib location in shaders)
    // side effects: affects subsequent buffer locations of already attached color buffers
    GLenum attachColorTextureLayer(Texture2DArray* texture, int attachmentNumber, int layer, bool attachFromRear = false, int forcedLocation=-1);

    //For attaching a 3D Texture
    void attachTexture(Texture3D* texture, GLenum attachmentID);
    GLenum attachColorTexture(Texture3D* texture);
    // if forcedLocation is > -1, this will enforce to position the color attachment 
    // at the given location in the draw buffer list (as used for attrib location in shaders)
    // side effects: affects subsequent buffer locations of already attached color buffers
    GLenum attachColorTexture(Texture3D* texture, int attachmentNumber, bool attachFromRear = false, int forcedLocation=-1);

    //For attaching a layer of a 3D Texture
    void attachTextureLayer(Texture3D* texture, GLenum attachmentID, int layer);
    GLenum attachColorTextureLayer(Texture3D* texture, int layer);
    // if forcedLocation is > -1, this will enforce to position the color attachment 
    // at the given location in the draw buffer list (as used for attrib location in shaders)
    // side effects: affects subsequent buffer locations of already attached color buffers
    GLenum attachColorTextureLayer(Texture3D* texture, int attachmentNumber, int layer, bool attachFromRear = false, int forcedLocation=-1);

    void detachTexture(GLenum attachmentID);
    void detachAllTextures();

    unsigned int getID() const;
    // returns a compactified list of all color attachments (as used in glDrawBuffers())
    const std::vector<GLenum>& getDrawBuffers() const { return drawBuffers_; }
    // returns a boolean field indicating whether attachment i has an attached texture
    const std::vector<bool>& getDrawBuffersInUse() const { return buffersInUse_; }
    const GLenum* getDrawBuffersDeprecated() const;
    int getMaxColorAttachments() const;

    // returns the location of the given attachment withing the registered draw buffers
    // (e.g. used for glBindAttribLocation() and glFragDataLocation())
    int getAttachmentLocation(GLenum attachmentID) const;

    bool hasColorAttachment() const;
    bool hasDepthAttachment() const;
    bool hasStencilAttachment() const;

    void checkStatus();

    void setRead_Blit(bool set=true) const;
    void setDraw_Blit(bool set=true);

protected:
    void performAttachTexture(GLenum attachmentID);
    bool performAttachColorTexture(GLenum& outAttachNumber);
    // if forcedLocation is > -1, this will enforce to position the color attachment 
    // at the given location in the draw buffer list (as used for attrib location in shaders)
    // side effects: affects subsequent buffer locations of already attached color buffers
    // NOTE: if forcedLocation is larger than the number of attachments, it will not be considered
    bool performAttachColorTexture(GLenum& outAttachNumber, int attachmentNumber, 
        bool attachFromRear = false, int forcedLocation = -1);

    std::string printBuffers() const;
    static std::string getAttachmentStr(GLenum attachmentID);

private:
    unsigned int id_;
    bool hasDepthAttachment_;
    bool hasStencilAttachment_;

    std::vector<GLenum> drawBuffers_;
    std::vector<bool> buffersInUse_;
    int maxColorattachments_;

    const static GLenum colorAttachmentEnums_[];

    GLint prevFbo_;
    GLint prevDrawFbo_;
    mutable GLint prevReadFbo_;
};

} // namespace

#endif // IVW_FRAMEBUFFEROBJECT_H
