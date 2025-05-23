/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/image.h>    // for Image
#include <inviwo/core/datastructures/image/layer.h>    // for Layer
#include <inviwo/core/ports/imageport.h>               // for ImageOutport, ImageInport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>     // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>      // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>       // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, InvalidationLev...
#include <inviwo/core/properties/optionproperty.h>     // for OptionPropertyInt
#include <inviwo/core/properties/ordinalproperty.h>    // for FloatProperty
#include <inviwo/core/util/glmvec.h>                   // for size2_t, vec2
#include <modules/opengl/buffer/framebufferobject.h>   // for FrameBufferObject
#include <modules/opengl/geometry/meshgl.h>            // for MeshGL
#include <modules/opengl/image/imagegl.h>              // for ImageGL
#include <modules/opengl/image/layergl.h>              // for LayerGL
#include <modules/opengl/openglutils.h>                // for Enable
#include <modules/opengl/shader/shader.h>              // for Shader, Shader::Build
#include <modules/opengl/shader/shaderobject.h>        // for ShaderObject
#include <modules/opengl/shader/shadertype.h>          // for ShaderType, ShaderType::Fragment
#include <modules/opengl/sharedopenglresources.h>      // for SharedOpenGLResources
#include <modules/opengl/texture/textureutils.h>       // for activateTargetAndCopySource
#include <modules/opengl/texture/texture2d.h>          // IWYU pragma: keep

#include <array>        // for operator!=, array
#include <functional>   // for __base
#include <memory>       // for shared_ptr, shared_ptr<>::element_...
#include <string>       // for string, to_string, basic_string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

#include <glm/vec2.hpp>  // for vec<>::(anonymous)

namespace inviwo {
class DataFormatBase;

namespace {

void newTexture(GLuint& id) {
    if (id) glDeleteTextures(1, &id);
    glGenTextures(1, &id);
}

void newFramebuffer(GLuint& id) {
    if (id) glDeleteFramebuffers(1, &id);
    glGenFramebuffers(1, &id);
}

void delTexture(GLuint id) {
    if (id) glDeleteTextures(1, &id);
}

void delFramebuffer(GLuint id) {
    if (id) glDeleteFramebuffers(1, &id);
}

}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo FXAA::processorInfo_{
    "org.inviwo.FXAA",  // Class identifier
    "FXAA",             // Display name
    "Postprocessing",   // Category
    CodeState::Stable,  // Code state
    Tags::GL,           // Tags
    "Applies Fast approximate anti-aliasing (FXAA) as a postprocessing operation"_help,
};

const ProcessorInfo& FXAA::getProcessorInfo() const { return processorInfo_; }

FXAA::FXAA()
    : Processor()
    , inport_("inport", "Input image."_help)
    , outport_("outport", "Output image."_help)
    , enable_("enable", "Enable Operation", true)
    , dither_("dither", "Dither", "Sets amount of dithering."_help,
              {{"medium", "Medium (Default)", 1}, {"low", "Low", 2}, {"off", "Off (Expensive)", 3}})
    , quality_("quality", "Quality",
               util::ordinalLength(0.5f, 1.0f)
                   .set("Sets the quality (number of samples) used. Performance vs. Quality"_help))
    , fxaa_("fullscreenquad.vert", "fxaa.frag", Shader::Build::No)
    , prepass_("fullscreenquad.vert", "rgbl.frag", Shader::Build::Yes) {
    addPort(inport_);
    addPort(outport_);

    addProperties(enable_, dither_, quality_);

    auto width = static_cast<int>(outport_.getDimensions().x);
    auto height = static_cast<int>(outport_.getDimensions().y);
    initFramebuffer(width, height);

    initializeResources();
    fxaa_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    dither_.onChange([this]() { invalidate(InvalidationLevel::InvalidResources); });
    quality_.onChange([this]() { invalidate(InvalidationLevel::InvalidResources); });
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
