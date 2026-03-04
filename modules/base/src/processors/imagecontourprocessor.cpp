/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2026 Inviwo Foundation
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

#include <modules/base/processors/imagecontourprocessor.h>

#include <inviwo/core/algorithm/markdown.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/constraintbehavior.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/base/algorithm/image/marchingsquares.h>

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageContourProcessor::processorInfo_{
    "org.inviwo.ImageContourProcessor",  // Class identifier
    "Image Contour",                     // Display name
    "Image Processing",                  // Category
    CodeState::Stable,                   // Code state
    Tags::CPU,                           // Tags
    R"(Extracts contour lines for each of the given isovalues in the input Image using marching squares.
    The output contour is provided as a line mesh.)"_unindentHelp};

const ProcessorInfo& ImageContourProcessor::getProcessorInfo() const { return processorInfo_; }

ImageContourProcessor::ImageContourProcessor()
    : Processor()
    , image_("image", "Input image"_help, OutportDeterminesSize::Yes)
    , outport_("mesh", "Contour mesh"_help)
    , channel_{"channel", "Channel", "Selected channel used to extract the contour"_help,
               util::enumeratedOptions("Channel", 4)}
    , isoValues_{"isovalues",
                 "Isovalues",
                 "Isovalues and corresponding colors for the contours"_help,
                 {{{.pos = 0.5, .color = vec4{1.0f}}}}} {

    addPorts(image_, outport_);
    addProperties(channel_, isoValues_);
}

void ImageContourProcessor::process() {
    auto mesh =
        util::marchingSquares(image_.getData()->getColorLayer()->getRepresentation<LayerRAM>(),
                              isoValues_.get(), channel_);
    outport_.setData(mesh);
}

}  // namespace inviwo
