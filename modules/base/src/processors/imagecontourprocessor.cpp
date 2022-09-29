/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

#include <inviwo/core/algorithm/markdown.h>                             // for operator""_help
#include <inviwo/core/datastructures/image/layer.h>                     // for Layer
#include <inviwo/core/datastructures/image/layerram.h>                  // for LayerRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/ports/imageport.h>                                // for ImageInport, Outp...
#include <inviwo/core/ports/meshport.h>                                 // for MeshOutport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::CPU
#include <inviwo/core/properties/constraintbehavior.h>                  // for ConstraintBehavior
#include <inviwo/core/properties/ordinalproperty.h>                     // for IntSizeTProperty
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                    // for vec4
#include <modules/base/algorithm/image/imagecontour.h>                  // for ImageContour

#include <memory>                                                       // for shared_ptr, uniqu...
#include <string>                                                       // for string
#include <string_view>                                                  // for string_view
#include <type_traits>                                                  // for remove_extent_t
#include <unordered_set>                                                // for unordered_set

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageContourProcessor::processorInfo_{
    "org.inviwo.ImageContourProcessor",  // Class identifier
    "Image Contour",                     // Display name
    "Image Processing",                  // Category
    CodeState::Experimental,             // Code state
    Tags::CPU,                           // Tags
    R"(Extracts a contour line for a specific value in the image using marching squares.
    The output contour is provided as a line mesh.)"_unindentHelp};

const ProcessorInfo ImageContourProcessor::getProcessorInfo() const { return processorInfo_; }

ImageContourProcessor::ImageContourProcessor()
    : Processor()
    , image_("image", "Input image"_help, OutportDeterminesSize::Yes)
    , mesh_("mesh", "Contour mesh"_help)
    , channel_("channel", "Channel", "The image channel used to extract the contour"_help, 0,
               {0, ConstraintBehavior::Immutable}, {4, ConstraintBehavior::Editable})
    , isoValue_("iso", "ISO Value", "The contour iso value"_help, 0.5,
                {0, ConstraintBehavior::Ignore}, {1, ConstraintBehavior::Ignore})
    , color_("color", "Color", util::ordinalColor(vec4(1.0)).set("The contour color"_help)) {

    addPorts(image_, mesh_);
    addProperties(channel_, isoValue_, color_);
}

void ImageContourProcessor::process() {
    if (image_.isChanged()) {
        auto max = image_.getData()->getDataFormat()->getComponents() - 1;
        channel_.setMaxValue(max);
    }
    mesh_.setData(
        ImageContour::apply(image_.getData()->getColorLayer()->getRepresentation<LayerRAM>(),
                            channel_, isoValue_, color_));
}

}  // namespace inviwo
