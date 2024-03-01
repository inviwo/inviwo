/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/basegl/processors/layerprocessing/layerglprocessor.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/shader/standardshaders.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>

#include <algorithm>

namespace inviwo {

LayerGLProcessor::LayerGLProcessor(Shader shader)
    : Processor()
    , inport_("inport", "The input layer"_help)
    , outport_("outport", "Resulting output layer"_help)
    , shader_(std::move(shader))
    , config{} {

    addPorts(inport_, outport_);
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

LayerGLProcessor::LayerGLProcessor(std::shared_ptr<const ShaderResource> fragmentShader)
    : LayerGLProcessor(Shader{{utilgl::imgQuadVert(), {ShaderType::Fragment, fragmentShader}},
                              Shader::Build::No}) {}

void LayerGLProcessor::initializeResources() { shader_.build(); }

void LayerGLProcessor::process() {
    const auto& input = *inport_.getData();
    if (const auto newConfig = outputConfig(input); config != newConfig) {
        cache_.clear();
        config = newConfig;
    }

    auto&& [fbo, layer] = [&]() -> decltype(auto) {
        auto unusedIt = std::ranges::find_if(
            cache_, [](const std::pair<FrameBufferObject, std::shared_ptr<Layer>>& item) {
                return item.second.use_count() == 1;
            });
        if (unusedIt != cache_.end()) {
            return *unusedIt;
        } else {
            auto& item = cache_.emplace_back(FrameBufferObject{}, std::make_shared<Layer>(config));
            auto* layerGL = item.second->getEditableRepresentation<LayerGL>();
            utilgl::Activate activateFbo{&item.first};
            item.first.attachColorTexture(layerGL->getTexture().get(), 0);
            return item;
        }
    }();

    utilgl::Activate activateShader{&shader_};
    utilgl::setShaderUniforms(shader_, *layer, "outportParameters");

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, *inport_.getData(), "inport");

    preProcess(cont, input, *layer);

    const auto dim = layer->getDimensions();
    {
        utilgl::Activate activateFbo{&fbo};
        utilgl::ViewportState viewport{0, 0, static_cast<GLsizei>(dim.x),
                                       static_cast<GLsizei>(dim.y)};
        utilgl::DepthMaskState depthMask{GL_FALSE};
        utilgl::GlBoolState depthTest{GL_DEPTH_TEST, false};

        // We always need to ask for an editable representation, this will invalidate any other
        // representations
        layer->getEditableRepresentation<LayerGL>();
        utilgl::singleDrawImagePlaneRect();
    }

    postProcess(input, *layer);

    outport_.setData(layer);
}

LayerConfig LayerGLProcessor::outputConfig([[maybe_unused]] const Layer& input) const {
    return input.config();
}
void LayerGLProcessor::preProcess(TextureUnitContainer&, const Layer&, Layer&) {}
void LayerGLProcessor::postProcess(const Layer&, Layer&) {}

}  // namespace inviwo
