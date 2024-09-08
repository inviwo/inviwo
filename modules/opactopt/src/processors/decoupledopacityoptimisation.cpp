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

#include <modules/opactopt/processors/decoupledopacityoptimisation.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <modules/oit/ports/rasterizationport.h>  // for RasterizationOutport
#include <modules/opengl/shader/shaderutils.h>    // for ImageInport, setUnif...

#include <iostream>
#include <fstream>
#include <rapidjson/document.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DecoupledOpacityOptimisation::processorInfo_{
    "org.inviwo.DecoupledOpacityOptimisation",  // Class identifier
    "Decoupled Opacity Optimisation",           // Display name
    "Mesh Rendering",                           // Category
    CodeState::Experimental,                    // Code state
    Tags::GL,                                   // Tags
    "Performs decoupled opacity optimisation by sorting"
    " fragment lists in an A-buffer. The processor takes a"
    "rasterisationas input,and optionally an importance "
    "volume and background texture. The output is an opacity"
    "optimised image."_help};

const ProcessorInfo DecoupledOpacityOptimisation::getProcessorInfo() const {
    return processorInfo_;
}

DecoupledOpacityOptimisation::DecoupledOpacityOptimisation()
    : q_{"q", "q", 1.0f, 0.0f, 10000.0f, 0.01f, InvalidationLevel::InvalidOutput, PropertySemantics::SpinBox}
    , r_{"r", "r", 1.0f, 0.0f, 10000.0f, 0.01f, InvalidationLevel::InvalidOutput, PropertySemantics::SpinBox}
    , lambda_{"lambda", "lambda", 1.0f, 0.0f, 100.0f, 0.01f, InvalidationLevel::InvalidOutput, PropertySemantics::SpinBox}
    , importanceVolume_{"importanceVolume", "Optional scalar field with importance data"_help} {

    flr_ = std::make_unique<DecoupledOpacityOptimisationRenderer>(&camera_);
    door_ = dynamic_cast<DecoupledOpacityOptimisationRenderer*>(flr_.get());

    addPort(importanceVolume_).setOptional(true);

    illustrationSettings_.enabled_.setChecked(false);
    removeProperty(illustrationSettings_.enabled_);
    addProperties(q_, r_, lambda_);

    q_.onChange([this]() { door_->q = q_; });
    r_.onChange([this]() { door_->r = r_; });
    lambda_.onChange([this]() { door_->lambda = lambda_; });

    importanceVolume_.onChange([this]() {
        if (importanceVolume_.isReady())
            door_->importanceVolume = &importanceVolume_;
        else {
            door_->importanceVolume = nullptr;
        }
        door_->importanceVolumeDirty = true;
    });
}
}  // namespace inviwo
