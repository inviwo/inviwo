/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/base/processors/imageexport.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

const ProcessorInfo ImageExport::processorInfo_{
    "org.inviwo.ImageExport",  // Class identifier
    "Image Export",            // Display name
    "Data Output",             // Category
    CodeState::Stable,         // Code state
    Tags::CPU,                 // Tags
};

ImageExport::ImageExport()
    : DataExport<Layer, ImageInport>{}
    , outportDeterminesSize_{"outportDeterminesSize", "Let Outport Determine Size", false}
    , imageSize_{"imageSize",   "Image Size",        size2_t(1024, 1024),
                 size2_t(1, 1), size2_t(4096, 4096), size2_t(1, 1)}
    , prevSize_{0} {

    addProperties(outportDeterminesSize_, imageSize_);
    imageSize_.visibilityDependsOn(outportDeterminesSize_,
                                   [](const auto& p) -> bool { return !p; });

    outportDeterminesSize_.onChange([this] {
        this->port_.setOutportDeterminesSize(outportDeterminesSize_);
        sendResizeEvent();
    });

    this->port_.setOutportDeterminesSize(outportDeterminesSize_);

    imageSize_.onChange([this]() { sendResizeEvent(); });
}
const ProcessorInfo ImageExport::getProcessorInfo() const { return processorInfo_; }

void ImageExport::setNetwork(ProcessorNetwork* network) {
    if (network) network->addObserver(this);

    Processor::setNetwork(network);
}

void ImageExport::sendResizeEvent() {
    const size2_t newSize = outportDeterminesSize_ ? size2_t{0} : *imageSize_;

    if (newSize != prevSize_) {
        ResizeEvent event{newSize, prevSize_};
        this->port_.propagateEvent(&event, nullptr);
        prevSize_ = newSize;
    }
}

const Layer* ImageExport::getData() {
    if (auto img = port_.getData()) {
        return img->getColorLayer();
    }
    return nullptr;
}

void ImageExport::onProcessorNetworkDidAddConnection(const PortConnection& con) {
    const auto successors = util::getSuccessors(con.getInport()->getProcessor());
    if (util::contains(successors, this)) {
        sendResizeEvent();
    }
}

void ImageExport::onProcessorNetworkDidRemoveConnection(const PortConnection& con) {
    const auto successors = util::getSuccessors(con.getInport()->getProcessor());
    if (util::contains(successors, this)) {
        sendResizeEvent();
    }
}

}  // namespace inviwo
