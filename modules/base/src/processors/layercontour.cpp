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
#include <modules/base/algorithm/image/marchingsquares.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerContour::processorInfo_{
    "org.inviwo.LayerContour",                           // Class identifier
    "Layer Contour",                                     // Display name
    "Layer Processing",                                  // Category
    CodeState::Stable,                                   // Code state
    Tags::CPU | Tag{"Layer"} | Tag{"Marching Squares"},  // Tags
    R"(Extracts contour lines for each of the given isovalues in the input Layer using marching squares.
    The output contour is provided as a line mesh.)"_unindentHelp,
};

const ProcessorInfo& LayerContour::getProcessorInfo() const { return processorInfo_; }

LayerContour::LayerContour()
    : Processor{}
    , inport_{"inport", "Input layer"_help}
    , outport_{"mesh", "Contour mesh"_help}
    , channel_{"channel", "Channel", "Selected channel used to extract the contour"_help,
               util::enumeratedOptions("Channel", 4)}
    , isoValues_{"isovalues",
                 "Isovalues",
                 "Isovalues and corresponding colors for the contours"_help,
                 {{{.pos = 0.5, .color = vec4{1.0f}}}},
                 TFData{&inport_}} {

    addPorts(inport_, outport_);
    addProperties(channel_, isoValues_);
}

void LayerContour::process() {
    auto mesh = util::marchingSquares(inport_.getData()->getRepresentation<LayerRAM>(),
                                      isoValues_.get(), channel_);
    outport_.setData(mesh);
}

}  // namespace inviwo
