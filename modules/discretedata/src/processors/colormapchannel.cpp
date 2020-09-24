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

#include <modules/discretedata/processors/colormapchannel.h>
#include <inviwo/core/util/formatdispatching.h>
#include <modules/discretedata/util/util.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ColormapChannel::processorInfo_{
    "org.inviwo.ColormapChannel",  // Class identifier
    "Colormap Channel",            // Display name
    "Undefined",                   // Category
    CodeState::Experimental,       // Code state
    Tags::None,                    // Tags
};
const ProcessorInfo ColormapChannel::getProcessorInfo() const { return processorInfo_; }

ColormapChannel::ColormapChannel()
    : Processor()
    , dataIn_("dataSetIn")
    , dataOut_("dataSetOut")
    , scalarChannel_(dataIn_, "scalarChannel", "Scalar Channel",
                     [](auto ch) { return ch->getNumComponents() == 1; })
    , colormap_("colormap", "Colormap")
    , channelName_("channelName", "Color Output Name", "Color")
    , automaticName_("autoName", "Auto Name Channel", true) {

    addPort(dataIn_);
    addPort(dataOut_);
    addProperties(scalarChannel_, colormap_, channelName_, automaticName_);
}

void ColormapChannel::process() {
    if (!dataIn_.hasData()) {
        dataOut_.detachData();
        return;
    }

    if (automaticName_.get() && scalarChannel_.getCurrentChannel() &&
        (automaticName_.isModified() || scalarChannel_.isModified())) {
        channelName_.set(scalarChannel_.getCurrentChannel()->getName() + "Color");
    }
    channelName_.setReadOnly(automaticName_.get());

    if (channelName_.isModified() || scalarChannel_.isModified() || colormap_.isModified()) {
        auto minMax = dd_util::getMinMax(scalarChannel_.getCurrentChannel().get());

        detail::ColorMappedChannelDispatcher dispatcher;
        auto colorChannel =
            dispatching::dispatch<std::shared_ptr<Channel>, dispatching::filter::Scalars>(
                scalarChannel_.getCurrentChannel()->getDataFormatId(), dispatcher,
                scalarChannel_.getCurrentChannel(), colormap_.get(), channelName_.get(),
                minMax.first.x, minMax.second.x);
        if (!colorChannel) {
            dataOut_.detachData();
            return;
        }

        auto dataSet = std::make_shared<DataSet>(*dataIn_.getData());
        dataSet->addChannel(colorChannel);
        dataOut_.setData(dataSet);
    }
}

}  // namespace discretedata
}  // namespace inviwo
