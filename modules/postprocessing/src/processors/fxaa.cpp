/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/postprocessing/processors/fxaa.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/sharedopenglresources.h>
#include <modules/opengl/geometry/meshgl.h>
#include <random>

static void newTexture(GLuint &id) {
    if (id) glDeleteTextures(1, &id);
    glGenTextures(1, &id);
}

static void newFramebuffer(GLuint &id) {
    if (id) glDeleteFramebuffers(1, &id);
    glGenFramebuffers(1, &id);
}

static void delTexture(GLuint id) {
    if (id) glDeleteTextures(1, &id);
}

static void delFramebuffer(GLuint id) {
    if (id) glDeleteFramebuffers(1, &id);
}

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo FXAA::processorInfo_{
    "org.inviwo.FXAA",  // Class identifier
    "FXAA",             // Display name
    "Postprocessing",   // Category
    CodeState::Stable,  // Code state
    Tags::GL,           // Tags
};

const ProcessorInfo FXAA::getProcessorInfo() const { return processorInfo_; }

FXAA::FXAA()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , enable_("enable", "Enable Operation", true)
    , dither_("dither", "Dither")
    , quality_("quality", "Quality", 0.5f, 0.f, 1.f, 0.1f)
    , fxaa_("fullscreenquad.vert", "fxaa.frag", false)
    , prepass_("fullscreenquad.vert", "rgbl.frag", true) {
    addPort(inport_);
    addPort(outport_);

    addProperty(enable_);
    addProperty(dither_);
    addProperty(quality_);

    dither_.addOption("medium", "Medium (Default)", 1);
    dither_.addOption("low", "Low", 2);
    dither_.addOption("off", "Off (Expensive)", 3);
    dither_.setCurrentStateAsDefault();

    auto width = static_cast<int>(outport_.getDimensions().x);
    auto height = static_cast<int>(outport_.getDimensions().y);
    initFramebuffer(width, height);

    initializeResources();
    fxaa_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    dither_.onChange([this]() { invalidate(InvalidationLevel::InvalidResources); });
    quality_.onChange([this]() { invalidate(InvalidationLevel::InvalidResources); });
    inport_.onChange([this]() {
        const DataFormatBase *format = inport_.getData()->getDataFormat();
        const auto swizzleMask = inport_.getData()->getColorLayer()->getSwizzleMask();

        if (!outport_.hasEditableData() || format != outport_.getData()->getDataFormat() ||
            swizzleMask != outport_.getData()->getColorLayer()->getSwizzleMask()) {
            auto dim = outport_.getData()->getDimensions();
            Image *img = new Image(dim, format);
            img->copyMetaDataFrom(*inport_.getData());
            // forward swizzle mask of the input
            img->getColorLayer()->setSwizzleMask(swizzleMask);

            outport_.setData(img);
        }
    });
}

FXAA::~FXAA() {
    delTexture(prepassFbo_.tex);
    delFramebuffer(prepassFbo_.fbo);
}

void FXAA::initializeResources() {
    auto dither = dither_.get();
    auto quality = quality_.get();

    if (dither == 1) {
        quality *= 5.f;
    } else if (dither == 2) {
        quality *= 9.f;
    } else if (dither == 3) {
        quality = 9.f;
    }

    auto quality_str = std::to_string(dither * 10 + static_cast<int>(quality));

    auto frag = fxaa_.getShaderObject(ShaderType::Fragment);
    frag->addShaderDefine("FXAA_PC", "1");
    frag->addShaderDefine("FXAA_GLSL_130", "1");
    frag->addShaderDefine("FXAA_GREEN_AS_LUMA", "0");  // SET TO 0 LATER WHEN PREPASS IS IN PLACE
    frag->addShaderDefine("FXAA_QUALITY__PRESET", quality_str);

    fxaa_.build();
}

void FXAA::process() {
    if (!enable_.get()) {
        outport_.setData(inport_.getData());
        return;
    }
    int width = static_cast<int>(outport_.getDimensions().x);
    int height = static_cast<int>(outport_.getDimensions().y);

    if (width != prepassFbo_.width || height != prepassFbo_.height) {
        initFramebuffer(width, height);
    }

    // Copy depth and picking
    utilgl::activateTargetAndCopySource(outport_, inport_);

    auto outImageGL = outport_.getEditableData()->getRepresentation<ImageGL>();
    auto outFbo = outImageGL->getFBO()->getID();

    auto imageGL = inport_.getData()->getRepresentation<ImageGL>();
    auto colorLayer = imageGL->getColorLayerGL();
    auto colorTex = colorLayer->getTexture()->getID();

    // This geometry is actually never used in the shader
    auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
    utilgl::Enable<MeshGL> enable(rect);

    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);

    // --- PREPASS TO RENDER RGB-LUMINANCE ---
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glBindFramebuffer(GL_FRAMEBUFFER, prepassFbo_.fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    prepass_.activate();
    prepass_.setUniform("tex_", 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    prepass_.deactivate();

    // --- FXAA PASS ---
    glColorMask(1, 1, 1, 0);
    glBindTexture(GL_TEXTURE_2D, prepassFbo_.tex);
    glBindFramebuffer(GL_FRAMEBUFFER, outFbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    fxaa_.activate();
    fxaa_.setUniform("tex_", 0);
    fxaa_.setUniform("oneOverRes_",
                     vec2(1.f / static_cast<float>(width), 1.f / static_cast<float>(height)));
    glDrawArrays(GL_TRIANGLES, 0, 3);
    fxaa_.deactivate();

    glColorMask(1, 1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
}

void FXAA::initFramebuffer(int width, int height) {
    prepassFbo_.width = width;
    prepassFbo_.height = height;

    newTexture(prepassFbo_.tex);
    glBindTexture(GL_TEXTURE_2D, prepassFbo_.tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    newFramebuffer(prepassFbo_.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, prepassFbo_.fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, prepassFbo_.tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace inviwo
