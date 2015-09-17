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

/**
 * Fast Single-pass A-Buffer using OpenGL 4.0 V2.0 : Linked Lists
 * Copyright Cyril Crassin, June/July 2010
 **/

#ifndef IVW_ABUFFERGL4_LINKEDLIST_H
#define IVW_ABUFFERGL4_LINKEDLIST_H

#include <modules/abuffergl/abufferglmoduledefine.h>
#include <modules/abuffergl/abufferglhelpers/abuffergl.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/image.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/texture/textureunit.h>

namespace inviwo {

class IVW_MODULE_ABUFFERGL_API Inviwo_ABufferGL4_LinkedList : public Inviwo_ABufferGL4 {
public:
    Inviwo_ABufferGL4_LinkedList(ivec2 dim = ivec2(512));
    virtual ~Inviwo_ABufferGL4_LinkedList();
    virtual void allocateABuffer(void);
    virtual void initABuffer(int abufferSize=0);
    virtual void addUniforms(Shader* shader);
    virtual void deinitABuffer(void);
    virtual void clear(void);
    Image* get_AbufferFragCountImgTexture() { return abufferFragCountImgTexture_; }
    size_t getDefaultSharedPoolSize() { return sharedPoolSize_;}

private:

    void initShaders_LinkedList(void);
    void initABuffer_LinkedList(void);
    void initSharedPool_LinkedList(void);
    bool display_LinkedList_ManageSharedPool(void);

    ///Linked Lists///
    size_t sharedPoolSize_;
    //Occlusion queries
    GLuint totalFragmentQuery_;
    GLuint lastFrameNumFrags_;
    bool queryRequested_;

    std::vector<TextureUnit*> texUnits_;

    GLuint		curSharedPageBuffID_;
    GLuint64 curSharedPageAddress_;

    GLuint abufferPageIdxBuffID_;
    GLuint64 abufferPageIdxAddress_;
    GLuint abufferPageIdxTexID_;

    GLuint abufferFragCountBuffID_;
    GLuint64 abufferFragCountAddress_;
    GLuint abufferFragCountTexID_;

    GLuint shared_RGBA_DataListBuffID_;
    GLuint64 shared_RGBA_DataListAddress_;
    GLuint shared_RGBA_DataListTexID_;

    GLuint shared_Ext_DataListBuffID_;
    GLuint64 shared_Ext_DataListAddress_;
    GLuint shared_Ext_DataListTexID_;

    GLuint sharedPageListComparisonBuffID_;
    GLuint64 sharedPageListComparisonAddress_;
    GLuint sharedPageListComparisonTexID_;

    GLuint sharedLinkListBuffID_;
    GLuint64 sharedLinkListAddress_;
    GLuint sharedLinkListTexID_;

    GLuint semaphoreBuffID_;
    GLuint64 semaphoreAddress_;
    GLuint semaphoreTexID_;

    //Abuffer Link List images
    Image* abufferPageIdxImgTexture_;
    Image* abufferFragCountImgTexture_;
    Image* semaphoreImgTexture_;

    glm::mat4 projectionMatrix_;
    glm::mat4 modelViewMatrix_;

    GLuint* globalAtomicCounterBuffer_;
    GLuint globalAtomicsBufferId_;
    int abufferSize_;
};

typedef Inviwo_ABufferGL4_LinkedList InviwoABuffer;

} // namespace

#endif // IVW_ABUFFERGL4_LINKEDLIST_H