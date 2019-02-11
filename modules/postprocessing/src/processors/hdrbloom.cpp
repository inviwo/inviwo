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

#include <modules/postprocessing/processors/hdrbloom.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/sharedopenglresources.h>
#include <modules/opengl/geometry/meshgl.h>
#include <random>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo HdrBloom::processorInfo_{
    "org.inviwo.HdrBloom",  // Class identifier
    "HDR Bloom",            // Display name
    "Postprocessing",       // Category
    CodeState::Stable,      // Code state
    Tags::GL,               // Tags
};

const ProcessorInfo HdrBloom::getProcessorInfo() const { return processorInfo_; }

HdrBloom::HdrBloom()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , enable_("enable", "Enable Operation", true)
    , threshold_("threshold", "Threshold", 1.f, 0.f, 10.f)
    , strength_("strength", "Strength", 1.f, 0.f, 3.f)
    , radius_("radius", "Radius", 0.5f, 0.f, 1.f)
    , highPass_("fullscreenquad.vert", "bloomhighpass.frag")
    , blur_("fullscreenquad.vert", "bloomblur.frag")
    , compose_("fullscreenquad.vert", "bloomcompose.frag")
    , width_(0)
    , height_(0)
    , texBright_(size2_t(256, 256), GLFormats::get(DataFormatId::Vec4Float16), GL_LINEAR) {
    addPort(inport_);
    addPort(outport_);

    addProperty(enable_);
    addProperty(threshold_);
    addProperty(strength_);
    addProperty(radius_);

    for (int i = 0; i < Levels; i++) {
        texHorizontal_[i] = std::make_unique<Texture2D>(
            size2_t(256, 256), GLFormats::get(DataFormatId::Vec4Float16), GL_LINEAR);
        texVertical_[i] = std::make_unique<Texture2D>(
            size2_t(256, 256), GLFormats::get(DataFormatId::Vec4Float16), GL_LINEAR);
        fboHorizontal_[i].activate();
        fboHorizontal_[i].attachColorTexture(texHorizontal_[i].get());
        fboVertical_[i].activate();
        fboVertical_[i].attachColorTexture(texVertical_[i].get());
    }
    fboBright_.activate();
    fboBright_.attachColorTexture(&texBright_);
    FrameBufferObject::deactivateFBO();

    inport_.onChange([this]() {
        const DataFormatBase* format = inport_.getData()->getDataFormat();
        const auto swizzleMask = inport_.getData()->getColorLayer()->getSwizzleMask();

        if (!outport_.hasEditableData() || format != outport_.getData()->getDataFormat() ||
            swizzleMask != outport_.getData()->getColorLayer()->getSwizzleMask()) {
            auto dim = outport_.getData()->getDimensions();
            Image* img = new Image(dim, format);
            img->copyMetaDataFrom(*inport_.getData());
            // forward swizzle mask of the input
            img->getColorLayer()->setSwizzleMask(swizzleMask);

            outport_.setData(img);
        }
    });
}

HdrBloom::~HdrBloom() {}

void HdrBloom::process() {
    if (!enable_.get()) {
        outport_.setData(inport_.getData());
        return;
    }
    const int width = static_cast<int>(outport_.getDimensions().x);
    const int height = static_cast<int>(outport_.getDimensions().y);

    if (width != width_ || height != height_) {
        resizeTextures(width, height);
    }

    // Copy color, depth and picking
    utilgl::activateTargetAndCopySource(outport_, inport_);

    auto imageGL = inport_.getData()->getRepresentation<ImageGL>();
    auto colorLayer = imageGL->getColorLayerGL();
    auto colorTex = colorLayer->getTexture()->getID();

    // This geometry is actually never used in the shader
    auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
    utilgl::Enable<MeshGL> enable(rect);

    utilgl::GlBoolState depth(GL_DEPTH_TEST, false);
    glActiveTexture(GL_TEXTURE0);

    // --- HIGHPASS ---
    fboBright_.activate();
    utilgl::ViewportState(ivec4(0, 0, texBright_.getWidth(), texBright_.getHeight()));
    glBindTexture(GL_TEXTURE_2D, colorTex);
    highPass_.activate();
    highPass_.setUniform("threshold", threshold_.get());
    highPass_.setUniform("texSource", 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // --- BLUR ---
    Texture2D* input_tex = &texBright_;
    blur_.activate();
    blur_.setUniform("texSource", 0);
    for (int i = 0; i < Levels; i++) {
        auto inv_res = 1.f / vec2(input_tex->getWidth(), input_tex->getHeight());
        glViewport(0, 0, static_cast<GLsizei>(texHorizontal_[i]->getWidth()),
                   static_cast<GLsizei>(texHorizontal_[i]->getHeight()));

        // --- X-PASS ---
        input_tex->bind();
        fboHorizontal_[i].activate();
        blur_.setUniform("direction", vec2(1, 0) * inv_res);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // --- Y-PASS ---
        texHorizontal_[i]->bind();
        fboVertical_[i].activate();
        blur_.setUniform("direction", vec2(0, 1) * inv_res);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        input_tex = texVertical_[i].get();
    }

    // --- COMPOSE ---
    utilgl::activateTarget(outport_, ImageType::ColorOnly);
    compose_.activate();

    // Blend bloom results onto colorchannel of outport.
    utilgl::GlBoolState blend(GL_BLEND, true);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);

    compose_.setUniform("bloomStrength", strength_.get());
    compose_.setUniform("bloomRadius", radius_.get());
    for (int i = 0; i < Levels; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        texVertical_[i]->bind();
        compose_.setUniform("tex" + std::to_string(i), i);
    }
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBlendEquation(GL_FUNC_ADD);

    compose_.deactivate();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void HdrBloom::resizeTextures(int width, int height) {
    int res_x = width / 2;
    int res_y = height / 2;

    texBright_.resize(size2_t(width, height));

    for (int i = 0; i < Levels; i++) {
        texHorizontal_[i]->resize(size2_t(res_x, res_y));
        texVertical_[i]->resize(size2_t(res_x, res_y));
        res_x /= 2;
        res_y /= 2;
    }

    width_ = width;
    height_ = height;
}

}  // namespace inviwo
