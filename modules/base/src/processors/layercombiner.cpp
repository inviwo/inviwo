/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

const std::vector<OptionPropertyIntOption> channelsList = {{"channel1", "Channel 1", 0},
                                                           {"channel2", "Channel 2", 1},
                                                           {"channel3", "Channel 3", 2},
                                                           {"channel4", "Channel 4", 3}};

LayerCombiner::LayerCombiner()
    : Processor{}
    , source1_{"source1", "Input for the first channel (red)"_help}
    , source2_{"source2", "Input for the second channel (green, optional)"_help}
    , source3_{"source3", "Input for the third channel (blue, optional)"_help}
    , source4_{"source4", "Input for the fourth channel (alpha, optional)"_help}
    , outport_{"outport", "Resulting Layer with combined channels"_help}

    , channel1Source_{"dest1", "Channel 1 Out", "Selected channel of the first input"_help,
                      channelsList}
    , channel2Source_{"dest2", "Channel 2 Out", "Selected channel of the second input"_help,
                      channelsList}
    , channel3Source_{"dest3", "Channel 3 Out", "Selected channel of the third input"_help,
                      channelsList}
    , channel4Source_{"dest4", "Channel 4 Out", "Selected channel of the fourth input"_help,
                      channelsList}
    , dataRange_{"dataRange", "Data Range", source1_, true} {

    addPorts(source1_, source2_, source3_, source4_, outport_);
    source2_.setOptional(true);
    source3_.setOptional(true);
    source4_.setOptional(true);

    addProperties(channel1Source_, channel2Source_, channel3Source_, channel4Source_, dataRange_);
}

void LayerCombiner::process() {
    // auto activePorts =
    //     util::copy_if(std::array<LayerInport*, 4>{&source1_, &source2_, &source3_, &source4_},
    //                   [](LayerInport* p) { return p->hasData(); });

    std::vector<std::pair<LayerInport*, int>> activePorts;
    for (auto&& [port, channel] :
         util::zip(std::array<LayerInport*, 4>{&source1_, &source2_, &source3_, &source4_},
                   std::array<int, 4>{channel1Source_.get(), channel2Source_.get(),
                                      channel3Source_.get(), channel4Source_.get()})) {
        if (port->hasData()) {
            activePorts.push_back({port, channel});
        }
    }

    const size2_t dims{activePorts.front().first->getData()->getDimensions()};
    for (auto p : activePorts) {
        if (p.first->getData()->getDimensions() != dims) {
            throw Exception("Image dimensions of all inports need to be identical", IVW_CONTEXT);
        }
    }

    auto&& [type, precision] = [&]() {
        std::vector<const DataFormatBase*> formats;
        for (auto p : activePorts) {
            formats.push_back(p.first->getData()->getDataFormat());
        }

        NumericType type = formats.front()->getNumericType();
        for (auto f : formats) {
            if (type == NumericType::Float || f->getNumericType() == NumericType::Float) {
                type = NumericType::Float;
            } else if (type == NumericType::SignedInteger ||
                       f->getNumericType() == NumericType::SignedInteger) {
                type = NumericType::SignedInteger;
            } else if (f->getNumericType() == NumericType::UnsignedInteger) {
                type = NumericType::UnsignedInteger;
            }
        }
        size_t precision = (*std::ranges::max_element(formats, [](auto format1, auto format2) {
                               return format1->getPrecision() < format2->getPrecision();
                           }))->getPrecision();
        return std::make_pair(type, precision);
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

    auto swizzleMask = [](size_t numComponents) {
        switch (numComponents) {
            case 1:
                return swizzlemasks::luminance;
            case 2:
                return SwizzleMask{ImageChannel::Red, ImageChannel::Green, ImageChannel::Zero,
                                   ImageChannel::One};
            case 3:
                return swizzlemasks::rgb;
            case 4:
            default:
                return swizzlemasks::rgba;
        }
    };
    layer->setSwizzleMask(swizzleMask(activePorts.size()));
    layer->dataMap.dataRange = dataRange_.getDataRange();
    layer->dataMap.valueRange = dataRange_.getValueRange();

    outport_.setData(layer);
}

}  // namespace inviwo
