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

#ifndef IVW_ABUFFERGL4_H
#define IVW_ABUFFERGL4_H

#include <modules/abuffergl/abufferglmoduledefine.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/interactionhandler.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/clock.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/shader/shader.h>

namespace inviwo {

class TextureUnit;
class ImageGL;

typedef glm::u8vec4 Abuffer_RGBA_Type;
typedef glm::float32 Abuffer_ExtDataType;
#define ABUFFER_DATA_PER_NODE 1

#ifdef IVW_PROFILING
#define ENABLE_ABUFFER_PROFILING(msg) IVW_CPU_PROFILING(msg)
#else
#define ENABLE_ABUFFER_PROFILING(msg)
#endif

#define ABUFFER_PROFILE(msg) ENABLE_ABUFFER_PROFILING(msg)

class IVW_MODULE_ABUFFERGL_API ABufferGLCompositeProperty : public CompositeProperty {
public:
    InviwoPropertyInfo();    
    ABufferGLCompositeProperty(std::string identifier, std::string displayName,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);

    ABufferGLCompositeProperty(const ABufferGLCompositeProperty& rhs);
    ABufferGLCompositeProperty& operator=(const ABufferGLCompositeProperty& that);
    virtual ABufferGLCompositeProperty* clone() const;
    virtual ~ABufferGLCompositeProperty();
    virtual std::string getClassIdentifierForWidget() const;

    BoolProperty abufferEnable_;
    IntProperty abufferPageSize_;
    IntProperty abufferLocalMemorySize_;
    ButtonProperty abufferReSize_;
    BoolProperty abufferWriteABufferInfoToFile_;
    FloatVec4Property bgColor_;
    size_t sharedPoolSize_;
};

class IVW_MODULE_ABUFFERGL_API Inviwo_ABufferGL4 {
public:
    Inviwo_ABufferGL4(ivec2 dim = ivec2(512));
    virtual ~Inviwo_ABufferGL4();

    class ABufferGLInteractionHandler : public InteractionHandler {
    public:
        ABufferGLInteractionHandler(Inviwo_ABufferGL4* parent);
        ~ABufferGLInteractionHandler(){};
        void invokeEvent(Event* event);

    private:
        Inviwo_ABufferGL4* parent_;
        glm::ivec2 prevMousePos_;
        MouseEvent mousePressEvent_;
        KeyboardEvent upEvent_;
        KeyboardEvent downEvent_;
    };

    void abuffer_allocateMemory();
    void abuffer_initABuffer(ivec2 dim, bool forceInitialization = false);
    void aBuffer_bindTextures();
    void abuffer_addUniforms(Shader* shader);
    void abuffer_addShaderDefinesAndBuild(Shader* shader);
    void aBuffer_unbind();
    void aBuffer_resolveLinkList(ImageGL* imageGL);
    void aBuffer_resetLinkList(ImageGL* imageGL);
    glm::uint abuffer_fetchCurrentAtomicCounterValue();
    void abuffer_printDebugInfo(glm::ivec2 pos);
    void abuffer_textureInfo();
    ABufferGLCompositeProperty settings_;
    Shader resolveABufferShader_;
    Shader resetABufferShader_;

protected:
    void aBuffer_incrementSharedPoolSize();
    bool aBuffer_isMemoryReallocationRequired(ivec2 currentPortDim);
    bool abuffer_isMemoryPoolExpansionRequired();


    ABufferGLInteractionHandler abuffInteractionHandler_;

    GLuint shared_RGBA_DataListBuffID_;
    GLuint64 shared_RGBA_DataListAddress_;
    GLuint shared_Ext_DataListBuffID_;
    GLuint64 shared_Ext_DataListAddress_;
    GLuint sharedLinkListBuffID_;
    GLuint64 sharedLinkListAddress_;
    GLuint globalAtomicsBufferId_;

    Image* abufferPageIdxImgTexture_;
    Image* abufferFragCountImgTexture_;
    Image* semaphoreImgTexture_;

    GLuint* globalAtomicCounterBuffer_;

    std::vector<TextureUnit*> texUnits_;

    ivec2 dim_;

    // Global init function
    void init(void);
};

}  // namespace

#endif  // IVW_ABUFFERGL4_H
