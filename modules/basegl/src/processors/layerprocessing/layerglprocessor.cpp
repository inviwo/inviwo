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

namespace inviwo {

LayerGLProcessor::LayerGLProcessor(std::shared_ptr<const ShaderResource> fragmentShader,
                                   Shader::Build build)
    : Processor()
    , inport_("inport", "The input layer"_help)
    , outport_("output", "Resulting output layer"_help)
    , dataFormat_(nullptr)
    , swizzleMask_(swizzlemasks::rgba)
    , internalInvalid_(false)
    , shader_({utilgl::imgQuadVert(), {ShaderType::Fragment, fragmentShader}}, build) {

    addPorts(inport_, outport_);

    inport_.onChange([this]() {
        markInvalid();
        afterInportChanged();
    });

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

void LayerGLProcessor::initializeResources() {
    shader_.build();
    internalInvalid_ = true;
}

void LayerGLProcessor::process() {
    bool reattach = false;

    if (internalInvalid_) {
        reattach = true;
        internalInvalid_ = false;
        layer_ = std::make_shared<Layer>(*inport_.getData(), noData, std::nullopt, dataFormat_);
        if (dataFormat_) {
            layer_->setSwizzleMask(swizzleMask_);
        }
        outport_.setData(layer_);
    }

    shader_.activate();
    utilgl::setShaderUniforms(shader_, outport_, "outportParameters");

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, *inport_.getData(), "inport");

    preProcess(cont);

    const size2_t dim{inport_.getData()->getDimensions()};
    fbo_.activate();
    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));

    // We always need to ask for an editable representation, this will invalidate any other
    // representations
    LayerGL* destLayer = layer_->getEditableRepresentation<LayerGL>();
    if (reattach) {
        fbo_.attachColorTexture(destLayer->getTexture().get(), 0);
    }

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    fbo_.deactivate();

    postProcess();
}

void LayerGLProcessor::markInvalid() { internalInvalid_ = true; }

void LayerGLProcessor::preProcess(TextureUnitContainer&) {}

void LayerGLProcessor::postProcess() {}

void LayerGLProcessor::afterInportChanged() {}

}  // namespace inviwo
