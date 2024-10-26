/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/image.h>       // for Image
#include <inviwo/core/datastructures/image/imagetypes.h>  // for ImageType, ImageType::ColorOnly
#include <inviwo/core/datastructures/image/layer.h>       // for Layer
#include <inviwo/core/ports/imageport.h>                  // for ImageOutport, ImageInport
#include <inviwo/core/processors/processor.h>             // for Processor
#include <inviwo/core/processors/processorinfo.h>         // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>        // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>         // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>          // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>       // for FloatProperty
#include <inviwo/core/util/formats.h>                     // for DataFormatId, DataFormatId::Vec...
#include <inviwo/core/util/glmvec.h>                      // for vec2, size2_t, ivec4
#include <inviwo/core/util/stdextensions.h>               // for make_array
#include <modules/opengl/buffer/framebufferobject.h>      // for FrameBufferObject
#include <modules/opengl/geometry/meshgl.h>               // for MeshGL
#include <modules/opengl/glformats.h>                     // for GLFormats
#include <modules/opengl/image/imagegl.h>                 // for ImageGL
#include <modules/opengl/image/layergl.h>                 // for LayerGL
#include <modules/opengl/inviwoopengl.h>                  // for glDrawArrays, glBindTexture
#include <modules/opengl/openglutils.h>                   // for BlendModeEquationState, Enable
#include <modules/opengl/shader/shader.h>                 // for Shader
#include <modules/opengl/sharedopenglresources.h>         // for SharedOpenGLResources
#include <modules/opengl/texture/texture2d.h>             // for Texture2D
#include <modules/opengl/texture/textureutils.h>          // for activateTarget, activateTargetA...

#include <cmath>        // for pow
#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <memory>       // for shared_ptr, shared_ptr<>::eleme...
#include <string>       // for operator+, string, to_string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

#include <glm/ext/vector_float2.hpp>          // for vec2
#include <glm/gtx/scalar_multiplication.hpp>  // for operator/
#include <glm/vec2.hpp>                       // for operator*, operator!=, operator/

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo HdrBloom::processorInfo_{
    "org.inviwo.HdrBloom",  // Class identifier
    "HDR Bloom",            // Display name
    "Postprocessing",       // Category
    CodeState::Stable,      // Code state
    Tags::GL,               // Tags
};

const ProcessorInfo& HdrBloom::getProcessorInfo() const { return processorInfo_; }

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
    , size_{512, 512}
    , horizontal_{util::make_array<levels_>([this](auto i) {
        return HdrBloom::FBOTex{
            {}, {size_ / pow(size_t{2}, i + 1), GL_RGBA, GL_RGBA16F, GL_HALF_FLOAT, GL_LINEAR}};
    })}
    , vertical_{util::make_array<levels_>([this](auto i) {
        return HdrBloom::FBOTex{
            {}, {size_ / pow(size_t{2}, i + 1), GL_RGBA, GL_RGBA16F, GL_HALF_FLOAT, GL_LINEAR}};
    })}
    , bright_{{}, {size_, GL_RGBA, GL_RGBA16F, GL_HALF_FLOAT, GL_LINEAR}} {

    addPort(inport_);
    addPort(outport_);

    addProperties(enable_, threshold_, strength_, radius_);

    for (int i = 0; i < levels_; i++) {
        horizontal_[i].fbo.activate();
        horizontal_[i].fbo.attachColorTexture(&horizontal_[i].tex);
        vertical_[i].fbo.activate();
        vertical_[i].fbo.attachColorTexture(&vertical_[i].tex);
    }
    bright_.fbo.activate();
    bright_.fbo.attachColorTexture(&bright_.tex);
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

    if (size_ != outport_.getDimensions()) {
        resizeTextures(outport_.getDimensions());
    }

    // Copy color, depth and picking
    utilgl::activateTargetAndCopySource(outport_, inport_);

    auto imageGL = inport_.getData()->getRepresentation<ImageGL>();
    auto colorLayer = imageGL->getColorLayerGL();
    auto colorTex = colorLayer->getTexture()->getID();

    // This geometry is actually never used in the shader
    const auto rect = SharedOpenGLResources::getPtr()->imagePlaneRect();
    utilgl::Enable<MeshGL> enable(rect);

    utilgl::GlBoolState depth(GL_DEPTH_TEST, false);
    glActiveTexture(GL_TEXTURE0);

    // --- HIGHPASS ---
    bright_.fbo.activate();
    utilgl::ViewportState viewport(ivec4(0, 0, bright_.tex.getWidth(), bright_.tex.getHeight()));
    glBindTexture(GL_TEXTURE_2D, colorTex);
    highPass_.activate();
    highPass_.setUniform("threshold", threshold_.get());
    highPass_.setUniform("texSource", 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // --- BLUR ---
    Texture2D* input_tex = &bright_.tex;
    blur_.activate();
    blur_.setUniform("texSource", 0);
    for (int i = 0; i < levels_; i++) {
        const auto inv_res = 1.f / vec2(input_tex->getWidth(), input_tex->getHeight());
        glViewport(0, 0, static_cast<GLsizei>(horizontal_[i].tex.getWidth()),
                   static_cast<GLsizei>(horizontal_[i].tex.getHeight()));

        // --- X-PASS ---
        input_tex->bind();
        horizontal_[i].fbo.activate();
        blur_.setUniform("direction", vec2(1, 0) * inv_res);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // --- Y-PASS ---
        horizontal_[i].tex.bind();
        vertical_[i].fbo.activate();
        blur_.setUniform("direction", vec2(0, 1) * inv_res);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        input_tex = &vertical_[i].tex;
    }

    // --- COMPOSE ---
    utilgl::activateTarget(outport_, ImageType::ColorOnly);
    compose_.activate();

    // Blend bloom results onto colorchannel of outport.
    utilgl::BlendModeEquationState blendEqn{GL_ONE, GL_ONE, GL_ONE, GL_ONE, GL_FUNC_ADD, GL_MAX};

    compose_.setUniform("bloomStrength", strength_.get());
    compose_.setUniform("bloomRadius", radius_.get());
    for (int i = 0; i < levels_; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        vertical_[i].tex.bind();
        compose_.setUniform("tex" + std::to_string(i), i);
    }
    glDrawArrays(GL_TRIANGLES, 0, 3);

    compose_.deactivate();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void HdrBloom::resizeTextures(size2_t size) {
    auto res = size / 2;

    bright_.tex.resize(size);

    for (int i = 0; i < levels_; i++) {
        horizontal_[i].tex.resize(res);
        vertical_[i].tex.resize(res);
        res /= 2;
    }
    size_ = size;
}

}  // namespace inviwo
