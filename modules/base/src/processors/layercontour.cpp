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

#include <modules/base/processors/layercontour.h>

#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/util/exception.h>
#include <modules/base/algorithm/image/layercontour.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerContour::processorInfo_{
    "org.inviwo.LayerContour",  // Class identifier
    "Layer Contour",            // Display name
    "Layer Processing",         // Category
    CodeState::Experimental,    // Code state
    Tags::CPU | Tag{"Layer"},   // Tags
    R"(Extracts a contour line for a specific value in the input Layer using marching squares.
    The output contour is provided as a line mesh.)"_unindentHelp,
};

const ProcessorInfo& LayerContour::getProcessorInfo() const { return processorInfo_; }

LayerContour::LayerContour()
    : Processor{}
    , inport_{"inport", "Input layer"_help}
    , outport_("mesh", "Contour mesh"_help)
    , channel_("channel", "Channel", "Selected channel used to extract the contour"_help,
               util::enumeratedOptions("Channel", 4))
    , isoValue_("iso", "Isovalue", "The isovalue of the contour"_help, 0.5,
                {0, ConstraintBehavior::Ignore}, {1, ConstraintBehavior::Ignore})
    , color_("color", "Color", util::ordinalColor(vec4(1.0)).set("The color of the contour"_help)) {

    addPorts(inport_, outport_);
    addProperties(channel_, isoValue_, color_);
}

void LayerContour::process() {
    if (auto components = inport_.getData()->getDataFormat()->getComponents();
        static_cast<int>(components) < channel_.get()) {
        throw Exception(IVW_CONTEXT, "Invalid channel {} selected, input Layer has {} channels.",
                        channel_.get() + 1, components);
    }

    auto mesh = computeLayerContour(inport_.getData()->getRepresentation<LayerRAM>(), channel_,
                                    isoValue_, color_);
    outport_.setData(mesh);
}

}  // namespace inviwo
