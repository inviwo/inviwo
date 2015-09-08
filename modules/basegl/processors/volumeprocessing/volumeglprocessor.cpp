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

#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>

namespace inviwo {

VolumeGLProcessor::VolumeGLProcessor(std::string fragmentShader)
    : Processor()
    , inport_(fragmentShader + "inport")
    , outport_(fragmentShader + "outport")
    , dataFormat_(nullptr)
    , internalInvalid_(true)
    , fragmentShader_(fragmentShader)
    , shader_("volume_gpu.vert", "volume_gpu.geom", fragmentShader_, true)
    , fbo_() {
    addPort(inport_);
    addPort(outport_);

    inport_.onChange(this, &VolumeGLProcessor::inportChanged);
    shader_.onReload([this]() { invalidate(INVALID_RESOURCES); });
}

VolumeGLProcessor::~VolumeGLProcessor() {}

void VolumeGLProcessor::process() {
    bool reattach = false;

    if (internalInvalid_) {
        reattach = true;
        internalInvalid_ = false;
        const DataFormatBase* format = dataFormat_?dataFormat_:inport_.getData()->getDataFormat();
        volume_ = std::make_shared<Volume>(inport_.getData()->getDimensions(), format);
        volume_->setModelMatrix(inport_.getData()->getModelMatrix());
        volume_->setWorldMatrix(inport_.getData()->getWorldMatrix());
        // pass meta data on
        volume_->copyMetaDataFrom(*inport_.getData());
        volume_->dataMap_ = inport_.getData()->dataMap_;
        outport_.setData(volume_);
    }

    shader_.activate();

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, *inport_.getData(), "volume");

    preProcess();

    const size3_t dim{inport_.getData()->getDimensions()};
    fbo_.activate();
    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));
    if (reattach) {
        VolumeGL* outVolumeGL = volume_->getEditableRepresentation<VolumeGL>();
        fbo_.attachColorTexture(outVolumeGL->getTexture().get(), 0);
    }

    utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));

    shader_.deactivate();
    fbo_.deactivate();

    postProcess();
}

void VolumeGLProcessor::markInvalid() { internalInvalid_ = true; }

void VolumeGLProcessor::inportChanged() {
    markInvalid();
    afterInportChanged();
}

}  // namespace
