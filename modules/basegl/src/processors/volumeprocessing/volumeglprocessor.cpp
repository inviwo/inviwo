/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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

#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>

namespace inviwo {

VolumeGLProcessor::VolumeGLProcessor(std::shared_ptr<const ShaderResource> fragmentShader,
                                     bool buildShader)
    : Processor()
    , inport_("inputVolume", "Input volume"_help)
    , outport_("outputVolume", "Output volume"_help)
    , dataFormat_(nullptr)
    , internalInvalid_(true)
    , shader_({{ShaderType::Vertex, utilgl::findShaderResource("volume_gpu.vert")},
               {ShaderType::Geometry, utilgl::findShaderResource("volume_gpu.geom")},
               {ShaderType::Fragment, fragmentShader}},
              buildShader ? Shader::Build::Yes : Shader::Build::No)
    , fbo_() {
    addPorts(inport_, outport_);

    inport_.onChange([this]() {
        markInvalid();
        afterInportChanged();
    });
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

VolumeGLProcessor::VolumeGLProcessor(const std::string& fragmentShader, bool buildShader)
    : VolumeGLProcessor(utilgl::findShaderResource(fragmentShader), buildShader) {}

VolumeGLProcessor::~VolumeGLProcessor() = default;

void VolumeGLProcessor::process() {
    bool reattach = false;

    if (internalInvalid_) {
        reattach = true;
        internalInvalid_ = false;
        volume_ = std::make_shared<Volume>(*inport_.getData(), noData);
        if (dataFormat_) volume_->setDataFormat(dataFormat_);
        outport_.setData(volume_);
    }

    shader_.activate();

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, *inport_.getData(), "volume");

    preProcess(cont);

    const size3_t dim{inport_.getData()->getDimensions()};
    fbo_.activate();
    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));

    // We always need to ask for a editable representation
    // this will invalidate any other representations
    VolumeGL* outVolumeGL = volume_->getEditableRepresentation<VolumeGL>();
    if (reattach) {
        fbo_.attachColorTexture(outVolumeGL->getTexture().get(), 0);
    }

    utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));

    shader_.deactivate();
    fbo_.deactivate();

    postProcess();
}

void VolumeGLProcessor::markInvalid() { internalInvalid_ = true; }

void VolumeGLProcessor::preProcess(TextureUnitContainer&) {}

void VolumeGLProcessor::postProcess() {}

void VolumeGLProcessor::afterInportChanged() {}

}  // namespace inviwo
