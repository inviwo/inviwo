/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <modules/plottinggl/processors/layeraxis.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/util/foreacharg.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerAxis::processorInfo_{"org.inviwo.LayerAxis",      // Class identifier
                                              "Layer Axis",                // Display name
                                              "Plotting",                  // Category
                                              CodeState::Stable,           // Code state
                                              Tags::GL | Tag{"Plotting"},  // Tags
                                              "Renders x and y axes next to the layer."_help};

const ProcessorInfo& LayerAxis::getProcessorInfo() const { return processorInfo_; }

LayerAxis::LayerAxis()
    : Processor{}
    , inport_{"layer", "Input layer"_help}
    , imageInport_{"imageInport", "Background image (optional)"_help}
    , outport_{"outport",
               "Output image containing the rendered layer axes and the optional input image"_help}
    , axisHelper_{util::boundingBox(inport_), Axis2DProcessorHelper::DimsRangeMode::Yes} {

    imageInport_.setOptional(true);

    addPorts(inport_, imageInport_, outport_);

    util::for_each_in_tuple([&](Property& p) { addProperty(p); }, axisHelper_.props());

    // adjust scaling factor for label offsets and tick lengths
    axisHelper_.offsetScaling_.onChange(
        [&]() { axisHelper_.adjustScalingFactor(inport_.getData().get()); });

    // adjust axis ranges when input mesh, i.e. its basis, changes
    inport_.onChange([&]() {
        axisHelper_.adjustScalingFactor(inport_.getData().get());
        axisHelper_.adjustRanges(inport_.getData().get());
    });
    // sync ranges when custom range is enabled or disabled
    axisHelper_.rangeMode_.onChange(
        [this]() { axisHelper_.adjustRanges(inport_.getData().get()); });

    setAllPropertiesCurrentStateAsDefault();
}

void LayerAxis::process() {
    if (imageInport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, imageInport_, ImageType::ColorDepth);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepth);
    }

    axisHelper_.renderAxes(outport_.getDimensions(), *inport_.getData());

    utilgl::deactivateCurrentTarget();
}

}  // namespace plot

}  // namespace inviwo
