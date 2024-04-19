/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/base/processors/histogram2d.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/volumeramutils.h>
#include <inviwo/core/util/glmcomp.h>
#include <inviwo/core/util/exception.h>

#include <ranges>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Histogram2D::processorInfo_{
    "org.inviwo.Histogram2D",      // Class identifier
    "Histogram 2D",                // Display name
    "Volume Operation",            // Category
    CodeState::Experimental,       // Code state
    Tags::CPU | Tag{"Histogram"},  // Tags
    R"(Create a 2D Histogram from two Volumes)"_unindentHelp,
};

const ProcessorInfo& Histogram2D::getProcessorInfo() const { return processorInfo_; }

Histogram2D::Histogram2D()
    : Processor{}
    , inport1_{"inport1", "Volume 1"_help}
    , inport2_{"inport2", "Volume 2"_help}
    , outport_{"outport", "Histogram Layer"_help}
    , histogramResolution_{"histogramResolution", "Histogram Resolution",
                           util::ordinalCount(1024, 16384)
                               .setMin(1)
                               .set("Resolution of the resulting 2D histogram."_help)}
    , channel1_{"channel1", "Volume 1 Channel", util::enumeratedOptions("Channel", 4), 0}
    , channel2_{"channel2", "Volume 2 Channel", util::enumeratedOptions("Channel", 4), 0}
    , scaling_{"scaling", "Scaling",
               OptionPropertyState<Scaling>{.options = {{"linear", "Linear", Scaling::Linear},
                                                        {"log", "Logarithmic", Scaling::Log}}}
                   .setSelectedValue(Scaling::Linear)}
    , dataRange_{"dataRange", "Data Range"} {

    addPorts(inport1_, inport2_, outport_);
    addProperties(histogramResolution_, channel1_, channel2_, scaling_, dataRange_);
}

void Histogram2D::process() {
    const auto histSize = size2_t{static_cast<size_t>(std::max(histogramResolution_.get(), 1))};

    auto layerRep =
        std::make_shared<LayerRAMPrecision<float>>(LayerReprConfig{.dimensions = histSize});

    auto dst = layerRep->getView();

    const util::IndexMapper2D layerIm{layerRep->getDimensions()};

    auto volume1 = inport1_.getData();
    auto volume2 = inport2_.getData();

    const auto c1 = channel1_.getSelectedIndex();
    const auto c2 = channel2_.getSelectedIndex();

    if (volume1->getDimensions() != volume2->getDimensions()) {
        throw Exception(IVW_CONTEXT, "Volume dimensions must match, got {} and {}",
                        volume1->getDimensions(), volume2->getDimensions());
    }

    if (c1 >= volume1->getDataFormat()->getComponents()) {
        throw Exception(IVW_CONTEXT, "Channel 1 is greater than the available channels {} >= {}",
                        c1 + 1, volume1->getDataFormat()->getComponents());
    }
    if (c2 >= volume2->getDataFormat()->getComponents()) {
        throw Exception(IVW_CONTEXT, "Channel 2 is greater than the available channels {} >= {}",
                        c2 + 1, volume2->getDataFormat()->getComponents());
    }

    auto vrep1 = volume1->getRepresentation<VolumeRAM>();
    auto vrep2 = volume2->getRepresentation<VolumeRAM>();

    auto& map1 = volume1->dataMap;
    auto& map2 = volume2->dataMap;

    const util::IndexMapper3D volumeIm{vrep1->getDimensions()};

    size_t overflow = 0;

    dispatching::doubleDispatch<void, dispatching::filter::All, dispatching::filter::All>(
        vrep1->getDataFormatId(), vrep2->getDataFormatId(), [&]<typename T1, typename T2>() {
            auto src1 = static_cast<const VolumeRAMPrecision<T1>*>(vrep1)->getView();
            auto src2 = static_cast<const VolumeRAMPrecision<T2>*>(vrep2)->getView();

            util::forEachVoxel(vrep1->getDimensions(), [&](const size3_t& pos) {
                const auto voxel1 = static_cast<double>(util::glmcomp(src1[volumeIm(pos)], c1));
                const auto voxel2 = static_cast<double>(util::glmcomp(src2[volumeIm(pos)], c2));

                const auto voxel1Normalized = map1.mapFromDataToNormalized(voxel1);
                const auto voxel2Normalized = map2.mapFromDataToNormalized(voxel2);

                if ((voxel1Normalized < 0.0 || voxel1Normalized > 1.0) ||
                    (voxel2Normalized < 0.0 || voxel2Normalized > 1.0)) {
                    ++overflow;
                } else {
                    const auto i1 =
                        static_cast<size_t>(voxel1Normalized * static_cast<double>(histSize[0]));

                    const auto i2 =
                        static_cast<size_t>(voxel2Normalized * static_cast<double>(histSize[1]));

                    const auto index = glm::min(size2_t{i1, i2}, histSize - size2_t{1});

                    ++dst[layerIm(index)];
                }
            });
        });

    auto max = *std::ranges::max_element(dst);
    dvec2 range{0.0, max};
    dataRange_.setDataRange(range);

    max = static_cast<float>(dataRange_.getDataRange().y);
    dataRange_.setValueRange(dvec2{0.0, max});

    switch (scaling_) {
        case Scaling::Log: {
            const float logMax = std::log(1.0f + max);
            std::ranges::transform(dst, dst.begin(),
                                   [logMax](float v) { return std::log(1.0f + v) / logMax; });
            break;
        }
        case Scaling::Linear:
        default:
            std::ranges::transform(dst, dst.begin(), [max](float v) { return v / max; });
            break;
    }

    auto layer = std::make_shared<Layer>(LayerConfig{
        .dimensions = histSize,
        .interpolation = InterpolationType::Nearest,
        .xAxis = volume1->dataMap.valueAxis,
        .yAxis = volume2->dataMap.valueAxis,
        .dataRange = dvec2{0.0, 1.0},
        .valueRange = dvec2{0.0, 1.0},
    });
    layer->addRepresentation(layerRep);
    outport_.setData(layer);
}

}  // namespace inviwo
