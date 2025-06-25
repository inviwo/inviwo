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

#include <modules/base/processors/volumehistogram2d.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/volumeramutils.h>
#include <inviwo/core/util/glmcomp.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/algorithm/histogram2d.h>
#include <inviwo/core/algorithm/histogram1d.h>

#include <inviwo/core/util/glmcomp.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/zip.h>

#include <ranges>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeHistogram2D::processorInfo_{
    "org.inviwo.VolumeHistogram2D",  // Class identifier
    "Volume Histogram 2D",           // Display name
    "Volume Operation",              // Category
    CodeState::Stable,               // Code state
    Tags::CPU | Tag{"Histogram"},    // Tags
    R"(Create a 2D Histogram from two volume channels. The resulting histogram is normalized with
       respect to the maximum bin count or the custom data range, if set.)"_unindentHelp,
};

const ProcessorInfo& VolumeHistogram2D::getProcessorInfo() const { return processorInfo_; }

namespace {

std::vector<OptionPropertyIntOption> enumeratedOptions() {
    std::vector<OptionPropertyIntOption> res;
    for (int volume = 1; volume < 3; ++volume) {
        for (int i = 0; i < 4; ++i) {
            res.emplace_back(fmt::format("volume{}Channel{}", volume, i + 1),
                             fmt::format("Volume {} Channel {}", volume, i + 1), i);
        }
    }
    return res;
}

std::shared_ptr<Layer> createLayer(const Histogram2D& hist, dvec2 normalizationRange,
                                   VolumeHistogram2D::Scaling scaling) {
    auto layerRep =
        std::make_shared<LayerRAMPrecision<float>>(LayerReprConfig{.dimensions = hist.dimensions});
    auto dst = layerRep->getView();

    const double delta = normalizationRange.y - normalizationRange.x;
    const double min = normalizationRange.x;

    using enum VolumeHistogram2D::Scaling;
    switch (scaling) {
        case Log: {
            const double logMax = std::log(1.0 + delta);
            std::ranges::transform(hist.counts, dst.begin(), [min, logMax](size_t v) {
                return static_cast<float>(std::log(1.0 + static_cast<double>(v) - min) / logMax);
            });
            break;
        }
        case Linear:
            std::ranges::transform(hist.counts, dst.begin(), [min, delta](size_t v) {
                return static_cast<float>((static_cast<double>(v) - min) / delta);
            });
            break;
        default:
            throw Exception{SourceContext{}, "Invalid scaling mode {}", static_cast<int>(scaling)};
            break;
    }

    auto layer = std::make_shared<Layer>(LayerConfig{
        .dimensions = hist.dimensions,
        .interpolation = InterpolationType::Nearest,
        .dataRange = dvec2{0.0, 1.0},
        .valueRange = dvec2{0.0, 1.0},
    });
    layer->addRepresentation(layerRep);
    return layer;
}

}  // namespace

VolumeHistogram2D::VolumeHistogram2D()
    : Processor{}
    , inport1_{"inport1", "Volume 1"_help}
    , inport2_{"inport2", "Volume 2"_help}
    , outport_{"outport", "Histogram Layer"_help}
    , histogramResolution_{"histogramResolution",
                           "Histogram Resolution. The actual resolution might deviate based on the "
                           "underlying data types in order to avoid binning artifacts.",
                           util::ordinalCount(1024, 16384)
                               .setMin(1)
                               .set("Resolution of the resulting 2D histogram."_help)}
    , channel1_{"channel1", "Channel 1", enumeratedOptions(), 0}
    , channel2_{"channel2", "Channel 2", enumeratedOptions(), 1}
    , scaling_{"scaling", "Scaling",
               OptionPropertyState<Scaling>{.options = {{"linear", "Linear", Scaling::Linear},
                                                        {"log", "Logarithmic", Scaling::Log}}}
                   .setSelectedValue(Scaling::Linear)
                   .set(R"(The scaling is applied to the resulting histogram and used for
normalization. With `v` being the bin value, the normalized bin value `v'` is computed as
follows:

    + `Linear`       uses `v' = v / max`
    + `Logarithmic`  uses `v' = log(1 + v) / log(1 + max)`

where `max` refers to the maximum bin count or the upper limit of the custom data range, if set.
        )"_unindentHelp)}
    , dataRange_{"dataRange", "Data Range"} {

    inport2_.setOptional(true);
    addPorts(inport1_, inport2_, outport_);
    addProperties(histogramResolution_, channel1_, channel2_, scaling_, dataRange_);
}

void VolumeHistogram2D::process() {
    const auto histSize = size2_t{static_cast<size_t>(std::max(histogramResolution_.get(), 1))};

    if (channel1_.getSelectedIndex() >= 4 && !inport2_.hasData()) {
        throw Exception(SourceContext{},
                        "Channel 1 refers to the second Volume inport, which is not "
                        "connected/has no data.");
    }
    if (channel2_.getSelectedIndex() >= 4 && !inport2_.hasData()) {
        throw Exception(SourceContext{},
                        "Channel 2 refers to the second Volume inport, which is not "
                        "connected/has no data.");
    }

    auto accessVolume =
        [&](const OptionPropertyInt& p) -> std::tuple<std::shared_ptr<const Volume>, size_t> {
        if (p.getSelectedIndex() >= 4) {
            return {inport2_.getData(), p.get()};
        } else {
            return {inport1_.getData(), p.get()};
        }
    };

    auto&& [volume1, c1] = accessVolume(channel1_);
    auto&& [volume2, c2] = accessVolume(channel2_);

    if (volume1->getDimensions() != volume2->getDimensions()) {
        throw Exception(SourceContext{}, "Volume dimensions must match, got {} and {}",
                        volume1->getDimensions(), volume2->getDimensions());
    }

    if (c1 >= volume1->getDataFormat()->getComponents()) {
        throw Exception(SourceContext{},
                        "Channel 1 is greater than the available channels {} >= {}", c1 + 1,
                        volume1->getDataFormat()->getComponents());
    }
    if (c2 >= volume2->getDataFormat()->getComponents()) {
        throw Exception(SourceContext{},
                        "Channel 2 is greater than the available channels {} >= {}", c2 + 1,
                        volume2->getDataFormat()->getComponents());
    }

    const auto* vrep1 = volume1->getRepresentation<VolumeRAM>();
    const auto* vrep2 = volume2->getRepresentation<VolumeRAM>();

    auto [numbins1, effectiveRange1] =
        dispatching::singleDispatch<std::pair<size_t, double>, dispatching::filter::All>(
            vrep1->getDataFormatId(), [volume1, histSize]<typename T>() {
                return util::detail::optimalBinCount<T>(volume1->dataMap, histSize.x);
            });
    auto [numbins2, effectiveRange2] =
        dispatching::singleDispatch<std::pair<size_t, double>, dispatching::filter::All>(
            vrep2->getDataFormatId(), [volume2, histSize]<typename U>() {
                return util::detail::optimalBinCount<U>(volume2->dataMap, histSize.y);
            });

    const dvec2 effectiveRange{effectiveRange1, effectiveRange2};
    const size2_t numbins{numbins1, numbins2};

    const auto hist = dispatching::doubleDispatch<Histogram2D, dispatching::filter::All,
                                                  dispatching::filter::All>(
        vrep1->getDataFormatId(), vrep2->getDataFormatId(), [&]<typename T1, typename T2>() {
            auto src1 = static_cast<const VolumeRAMPrecision<T1>*>(vrep1)->getView();
            auto src2 = static_cast<const VolumeRAMPrecision<T2>*>(vrep2)->getView();

            return util::calculateHistogram2D<T1, T2>(src1, c1, volume1->dataMap, src2, c2,
                                                      volume2->dataMap, numbins);
        });

    const dvec2 range{0.0, hist.maxCount};
    dataRange_.setDataRange(range);
    dataRange_.setValueRange(dataRange_.getDataRange());

    auto layer = createLayer(hist, dataRange_.getDataRange(), scaling_);
    layer->axes[0] = volume1->dataMap.valueAxis;
    layer->axes[1] = volume2->dataMap.valueAxis;

    outport_.setData(layer);
}

}  // namespace inviwo
