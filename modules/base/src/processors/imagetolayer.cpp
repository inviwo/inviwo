/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2026 Inviwo Foundation
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

#include <modules/base/processors/imagetolayer.h>

#include <inviwo/core/util/zip.h>

#include <fmt/format.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageToLayer::processorInfo_{
    "org.inviwo.ImageToLayer",                // Class identifier
    "Image To Layer",                         // Display name
    "Image Operation",                        // Category
    CodeState::Experimental,                  // Code state
    Tags::CPU | Tag{"Image"} | Tag{"Layer"},  // Tags
    R"(Extracts one layer from an image.)"_unindentHelp};

const ProcessorInfo& ImageToLayer::getProcessorInfo() const { return processorInfo_; }

ImageToLayer::ImageToLayer()
    : Processor{}
    , inport_{"inport", "Input image"_help, OutportDeterminesSize::Yes}
    , outport_{"outport", "Selected layer "_help}
    , outputLayer_{"outputLayer",
                   "Output Layer",
                   "Determines which layer of the input image is used as new color layer"_help,
                   {{"color0", "Color Layer 1", 0},
                    {"color1", "Color Layer 2", 1},
                    {"color2", "Color Layer 3", 2},
                    {"color3", "Color Layer 4", 3},
                    {"color4", "Color Layer 5", 4},
                    {"color5", "Color Layer 6", 5},
                    {"color6", "Color Layer 7", 6},
                    {"depth", "Depth Layer", static_cast<int>(LayerEnum::Depth)},
                    {"picking", "Picking Layer", static_cast<int>(LayerEnum::Picking)}}} {

    addPorts(inport_, outport_);
    addProperties(outputLayer_);
}

void ImageToLayer::process() {
    auto data = inport_.getData();
    auto layer = [&]() {
        switch (outputLayer_.get()) {
            case static_cast<int>(LayerEnum::Depth):
                return data->getDepthLayer()->clone();
            case static_cast<int>(LayerEnum::Picking):
                return data->getPickingLayer()->clone();
            default:
                const auto nColorLayers = inport_.getData()->getNumberOfColorLayers();
                if (outputLayer_.get() >= static_cast<int>(nColorLayers)) {
                    throw Exception(SourceContext{},
                                    "Selected color layer '{}' is out of bounds '{}'",
                                    outputLayer_.get(), nColorLayers);
                }
                return data->getColorLayer(outputLayer_.get())->clone();
        }
    }();

    outport_.setData(layer);
}

}  // namespace inviwo
