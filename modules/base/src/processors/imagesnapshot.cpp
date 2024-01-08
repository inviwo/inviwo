/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <modules/base/processors/imagesnapshot.h>

#include <inviwo/core/datastructures/image/image.h>  // for Image
#include <inviwo/core/ports/imageport.h>             // for ImageOutport, ImageInport
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>   // for CodeState, CodeState::Experimental
#include <inviwo/core/processors/processortags.h>    // for Tags, Tags::None
#include <inviwo/core/properties/buttonproperty.h>   // for ButtonProperty
#include <inviwo/core/properties/ordinalproperty.h>  // for IntProperty

#include <functional>   // for __base
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

namespace inviwo {
class Deserializer;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageSnapshot::processorInfo_{
    "org.inviwo.ImageSnapshot",  // Class identifier
    "Image Snapshot",            // Display name
    "Image Operation",           // Category
    CodeState::Experimental,     // Code state
    Tags::None,                  // Tags
    R"(Save snapshot of images that can be viewed later. Useful for comparisons.)"_unindentHelp};
const ProcessorInfo ImageSnapshot::getProcessorInfo() const { return processorInfo_; }

ImageSnapshot::ImageSnapshot()
    : Processor()
    , inport_("inport_", "Input image"_help)
    , outport1_("outport1", "Outputs the input image or a saved image"_help)
    , outport2_("outport2", "Outputs the input image or a saved image"_help)
    , outport1ImageIndex_("outport1ImageIndex", "Image 1 index",
                          "The image to output on outport1, -1 means input pass through"_help, -1,
                          {-1, ConstraintBehavior::Immutable}, {-1, ConstraintBehavior::Mutable})
    , outport2ImageIndex_("outport2ImageIndex", "Image 2 index",
                          "The image to output on outport2, -1 means input pass through."_help, -1,
                          {-1, ConstraintBehavior::Immutable}, {-1, ConstraintBehavior::Mutable})
    , snapshot_("snapshot", "Snapshot")
    , clear_("clear", "Clear") {
    addPorts(inport_, outport1_, outport2_);
    addProperties(outport1ImageIndex_, outport2ImageIndex_, snapshot_, clear_);

    outport1_.setHandleResizeEvents(false);
    outport2_.setHandleResizeEvents(false);

    snapshot_.onChange([&]() {
        outport1ImageIndex_.setMaxValue(static_cast<int>(snapshots_.size()));
        outport2ImageIndex_.setMaxValue(static_cast<int>(snapshots_.size()));
        snapshots_.emplace_back(inport_.getData()->clone());
    });

    clear_.onChange([&]() {
        snapshots_.clear();
        outport1ImageIndex_.setMaxValue(-1);
        outport2ImageIndex_.setMaxValue(-1);
    });
}

void ImageSnapshot::process() {
    auto i1 = outport1ImageIndex_.get();
    auto i2 = outport2ImageIndex_.get();
    if (i1 == -1) {
        outport1_.setData(inport_.getData());
    } else {
        outport1_.setData(snapshots_[i1]);
    }

    if (i2 == -1) {
        outport2_.setData(inport_.getData());
    } else {
        outport2_.setData(snapshots_[i2]);
    }
}

void ImageSnapshot::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    clear_.pressButton();
}

}  // namespace inviwo
