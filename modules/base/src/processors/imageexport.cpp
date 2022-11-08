/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
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

#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/datastructures/image/layer.h>      // for Layer
#include <inviwo/core/interaction/events/resizeevent.h>  // for ResizeEvent
#include <inviwo/core/io/datawriter.h>                   // for DataWriterType
#include <inviwo/core/io/datawriterexception.h>          // for DataWriterException
#include <inviwo/core/network/networkutils.h>            // for getSuccessors
#include <inviwo/core/network/portconnection.h>          // for PortConnection
#include <inviwo/core/network/processornetwork.h>        // for ProcessorNetwork
#include <inviwo/core/ports/imageport.h>                 // for ImageInport, BaseImageInport
#include <inviwo/core/ports/inport.h>                    // for Inport
#include <inviwo/core/ports/outportiterable.h>           // for OutportIterable
#include <inviwo/core/processors/processor.h>            // for Processor
#include <inviwo/core/processors/processorinfo.h>        // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>       // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>        // for Tags, Tags::CPU
#include <inviwo/core/properties/boolproperty.h>         // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>      // for IntSize2Property, OrdinalProperty
#include <inviwo/core/util/glmvec.h>                     // for size2_t, uvec3
#include <inviwo/core/util/stdextensions.h>              // for contains
#include <modules/base/processors/dataexport.h>          // for DataExport

#include <functional>     // for __base
#include <memory>         // for shared_ptr
#include <string>         // for string
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for operator!=
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

#include <fmt/core.h>    // for format_to, basic_string_view
#include <glm/vec2.hpp>  // for operator!=

namespace inviwo {

const ProcessorInfo ImageExport::processorInfo_{
    "org.inviwo.ImageExport",  // Class identifier
    "Image Export",            // Display name
    "Data Output",             // Category
    CodeState::Stable,         // Code state
    Tags::CPU,                 // Tags
    R"(A processor to save images to disk)"_unindentHelp};

ImageExport::ImageExport(InviwoApplication* app)
    : DataExport<Layer, ImageInport>{util::getDataWriterFactory(app), "", "image"}
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
