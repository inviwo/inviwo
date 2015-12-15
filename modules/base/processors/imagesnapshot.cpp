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

#include "imagesnapshot.h"

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageSnapshot::processorInfo_{
    "org.inviwo.ImageSnapshot",  // Class identifier
    "Image Snapshot",            // Display name
    "Undefined",                 // Category
    CodeState::Experimental,     // Code state
    Tags::None,                  // Tags
};
const ProcessorInfo ImageSnapshot::getProcessorInfo() const { return processorInfo_; }

ImageSnapshot::ImageSnapshot()
    : Processor()
    , inport_("inport_")
    , outport1_("outport1")
    , outport2_("outport2")
    , outport1ImageIndex_("outport1ImageIndex", "Image 1 index", -1, -1, -1)
    , outport2ImageIndex_("outport2ImageIndex", "Image 2 index", -1, -1, -1)
    , snapshot_("snapshot", "Snapshot") 
    , clear_("clear","Clear")
{
    addPort(inport_);
    addPort(outport1_);
    addPort(outport2_);
    addProperty(outport1ImageIndex_);
    addProperty(outport2ImageIndex_);
    addProperty(snapshot_);
    addProperty(clear_);

    outport1_.setHandleResizeEvents(false);
    outport2_.setHandleResizeEvents(false);

    //outport1ImageIndex_.setSerializationMode(PropertySerializationMode::NONE);
    //outport2ImageIndex_.setSerializationMode(PropertySerializationMode::NONE);

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
    }
    else {
        outport1_.setData(snapshots_[i1]);
    }

    if (i2 == -1) {
        outport2_.setData(inport_.getData());
    }
    else {
        outport2_.setData(snapshots_[i2]);
    }
}



void ImageSnapshot::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    clear_.pressButton();
}




}  // namespace
