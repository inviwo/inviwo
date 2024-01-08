/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2024 Inviwo Foundation
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

#include <modules/base/processors/layercombiner.h>

#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/glmutils.h>

#include <algorithm>
#include <span>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerCombiner::processorInfo_{"org.inviwo.LayerCombiner",  // Class identifier
                                                  "Layer Combiner",            // Display name
                                                  "Image Operation",           // Category
                                                  CodeState::Experimental,     // Code state
                                                  Tags::CPU | Tag{"Layer"},    // Tags
                                                  R"(
Combines multiple layers into a single layer with multiple channels. All layers must 
share the same dimensions. The resulting data format depends on the common data type
and precision of the inputs.
)"_unindentHelp};

const ProcessorInfo LayerCombiner::getProcessorInfo() const { return processorInfo_; }

namespace {
const std::vector<OptionPropertyIntOption> channelsList = {{"channel1", "Channel 1", 0},
                                                           {"channel2", "Channel 2", 1},
                                                           {"channel3", "Channel 3", 2},
                                                           {"channel4", "Channel 4", 3}};
}

LayerCombiner::LayerCombiner()
    : Processor{}
    , source_{LayerInport{"source1", "Input for the first channel (red)"_help},
              LayerInport{"source2", "Input for the second channel (green, optional)"_help},
              LayerInport{"source3", "Input for the third channel (blue, optional)"_help},
              LayerInport{"source4", "Input for the fourth channel (alpha, optional)"_help}}
    , outport_{"outport", "Resulting Layer with combined channels"_help}

    , channel_{OptionPropertyInt{"dest1", "Channel 1 Out",
                                 "Selected channel of the first input"_help, channelsList},
               OptionPropertyInt{"dest2", "Channel 2 Out",
                                 "Selected channel of the second input"_help, channelsList},
               OptionPropertyInt{"dest3", "Channel 3 Out",
                                 "Selected channel of the third input"_help, channelsList},
               OptionPropertyInt{"dest4", "Channel 4 Out",
                                 "Selected channel of the fourth input"_help, channelsList}}
    , dataRange_{"dataRange", "Data Range", source_[0], true} {

    addPorts(source_[0], source_[1], source_[2], source_[3], outport_);
    for (auto& port : std::span(source_.begin() + 1, 3)) {
        port.setOptional(true);
    }

    for (auto& prop : channel_) {
        addProperty(prop);
    }
    addProperty(dataRange_);
}

void LayerCombiner::process() {
    std::vector<std::pair<LayerInport*, int>> activePorts;
    for (auto&& [port, channel] : util::zip(source_, channel_)) {
        if (port.hasData()) {
            activePorts.push_back({&port, channel.get()});
        }
    }

    const size2_t dims{activePorts.front().first->getData()->getDimensions()};
    if (std::ranges::any_of(
            activePorts, [dims](auto& p) { return p.first->getData()->getDimensions() != dims; })) {
        throw Exception("Image dimensions of all inports need to be identical", IVW_CONTEXT);
    }

    auto&& [type, precision] = [&]() {
        std::vector<const DataFormatBase*> formats;
        for (auto p : activePorts) {
            formats.push_back(p.first->getData()->getDataFormat());
        }
        return std::make_pair(util::commonNumericType(formats),
                              util::commonFormatPrecision(formats));
    }();

    auto layer = std::make_shared<Layer>(*activePorts.front().first->getData(), noData,
                                         DataFormatBase::get(type, activePorts.size(), precision));

    auto layerRam = layer->getEditableRepresentation<LayerRAM>();

#include <warn/push>
#include <warn/ignore/conversion>
#include <warn/ignore/conversion-loss>

    layerRam->dispatch<void>([&](auto layerram) {
        using PrecisionType = util::PrecisionValueType<decltype(layerram)>;
        using ValueType = util::value_type_t<PrecisionType>;

        const size2_t dims{layerram->getDimensions()};
        auto destData = layerram->getDataTyped();

        for (size_t inputChannel = 0; inputChannel < activePorts.size(); ++inputChannel) {
            auto&& [port, srcChannel] = activePorts[inputChannel];
            port->getData()->getRepresentation<LayerRAM>()->dispatch<void>(
                [&](auto srclayer, int srcChannel) {
                    const auto srcData = srclayer->getDataTyped();

                    for (size_t i = 0; i < glm::compMul(dims); ++i) {
                        util::glmcomp(destData[i], inputChannel) =
                            static_cast<ValueType>(util::glmcomp(srcData[i], srcChannel));
                    }
                },
                srcChannel);
        }
    });

#include <warn/pop>

    layer->setSwizzleMask(swizzlemasks::defaultData(activePorts.size()));
    layer->dataMap.dataRange = dataRange_.getDataRange();
    layer->dataMap.valueRange = dataRange_.getValueRange();

    outport_.setData(layer);
}

}  // namespace inviwo
