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

#include <modules/abuffergl/abufferglhelpers/abuffergl.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/inviwoopengl.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/image/layergl.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <modules/opengl/image/imagegl.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/clock.h>

namespace inviwo {

static int ABUFFER_RGBA_DATA_TYPE_SIZE = sizeof(Abuffer_RGBA_Type);
static int ABUFFER_EXT_DATA_TYPE_SIZE = sizeof(Abuffer_ExtDataType);
#define ABUFFER_PAGE_SIZE glm::pow(2, settings_.abufferPageSize_.get())
#define ABUFFER_SIZE settings_.abufferLocalMemorySize_.get()

PropertyClassIdentifier(ABufferGLCompositeProperty, "org.inviwo.ABufferGLCompositeProperty");

ABufferGLCompositeProperty::ABufferGLCompositeProperty(std::string identifier,
    std::string displayName,
    InvalidationLevel invalidationLevel,
    PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , abufferEnable_("enable-abuffer", "Enable ABuffer")
    , abufferPageSize_("abuffer-page-size", "ABuffer Page Size (power of 2)", 3, 3, 5, 1)
    , abufferLocalMemorySize_("abuffer-local-memory", "ABuffer Local Memory", 128, 128, 256, 128)
    , abufferReSize_("abuffer-resize", "ABuffer Resize")
    , abufferWriteABufferInfoToFile_("abuffer-write-abuffer-info", "Write ABuffer To File", false)
    , bgColor_("bgColor", "Background Color", vec4(1.0)) {
    addProperty(abufferEnable_);
    addProperty(abufferPageSize_);
    addProperty(abufferLocalMemorySize_);
    addProperty(abufferReSize_);
    addProperty(abufferWriteABufferInfoToFile_);
    addProperty(bgColor_);

    setCollapsed(true);
    setAllPropertiesCurrentStateAsDefault();
}

ABufferGLCompositeProperty::ABufferGLCompositeProperty(const ABufferGLCompositeProperty& rhs)
    : CompositeProperty(rhs)
    , abufferEnable_(rhs.abufferEnable_)
    , abufferPageSize_(rhs.abufferPageSize_)
    , abufferLocalMemorySize_(rhs.abufferLocalMemorySize_)
    , abufferReSize_(rhs.abufferReSize_)
    , abufferWriteABufferInfoToFile_(rhs.abufferWriteABufferInfoToFile_)
    , bgColor_(rhs.bgColor_) {
    setAllPropertiesCurrentStateAsDefault();
}

ABufferGLCompositeProperty& ABufferGLCompositeProperty::operator=(const ABufferGLCompositeProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        abufferEnable_ = that.abufferEnable_;
        abufferPageSize_ = that.abufferPageSize_;
        abufferLocalMemorySize_ = that.abufferLocalMemorySize_;
        abufferReSize_ = that.abufferReSize_;
        abufferWriteABufferInfoToFile_ = that.abufferWriteABufferInfoToFile_;
    }
    return *this;
}

ABufferGLCompositeProperty* ABufferGLCompositeProperty::clone() const {
    return new ABufferGLCompositeProperty(*this);
}

ABufferGLCompositeProperty::~ABufferGLCompositeProperty() {}

std::string ABufferGLCompositeProperty::getClassIdentifierForWidget() const {
    return CompositeProperty::CLASS_IDENTIFIER;
}

//////////////////////////////////////////////////////////////////////////

Inviwo_ABufferGL4::Inviwo_ABufferGL4(ivec2 dim)
    : abuffInteractionHandler_(this)
    , dim_(dim)
    , abufferPageIdxImgTexture_(nullptr)
    , abufferFragCountImgTexture_(nullptr)
    , semaphoreImgTexture_(nullptr)
    , globalAtomicCounterBuffer_(nullptr)
    , shared_RGBA_DataListBuffID_(0)
    , shared_Ext_DataListBuffID_(0)
    , sharedLinkListBuffID_(0)
    , globalAtomicsBufferId_(0)
    , resolveABufferShader_("abufferresolve.hglsl")
    , resetABufferShader_("abufferreset.hglsl")
    , settings_("abuffer-settings-property", "ABuffer Settings") {
    settings_.sharedPoolSize_ = dim_.x * dim_.y * ABUFFER_PAGE_SIZE * 40;
}

Inviwo_ABufferGL4::~Inviwo_ABufferGL4() {
    if (abufferPageIdxImgTexture_) delete abufferPageIdxImgTexture_;
    if (abufferFragCountImgTexture_) delete abufferFragCountImgTexture_;
    if (semaphoreImgTexture_) delete semaphoreImgTexture_;
    if (globalAtomicCounterBuffer_) {
        delete globalAtomicCounterBuffer_;
        globalAtomicCounterBuffer_ = 0;
    }

    for (size_t i = 0; i < texUnits_.size(); i++) delete texUnits_[i];
    texUnits_.clear();
}

bool Inviwo_ABufferGL4::aBuffer_isMemoryReallocationRequired(ivec2 currentPortDim) {
    bool requiresUpdate = false;
    if ((dim_.x * dim_.y != currentPortDim.x * currentPortDim.y) || texUnits_.size() == 0 ||
        abufferPageIdxImgTexture_ == 0 || abufferFragCountImgTexture_ == 0 ||
        semaphoreImgTexture_ == 0 || globalAtomicCounterBuffer_ == 0) {
        if (settings_.abufferEnable_.get()) requiresUpdate = true;
        dim_ = currentPortDim;
        // LogWarn("ABuffer requires update")
    }
    return requiresUpdate;
}

bool Inviwo_ABufferGL4::abuffer_isMemoryPoolExpansionRequired() {
    if (globalAtomicCounterBuffer_ && globalAtomicsBufferId_) {
        glm::uint totalABuffUsed = abuffer_fetchCurrentAtomicCounterValue() * ABUFFER_PAGE_SIZE;
        if (fabs(float(settings_.sharedPoolSize_ - totalABuffUsed)) <
            float(settings_.sharedPoolSize_ * .30f)) {
            return true;
        } else
            return false;
    }
    return false;
}

void Inviwo_ABufferGL4::abuffer_allocateMemory() {
    // allocate abuffer - one time only
    LogWarn("ABuffer allocate abuffer");
    if (abufferPageIdxImgTexture_) delete abufferPageIdxImgTexture_;
    if (abufferFragCountImgTexture_) delete abufferFragCountImgTexture_;
    if (semaphoreImgTexture_) delete semaphoreImgTexture_;
    if (globalAtomicCounterBuffer_) {
        delete globalAtomicCounterBuffer_;
        globalAtomicCounterBuffer_ = 0;
    }
    for (size_t i = 0; i < texUnits_.size(); i++) delete texUnits_[i];
    texUnits_.clear();
    abufferPageIdxImgTexture_ = 0;
    abufferFragCountImgTexture_ = 0;
    semaphoreImgTexture_ = 0;
    globalAtomicCounterBuffer_ = 0;

    if (settings_.abufferEnable_.get()) {
        abufferPageIdxImgTexture_ = new Image(dim_, DataUInt32::get());
        abufferFragCountImgTexture_ = new Image(dim_, DataUInt32::get());
        semaphoreImgTexture_ = new Image(dim_, DataUInt32::get());

        globalAtomicCounterBuffer_ = new GLuint[4];
        memset(globalAtomicCounterBuffer_, 0, 4 * sizeof(GLuint));
        // FIXME: Try to avoid retaining any texture units.
        texUnits_.push_back(new TextureUnit());
        texUnits_.push_back(new TextureUnit());
        texUnits_.push_back(new TextureUnit());
    }
}

void Inviwo_ABufferGL4::abuffer_initABuffer(ivec2 dim, bool forceInitialization) {
    bool initRequired = false;
    if (aBuffer_isMemoryReallocationRequired(dim)) {
        dim_ = dim;
        abuffer_allocateMemory();
        initRequired = true;
    }

    if (abuffer_isMemoryPoolExpansionRequired()) {
        aBuffer_incrementSharedPoolSize();
        initRequired = true;
    }

    if (forceInitialization) initRequired = true;

    if (!initRequired) return;

    ABUFFER_PROFILE("Initialize-abuffer: Generate Buffer");

    TextureUnit* tex1 = texUnits_[0];
    TextureUnit* tex2 = texUnits_[1];
    TextureUnit* tex3 = texUnits_[2];

    Layer* tempLayer = abufferPageIdxImgTexture_->getColorLayer();
    memset(tempLayer->getEditableRepresentation<LayerRAM>()->getData(), 0,
           dim.x * dim.y * sizeof(glm::uint32));

    tempLayer = abufferFragCountImgTexture_->getColorLayer();
    memset(tempLayer->getEditableRepresentation<LayerRAM>()->getData(), 0,
           dim.x * dim.y * sizeof(glm::uint32));

    tempLayer = semaphoreImgTexture_->getColorLayer();
    memset(tempLayer->getEditableRepresentation<LayerRAM>()->getData(), 0,
           dim.x * dim.y * sizeof(glm::uint32));

    const ImageGL* imageGL = 0;
    const LayerGL* layer = 0;
    TextureUnit* texUnit = 0;

    /// Page idx storage///
    imageGL = abufferPageIdxImgTexture_->getRepresentation<ImageGL>();
    layer = imageGL->getColorLayerGL(0);
    texUnit = tex1;

    layer->bindTexture(texUnit->getEnum());
    // Set filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTextureEXT(texUnit->getUnitNumber(), layer->getTexture()->getID(), 0, false, 0,
                          GL_READ_WRITE, GL_R32UI);
    LGL_ERROR;

    /// per-pixel page counter///
    imageGL = abufferFragCountImgTexture_->getRepresentation<ImageGL>();
    layer = imageGL->getColorLayerGL(0);
    texUnit = tex2;

    layer->bindTexture(texUnit->getEnum());
    // Set filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTextureEXT(texUnit->getUnitNumber(), layer->getTexture()->getID(), 0, false, 0,
                          GL_READ_WRITE, GL_R32UI);
    LGL_ERROR;

    /// Semaphore///
    imageGL = semaphoreImgTexture_->getRepresentation<ImageGL>();
    layer = imageGL->getColorLayerGL(0);
    texUnit = tex3;

    layer->bindTexture(texUnit->getEnum());
    // Set filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTextureEXT(texUnit->getUnitNumber(), layer->getTexture()->getID(), 0, false, 0,
                          GL_READ_WRITE, GL_R32UI);
    LGL_ERROR;

    if (globalAtomicCounterBuffer_) {
        if (!globalAtomicsBufferId_) glGenBuffers(1, &globalAtomicsBufferId_);
        // bind the buffer and define its initial storage capacity
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, globalAtomicsBufferId_);
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4 * sizeof(glm::uint), globalAtomicCounterBuffer_,
                     GL_DYNAMIC_DRAW);
        // unbind the buffer
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    }

    if (!shared_RGBA_DataListBuffID_) glGenBuffers(1, &shared_RGBA_DataListBuffID_);
    glBindBuffer(GL_TEXTURE_BUFFER, shared_RGBA_DataListBuffID_);
    glBufferData(GL_TEXTURE_BUFFER, (settings_.sharedPoolSize_) * ABUFFER_RGBA_DATA_TYPE_SIZE, NULL,
                 GL_STATIC_DRAW);
    glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
    glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV,
                                &shared_RGBA_DataListAddress_);

    if (!shared_Ext_DataListBuffID_) glGenBuffers(1, &shared_Ext_DataListBuffID_);
    glBindBuffer(GL_TEXTURE_BUFFER, shared_Ext_DataListBuffID_);
    glBufferData(GL_TEXTURE_BUFFER, (settings_.sharedPoolSize_) * ABUFFER_EXT_DATA_TYPE_SIZE, NULL,
                 GL_STATIC_DRAW);
    glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
    glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV,
                                &shared_Ext_DataListAddress_);

    /// Shared link pointer list///
    if (!sharedLinkListBuffID_) glGenBuffers(1, &sharedLinkListBuffID_);
    glBindBuffer(GL_TEXTURE_BUFFER, sharedLinkListBuffID_);
    glBufferData(GL_TEXTURE_BUFFER, (settings_.sharedPoolSize_) * sizeof(glm::uint), NULL,
                 GL_STATIC_DRAW);
    glMakeBufferResidentNV(GL_TEXTURE_BUFFER, GL_READ_WRITE);
    glGetBufferParameterui64vNV(GL_TEXTURE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV,
                                &sharedLinkListAddress_);

    LogWarn("ABuffer init called. This is expensive")
}

void Inviwo_ABufferGL4::aBuffer_bindTextures() {
    TextureUnit* tex1 = texUnits_[0];
    TextureUnit* tex2 = texUnits_[1];
    TextureUnit* tex3 = texUnits_[2];

    const ImageGL* imageGL = 0;
    const LayerGL* layer = 0;

    imageGL = abufferPageIdxImgTexture_->getEditableRepresentation<ImageGL>();
    layer = imageGL->getColorLayerGL(0);

    layer->bindTexture(tex1->getEnum());
    // Set filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTextureEXT(tex1->getUnitNumber(), layer->getTexture()->getID(), 0, false, 0,
                          GL_READ_WRITE, GL_R32UI);
    LGL_ERROR;

    /// per-pixel page counter///
    imageGL = abufferFragCountImgTexture_->getEditableRepresentation<ImageGL>();
    layer = imageGL->getColorLayerGL(0);

    layer->bindTexture(tex2->getEnum());
    // Set filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTextureEXT(tex2->getUnitNumber(), layer->getTexture()->getID(), 0, false, 0,
                          GL_READ_WRITE, GL_R32UI);
    LGL_ERROR;

    /// Semaphore///
    imageGL = semaphoreImgTexture_->getEditableRepresentation<ImageGL>();
    layer = imageGL->getColorLayerGL(0);

    layer->bindTexture(tex3->getEnum());
    // Set filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTextureEXT(tex3->getUnitNumber(), layer->getTexture()->getID(), 0, false, 0,
                          GL_READ_WRITE, GL_R32UI);
    LGL_ERROR;
}

void Inviwo_ABufferGL4::abuffer_addUniforms(Shader* shader) {
    // LGL_ERROR;
    shader->setUniform("abufferPageIdxImg", texUnits_[0]->getUnitNumber());
    // LGL_ERROR;
    shader->setUniform("abufferFragCountImg", texUnits_[1]->getUnitNumber());
    // LGL_ERROR;
    shader->setUniform("semaphoreImg", texUnits_[2]->getUnitNumber());
    // LGL_ERROR;

    glProgramUniformui64NV(shader->getID(),
                           glGetUniformLocation(shader->getID(), "d_shared_RGBA_DataPageList"),
                           shared_RGBA_DataListAddress_);
    // LGL_ERROR;
    glProgramUniformui64NV(shader->getID(),
                           glGetUniformLocation(shader->getID(), "d_shared_Ext_DataPageList"),
                           shared_Ext_DataListAddress_);
    // LGL_ERROR;
    glProgramUniformui64NV(shader->getID(),
                           glGetUniformLocation(shader->getID(), "d_sharedLinkList"),
                           sharedLinkListAddress_);
    // LGL_ERROR;
    glProgramUniform1iEXT(shader->getID(),
                          glGetUniformLocation(shader->getID(), "abufferSharedPoolSize"),
                          static_cast<GLint>(settings_.sharedPoolSize_));
    // LGL_ERROR;

    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, globalAtomicsBufferId_);
    LGL_ERROR;
}

void Inviwo_ABufferGL4::aBuffer_incrementSharedPoolSize() {
    glm::uint newSize = dim_.x * dim_.y * ABUFFER_PAGE_SIZE;
    settings_.sharedPoolSize_ += newSize;
    LogWarn("Incrementing Shared Pool Size (Upper Bound)")
}

void Inviwo_ABufferGL4::abuffer_addShaderDefinesAndBuild(Shader* shader) {
    ShaderObject* fragObj = shader->getFragmentShaderObject();
    fragObj->clearShaderExtensions();
    fragObj->addShaderExtension("GL_NV_gpu_shader5", true);
    fragObj->addShaderExtension("GL_EXT_shader_image_load_store", true);
    fragObj->addShaderExtension("GL_EXT_bindable_uniform", true);

    ivec2 dim = dim_;
    // defaults
    // LogInfo("ABUFFER_DATA_TYPE_SIZE << " << toString(ABUFFER_DATA_TYPE_SIZE))
    fragObj->addShaderDefine("ABUFFER_SIZE", toString(ABUFFER_SIZE));
    fragObj->addShaderDefine("ABUFFER_RGBA_DATA_TYPE_SIZE", toString(ABUFFER_RGBA_DATA_TYPE_SIZE));
    fragObj->addShaderDefine("ABUFFER_EXT_DATA_TYPE_SIZE", toString(ABUFFER_EXT_DATA_TYPE_SIZE));
    fragObj->addShaderDefine("ABUFFER_DATA_PER_NODE", toString(ABUFFER_DATA_PER_NODE));
    fragObj->addShaderDefine("SCREEN_WIDTH", toString(dim.x));
    fragObj->addShaderDefine("SCREEN_HEIGHT", toString(dim.y));

    vec4 pBackgroundColor(settings_.bgColor_.get());
    fragObj->addShaderDefine("BACKGROUND_COLOR_R", toString(pBackgroundColor.x));
    fragObj->addShaderDefine("BACKGROUND_COLOR_G", toString(pBackgroundColor.y));
    fragObj->addShaderDefine("BACKGROUND_COLOR_B", toString(pBackgroundColor.z));
    fragObj->addShaderDefine("BACKGROUND_COLOR_A", toString(pBackgroundColor.w));

    fragObj->addShaderDefine("USE_ABUFFER", toString(settings_.abufferEnable_.get() ? 1 : 0));
    fragObj->addShaderDefine("ABUFFER_USE_TEXTURES", toString(1));        //( always 1)
    fragObj->addShaderDefine("SHAREDPOOL_USE_TEXTURES", toString(0));     //( always 0)
    fragObj->addShaderDefine("ABUFFER_RESOLVE_USE_SORTING", toString(1)); //( always 1)

    fragObj->addShaderDefine("ABUFFER_PAGE_SIZE", toString(ABUFFER_PAGE_SIZE));
    fragObj->addShaderDefine("ABUFFER_DISPNUMFRAGMENTS", toString(0));
    // LogInfo("Default shader defines added")
    shader->rebuild();
}

void Inviwo_ABufferGL4::aBuffer_unbind() {
    const ImageGL* imageGL = 0;
    const LayerGL* layer = 0;

    imageGL = abufferPageIdxImgTexture_->getRepresentation<ImageGL>();
    layer = imageGL->getColorLayerGL(0);
    layer->unbindTexture();

    imageGL = abufferFragCountImgTexture_->getRepresentation<ImageGL>();
    layer = imageGL->getColorLayerGL(0);
    layer->unbindTexture();

    imageGL = semaphoreImgTexture_->getRepresentation<ImageGL>();
    layer = imageGL->getColorLayerGL(0);
    layer->unbindTexture();
}

void Inviwo_ABufferGL4::aBuffer_resolveLinkList(ImageGL* imageGL, const Image* inputimage) {
    //TextureUnit* tex1 = texUnits_[0]; //unused
    //TextureUnit* tex2 = texUnits_[1]; //unused
    //TextureUnit* tex3 = texUnits_[2]; //unused

    TextureUnitContainer units;

    imageGL->activateBuffer();
    // utilgl::activateTarget(outport_);

    aBuffer_bindTextures();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    resolveABufferShader_.activate();
    // setGlobalShaderParameters(resolveABufferShader_);
    // utilgl::setShaderUniforms(&resolveABufferShader_, camera_, "camera_");
    // utilgl::setShaderUniforms(&resolveABufferShader_, lightingProperty_, "light_");
    

    abuffer_addUniforms(&resolveABufferShader_);
    // aBuffer_addUniforms(resolveABufferShader_, tex1, tex2, tex3);
    if (inputimage) {
        resolveABufferShader_.setUniform("isInputImageGiven", true);
        utilgl::bindAndSetUniforms(resolveABufferShader_, units,
            *inputimage, "inputimage", ImageType::ColorDepth);
    }
    else resolveABufferShader_.setUniform("isInputImageGiven", false);

    LGL_ERROR;

    utilgl::singleDrawImagePlaneRect();

    resolveABufferShader_.deactivate();

    glDisable(GL_BLEND);
    utilgl::deactivateCurrentTarget();
}

void Inviwo_ABufferGL4::aBuffer_resetLinkList(ImageGL* imageGL) {
    // TODO: Remove explicit reset. Alternatively perform reset after every reslove, this can avoid
    // some overheads.
    /*imageGL->activateBuffer();
    utilgl::clearCurrentTarget();
    //utilgl::activateTarget(outport_);
    aBuffer_bindTextures(tex1, tex2, tex3);
    //aBuffer_bindBuffers();
    resetABufferShader_->activate();
    setGlobalShaderParameters(resetABufferShader_);

    abuffer_addUniforms(resetABufferShader_);
    //aBuffer_addUniforms(resetABufferShader_, tex1, tex2, tex3);

    LGL_ERROR;

    utilgl::singleDrawImagePlaneRect();

    resetABufferShader_->deactivate();

    glDisable(GL_BLEND);
    utilgl::deactivateCurrentTarget();*/

    if (globalAtomicCounterBuffer_ && globalAtomicsBufferId_) {
        // bind the buffer and define its initial storage capacity
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, globalAtomicsBufferId_);
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4 * sizeof(glm::uint), globalAtomicCounterBuffer_,
                     GL_DYNAMIC_DRAW);
        // unbind the buffer
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    }
}

glm::uint Inviwo_ABufferGL4::abuffer_fetchCurrentAtomicCounterValue() {
    glm::uint totalABuffUsed = 0;
    glBindBuffer(GL_TEXTURE_BUFFER, globalAtomicsBufferId_);
    glm::uint* atomicCounterBuff = new glm::uint[4];
    void* glAtomicCounterBuff =
        glMapBufferRange(GL_TEXTURE_BUFFER, 0, 4 * sizeof(glm::uint), GL_MAP_READ_BIT);
    memcpy(atomicCounterBuff, glAtomicCounterBuff, 4 * sizeof(glm::uint));
    glUnmapBuffer(GL_TEXTURE_BUFFER);
    totalABuffUsed = atomicCounterBuff[0];
    delete atomicCounterBuff;
    return totalABuffUsed;
}

void Inviwo_ABufferGL4::abuffer_printDebugInfo(glm::ivec2 pos) {
    if (pos.x >= 0 && pos.y >= 0) {
        std::stringstream ss;
        ss << "MousePos: " << pos.x << " x " << pos.y << std::endl;

        Image* fcImage = abufferFragCountImgTexture_;
        const LayerRAM* layerRam_FragCount =
            fcImage->getColorLayer()->getRepresentation<LayerRAM>();
        const glm::uint32* fcImageData = (const glm::uint32*)layerRam_FragCount->getData();
        ivec2 dim = fcImage->getDimensions();
        ivec2 sPos = ivec2(pos.x, dim.y - pos.y);
        size_t ind = LayerRAM::posToIndex(sPos, fcImage->getDimensions());
        glm::uint32 fragCountVal = fcImageData[ind];
        ss << "Frag Count Value : " << fragCountVal << std::endl;

        Image* fPageImage = abufferPageIdxImgTexture_;
        const LayerRAM* layerRam_Page = fPageImage->getColorLayer()->getRepresentation<LayerRAM>();
        const glm::uint32* fpImageData = (const glm::uint32*)layerRam_Page->getData();
        glm::uint32 pageVal = (int)fpImageData[ind];
        ss << "Start Page Index : " << pageVal << std::endl;
        ss << " ------------------------------------ " << std::endl;

        std::vector<glm::uint> pageIndices;
        void* gldata = 0;
        glBindBuffer(GL_TEXTURE_BUFFER, sharedLinkListBuffID_);
        gldata = glMapBufferRange(GL_TEXTURE_BUFFER, 0,
                                  settings_.sharedPoolSize_ * sizeof(glm::uint), GL_MAP_READ_BIT);
        glm::uint* data = new glm::uint[settings_.sharedPoolSize_];
        if (gldata) {
            memcpy(data, gldata, settings_.sharedPoolSize_ * sizeof(glm::uint));
            // Unmap buffer after using it
            glUnmapBuffer(GL_TEXTURE_BUFFER);

            glm::uint currPage = (glm::uint)pageVal;

            ss << "Link-list indices : ";
            while (currPage) {
                pageIndices.push_back(currPage);
                ss << currPage << " ";
                currPage = data[currPage];
            };
            delete data;

            glm::uint totalABuffUsed = abuffer_fetchCurrentAtomicCounterValue();

            ss << "Atomic Counter Usage : used-counter = " << totalABuffUsed
               << " max-counter = " << settings_.sharedPoolSize_ / ABUFFER_PAGE_SIZE << std::endl;

            ss << std::endl;
            ss << " ------------------------------------ " << std::endl;

            if (pageIndices.size()) {
                // ss.str("");

                ss << "Link-list fragments : " << std::endl;

                if (fragCountVal) {
                    int totalCount = 0;
                    for (size_t i = 0; i < pageIndices.size(); i++) {
                        size_t offset = ABUFFER_PAGE_SIZE;
                        if (i == 0) {
                            offset = fragCountVal % ABUFFER_PAGE_SIZE;
                            if (offset == 0) offset = ABUFFER_PAGE_SIZE;
                        }

                        glm::uint dataSize = ABUFFER_PAGE_SIZE * ABUFFER_RGBA_DATA_TYPE_SIZE;
                        glBindBuffer(GL_TEXTURE_BUFFER, shared_RGBA_DataListBuffID_);
                        Abuffer_RGBA_Type* rgbData = new Abuffer_RGBA_Type[ABUFFER_PAGE_SIZE];
                        glm::uint startOffset_rgb =
                            (pageIndices[i] * ABUFFER_PAGE_SIZE) * ABUFFER_RGBA_DATA_TYPE_SIZE;
                        void* gldata_RGBA = glMapBufferRange(GL_TEXTURE_BUFFER, startOffset_rgb,
                                                             dataSize, GL_MAP_READ_BIT);
                        memcpy(rgbData, gldata_RGBA, dataSize);
                        glUnmapBuffer(GL_TEXTURE_BUFFER);

                        glBindBuffer(GL_TEXTURE_BUFFER, shared_Ext_DataListBuffID_);
                        Abuffer_ExtDataType* extData = new Abuffer_ExtDataType[ABUFFER_PAGE_SIZE];
                        dataSize = ABUFFER_PAGE_SIZE * ABUFFER_EXT_DATA_TYPE_SIZE;
                        glm::uint startOffset_ext =
                            (pageIndices[i] * ABUFFER_PAGE_SIZE) * ABUFFER_EXT_DATA_TYPE_SIZE;
                        void* gldata_EXT = glMapBufferRange(GL_TEXTURE_BUFFER, startOffset_ext,
                                                            dataSize, GL_MAP_READ_BIT);
                        memcpy(extData, gldata_EXT, dataSize);
                        glUnmapBuffer(GL_TEXTURE_BUFFER);

                        for (size_t j = 0; j < offset; j++) {
                            Abuffer_RGBA_Type rgb_val = rgbData[j];
                            ss << "[" << (int)rgb_val.x << "," << (int)rgb_val.y << ","
                               << (int)rgb_val.z << "," << (int)rgb_val.w << "]";
                            Abuffer_ExtDataType depth_val = extData[j];
                            ss << "[" << (Abuffer_ExtDataType)depth_val << "]";
                            ss << " " << std::endl;
                            totalCount++;
                            // if (totalCount>=fragCountVal) break;
                        }
                        ss << "####" << std::endl;
                        // if (totalCount>=fragCountVal) break;

                        delete rgbData;
                        delete extData;
                    }
                    ss << " ------------------------------------ " << std::endl;
                    ss << "Total fragements in Pages " << totalCount << std::endl;
                }

                ss << " ------------------------------------ " << std::endl;

                if (0) {
                    LogWarn(ss.str());
                } else {
                    std::string basePath = inviwo::filesystem::findBasePath();
                    std::ofstream fs((basePath + "abuffer_currentPixelInfo.txt").c_str());
                    fs << ss.str() << "\n";
                    fs.close();
                }
            }

        } else {
            LogError("Unable to map linked list data");
        }
        LogWarn(ss.str());
    }

    abuffer_textureInfo();
}

#define DUMP_TO_FILE 1
#define ABUFFER_FRAGMENT_COUNT_INFO 1
#define ABUFFER_PAGE_INDEX_INFO 0
#define ABUFFER_FULL_BUFFER 0

void Inviwo_ABufferGL4::abuffer_textureInfo() {
    std::vector<glm::uint32> allFragementCounts;

    allFragementCounts.clear();
    Image* fcImage = abufferFragCountImgTexture_;
    const LayerRAM* layerRam_FragCount = fcImage->getColorLayer()->getRepresentation<LayerRAM>();
    const glm::uint32* fcImageData = (const glm::uint32*)layerRam_FragCount->getData();
    glm::ivec2 dim = fcImage->getDimensions();
    size_t imageSize = (dim.x * dim.y);
    for (size_t i = 0; i < imageSize; i++) {
        if (DUMP_TO_FILE)
            allFragementCounts.push_back(fcImageData[i]);
        else if (fcImageData[i])
            allFragementCounts.push_back(fcImageData[i] + 1);
    }

    glm::uint32 cLower = *std::min_element(allFragementCounts.begin(), allFragementCounts.end());
    glm::uint32 cUpper = *std::max_element(allFragementCounts.begin(), allFragementCounts.end());
    LogWarn("Tex Min Link-list node per pixel " << cLower);
    LogWarn("Tex Max Link-list node per pixel " << cUpper);
    LogWarn(" ------------------------------------ ");

    std::vector<glm::uint32> allFragmentPageCounts;
    allFragmentPageCounts.clear();
    Image* fPageImage = abufferPageIdxImgTexture_;
    const LayerRAM* layerRam_Page = fPageImage->getColorLayer()->getRepresentation<LayerRAM>();
    const glm::uint32* fpImageData = (const glm::uint32*)layerRam_Page->getData();
    dim = fPageImage->getDimensions();
    imageSize = (dim.x * dim.y);
    for (size_t i = 0; i < imageSize; i++) {
        if (DUMP_TO_FILE)
            allFragmentPageCounts.push_back(fpImageData[i]);
        else if (fpImageData[i])
            allFragmentPageCounts.push_back(fpImageData[i]);
        // if (allFragmentPageCounts[i] < 100 && allFragmentPageCounts[i] > 1) LogWarn(" ----" <<
        // allFragmentPageCounts[i]);
    }

    glm::uint32 pLower = 0;
    glm::uint32 pUpper = 0;

    if (allFragmentPageCounts.size()) {
        pLower = *std::min_element(allFragmentPageCounts.begin(), allFragmentPageCounts.end());
        pUpper = *std::max_element(allFragmentPageCounts.begin(), allFragmentPageCounts.end());
    }

    LogWarn("Tex Min Page per pixel " << pLower * ABUFFER_PAGE_SIZE);
    LogWarn("Tex Max Page per pixel " << pUpper * ABUFFER_PAGE_SIZE);
    LogWarn(" ------------------------------------ ");

    if (!allFragmentPageCounts.size()) return;

    if (DUMP_TO_FILE) {
        std::string rndInt = toString(rand()) + "_";

        if (ABUFFER_PAGE_INDEX_INFO) {
            // std::sort(allFragmentPageCounts.begin(), allFragmentPageCounts.end());
            std::string basePath = inviwo::filesystem::findBasePath();
            std::ofstream fstream((basePath + rndInt + "abuffer_sortedPage.txt").c_str());
            for (size_t i = 0; i < allFragmentPageCounts.size(); i++) {
                if (allFragmentPageCounts[i] > 0) {
                    fstream << allFragmentPageCounts[i] << " ";
                    if (allFragmentPageCounts[i] % 100 == 0) fstream << "\n";
                }
            }
            fstream.close();
            LogWarn("ABuffer info @ " << basePath + rndInt + "abuffer_sortedPage.txt")
        }

        if (ABUFFER_FRAGMENT_COUNT_INFO) {
            // std::sort(allFragmentPageCounts.begin(), allFragmentPageCounts.end());
            std::string basePath = inviwo::filesystem::findBasePath();
            std::ofstream fstream((basePath + rndInt + "abuffer_linklistnodecount.txt").c_str());
            for (size_t i = 0; i < allFragementCounts.size(); i++) {
                fstream << allFragementCounts[i] << "\n";
            }
            fstream.close();
            LogWarn("ABuffer info @ " << basePath + rndInt + "abuffer_linklistnodecount.txt")
        }

        if (ABUFFER_FULL_BUFFER) {
            void* gldata = 0;
            glBindBuffer(GL_TEXTURE_BUFFER, sharedLinkListBuffID_);
            gldata =
                glMapBufferRange(GL_TEXTURE_BUFFER, 0,
                                 settings_.sharedPoolSize_ * sizeof(glm::uint), GL_MAP_READ_BIT);
            glm::uint* data = new glm::uint[settings_.sharedPoolSize_];
            if (gldata) {
                memcpy(data, gldata, settings_.sharedPoolSize_ * sizeof(glm::uint));
                // Unmap buffer after using it
                glUnmapBuffer(GL_TEXTURE_BUFFER);
            } else {
                LogError("Unable to map data");
            }

            if (gldata && DUMP_TO_FILE) {
                // GLuint64* data64 = (GLuint64*) data;
                std::string basePath = inviwo::filesystem::findBasePath();
                std::ofstream fs((basePath + rndInt + "abuffer_linkListBuffer.txt").c_str());
                for (size_t i = 0; i < settings_.sharedPoolSize_; i++) {
                    fs << data[i] << "\n";
                }
                fs.close();
                LogWarn("ABuffer info @ " << basePath + rndInt + "abuffer_linkListBuffer.txt")
            }

            delete data;
        }
    }
}

//////////////////////////////////////////////////////////////////////////

Inviwo_ABufferGL4::ABufferGLInteractionHandler::ABufferGLInteractionHandler(
    Inviwo_ABufferGL4* parent)
    : mousePressEvent_(MouseEvent::MOUSE_BUTTON_LEFT, InteractionEvent::MODIFIER_NONE)
    , upEvent_('W', InteractionEvent::MODIFIER_CTRL, KeyboardEvent::KEY_STATE_PRESS)
    , downEvent_('S', InteractionEvent::MODIFIER_CTRL, KeyboardEvent::KEY_STATE_PRESS)
    , parent_(parent) {}

void Inviwo_ABufferGL4::ABufferGLInteractionHandler::invokeEvent(Event* event) {
    MouseEvent* mEvent = dynamic_cast<MouseEvent*>(event);
    if (mEvent) prevMousePos_ = glm::ivec2(mEvent->x(), mEvent->y());

    if (mEvent && (mEvent->button() & (MouseEvent::MOUSE_STATE_RELEASE)) &&
        (mEvent->button() & (MouseEvent::MOUSE_BUTTON_RIGHT)) &&
        (mEvent->modifiers() & MouseEvent::MODIFIER_CTRL)) {
        parent_->abuffer_printDebugInfo(glm::ivec2(mEvent->x(), mEvent->y()));
    }

    if (mEvent) prevMousePos_ = glm::ivec2(mEvent->x(), mEvent->y());
}

}  // namespace