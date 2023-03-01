/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <modules/meshrenderinggl/processors/transformrasterization.h>

#include <inviwo/core/processors/processor.h>                                 // for Processor
#include <inviwo/core/processors/processorinfo.h>                             // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                            // for CodeState
#include <inviwo/core/processors/processortags.h>                             // for Tags, Tags::GL
#include <modules/base/properties/transformlistproperty.h>                    // for TransformLi...
#include <modules/meshrenderinggl/datastructures/transformedrasterization.h>  // for Transformed...
#include <modules/meshrenderinggl/ports/rasterizationport.h>                  // for Rasterizati...

#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TransformRasterization::processorInfo_{
    "org.inviwo.TransformRasterization",  // Class identifier
    "Transform Rasterization",            // Display name
    "Mesh Rendering",                     // Category
    CodeState::Stable,                    // Code state
    Tags::GL,                             // Tags
};
const ProcessorInfo TransformRasterization::getProcessorInfo() const { return processorInfo_; }

TransformRasterization::TransformRasterization()
    : RasterizationProcessor()
    , inport_("input")
    , transformSetting_("transformSettings", "Additional Transform") {

    addPort(inport_);

    addProperties(transformSetting_);

    transformSetting_.setCollapsed(false);
}

void TransformRasterization::rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform,
                                       std::function<void(Shader&)> setUniforms,
                                       std::function<void(Shader&)> initializeShader) {

    if (auto p = inport_.getData()->getProcessor()) {
        p->rasterize(imageSize, transformSetting_.getMatrix() * worldMatrixTransform, setUniforms,
                     initializeShader);
    }
}
bool TransformRasterization::usesFragmentLists() const {
    return inport_.getData()->usesFragmentLists();
}

std::optional<mat4> TransformRasterization::boundingBox() const {
    if (auto bb = inport_.getData()->boundingBox()) {
        return transformSetting_.getMatrix() * (*bb);
    } else {
        return std::nullopt;
    }
}

Document TransformRasterization::getInfo() const { return inport_.getData()->getInfo(); }

}  // namespace inviwo
