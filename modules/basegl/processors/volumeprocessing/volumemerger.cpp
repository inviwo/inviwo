/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "volumemerger.h"
#include <modules/opengl/volume/volumeutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeMerger::processorInfo_{
    "org.inviwo.VolumeMerger",  // Class identifier
    "Volume Merger",            // Display name
    "Undefined",                // Category
    CodeState::Experimental,    // Code state
    Tags::None,                 // Tags
};

const ProcessorInfo VolumeMerger::getProcessorInfo() const { return processorInfo_; }


VolumeMerger::VolumeMerger()
    : VolumeGLProcessor("volumemerger.frag"), vol2_("vol2"), vol3_("vol3"), vol4_("vol4") {
    addPort(vol2_);
    addPort(vol3_);
    addPort(vol4_);

    vol2_.setOptional(true);
    vol3_.setOptional(true);
    vol4_.setOptional(true);

    inport_.onChange([&]() {this->onVolChange(); });
    vol2_.onChange([&]() {this->onVolChange(); });
    vol3_.onChange([&]() {this->onVolChange(); });
    vol4_.onChange([&]() {this->onVolChange(); });
}


void VolumeMerger::onVolChange() {

    auto channels = 1;

    auto df = inport_.getData()->getDataFormat();

    if (vol2_.hasData()) {
        channels++;
        shader_.getFragmentShaderObject()->addShaderDefine("HAS_VOL2");
    }
    else {
        shader_.getFragmentShaderObject()->removeShaderDefine("HAS_VOL2");
    }
    if (vol3_.hasData()) {
        channels++;
        shader_.getFragmentShaderObject()->addShaderDefine("HAS_VOL3");
    }
    else {
        shader_.getFragmentShaderObject()->removeShaderDefine("HAS_VOL3");
    }
    if (vol4_.hasData()) {
        channels++;
        shader_.getFragmentShaderObject()->addShaderDefine("HAS_VOL4");
    }
    else {
        shader_.getFragmentShaderObject()->removeShaderDefine("HAS_VOL4");
    }
    shader_.build();

    dataFormat_ = DataFormatBase::get(df->getNumericType(), channels, df->getSize()*8);
    internalInvalid_ = true;
}

void VolumeMerger::postProcess()
{
    volume_->dataMap_.dataRange = dvec2(0, 1);
}

void VolumeMerger::preProcess(TextureUnitContainer &cont) {
    if (vol2_.hasData()) {
        utilgl::bindAndSetUniforms(shader_, cont, *vol2_.getData(), "vol2");
    }
    if (vol3_.hasData()) {
        utilgl::bindAndSetUniforms(shader_, cont, *vol3_.getData(), "vol3");
    }
    if (vol4_.hasData()) {
        utilgl::bindAndSetUniforms(shader_, cont, *vol4_.getData(), "vol4");
    }
}

}  // namespace
