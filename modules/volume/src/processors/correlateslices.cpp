/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/volume/processors/correlateslices.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CorrelateSlices::processorInfo_{
    "org.inviwo.CorrelateSlices",  // Class identifier
    "Correlate Slices",            // Display name
    "Undefined",                   // Category
    CodeState::Experimental,       // Code state
    Tags::None,                    // Tags
};
const ProcessorInfo CorrelateSlices::getProcessorInfo() const { return processorInfo_; }

CorrelateSlices::CorrelateSlices()
    : Processor()
    , imageOut_("correlationMatrix", DataFormat<vec4>::get(), false)
    , volumeIn_("volume")
    , range_("range", "Correlation Range", {0, 1}, {{0, 0}, ConstraintBehavior::Ignore},
             {{1, 1}, ConstraintBehavior::Ignore}, {1, 1}, InvalidationLevel::Valid,
             PropertySemantics::Text) {

    addPorts(volumeIn_, imageOut_);
    addProperty(range_);
    range_.setReadOnly(true);
}

void CorrelateSlices::process() {
    if (!volumeIn_.hasData()) {
        imageOut_.clear();
        return;
    }
    auto volume = volumeIn_.getData();

    size3_t volumeSize = volume->getDimensions();
    size_t imageSize = volumeSize.z;

    auto image = std::make_shared<Image>(size2_t{imageSize, imageSize}, DataFormat<vec4>::get());
    auto layer = image->getColorLayer();
    if (!layer) return;

    auto layerRam =
        dynamic_cast<LayerRAMPrecision<vec4>*>(layer->getEditableRepresentation<LayerRAM>());
    if (!layerRam) return;

    vec4* data = layerRam->getDataTyped();
    std::fill_n(data, imageSize * imageSize, vec4(0));
    if (!data) return;

    volume->getRepresentation<VolumeRAM>()->dispatch<void>([&](auto ram) -> void {
        using T = util::PrecisionValueType<decltype(ram)>;

        const T* ramData = ram->getDataTyped();
        float maxData = 0;

        for (size_t slice0 = 0; slice0 < volumeSize.z; ++slice0) {
            size_t flippedSlice0 = (imageSize - 1 - slice0);
            for (size_t slice1 = 0; slice1 < volumeSize.z; ++slice1) {
                if (slice0 >= slice1) continue;

                size_t flippedSlice1 = (imageSize - 1 - slice1);
                for (size_t y = 0; y < volumeSize.y; ++y) {
                    double rowDistance = 0;
                    size_t baseIdx0 = y * volumeSize.x + slice0 * volumeSize.x * volumeSize.y;
                    size_t baseIdx1 = y * volumeSize.x + slice1 * volumeSize.x * volumeSize.y;
                    for (size_t x = 0; x < volumeSize.x; ++x) {
                        rowDistance += double(glm::length2(util::glm_convert<dvec4>(
                            ramData[baseIdx0 + x] - ramData[baseIdx1 + x])));
                    }
                    data[flippedSlice0 * imageSize + slice1].x += rowDistance / volumeSize.x;
                }
                data[flippedSlice0 * imageSize + slice1].x /= volumeSize.y;
                data[flippedSlice1 * imageSize + slice0].x =
                    data[flippedSlice0 * imageSize + slice1].x;
                maxData = std::max<float>(maxData, data[flippedSlice0 * imageSize + slice1].x);
            }
        }

        range_.set(vec2{0, maxData});
        imageOut_.setData(image);
    });
}

}  // namespace inviwo
