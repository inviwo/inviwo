/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/discretedata/processors/sphericalcoordinates.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SphericalCoordinates::processorInfo_{
    "org.inviwo.SphericalCoordinates",  // Class identifier
    "Spherical Coordinates",            // Display name
    "DiscreteData",                     // Category
    CodeState::Experimental,            // Code state
    Tags::None,                         // Tags
};
const ProcessorInfo SphericalCoordinates::getProcessorInfo() const { return processorInfo_; }

SphericalCoordinates::SphericalCoordinates()
    : Processor()
    , dataIn_("DataIn")
    , dataOut_("DataOut")
    , positions_(dataIn_, "positions", "Lon/Lat Channel")
    , name_("outName", "Channel Name", "SphericalCoords")
    , autoName_("autoName", "Auto Name?", true)
    , radius_("radius", "Radius", 10, 0.1, 100)
    , verticalScale_("vertScale", "Vertical Scale", 0.01, 0.001, 10, 0.001) {

    addPort(dataIn_);
    addPort(dataOut_);
    addProperties(positions_, name_, radius_, verticalScale_);
}

void SphericalCoordinates::process() {
    if (dataIn_.isChanged() || positions_.isModified()) {
        auto channel = positions_.getCurrentChannel();
        if (!channel) return;

        if (autoName_.get()) {
            name_.set(fmt::format("Spherical{}", channel->getName()));
        }

        verticalScale_.setVisible(channel->getNumComponents() >= 3);

        // channel->dispatch<std::shared_ptr<Channel>>();
        detail::SphericalCoordinateDispatcher dispatcher;
        auto spherical = channeldispatching::dispatch<std::shared_ptr<const Channel>,
                                                      dispatching::filter::Scalars, 2,
                                                      DISCRETEDATA_MAX_NUM_DIMENSIONS>(
            channel->getDataFormatId(), channel->getNumComponents(), dispatcher, channel,
            name_.get(), radius_.get(), verticalScale_.get());
    }
}

}  // namespace discretedata
}  // namespace inviwo
