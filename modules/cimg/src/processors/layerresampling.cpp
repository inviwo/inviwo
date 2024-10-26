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

#include <modules/cimg/processors/layerresampling.h>

#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerResampling::processorInfo_{
    "org.inviwo.LayerResampling",  // Class identifier
    "Layer Resampling",            // Display name
    "Layer Operation",             // Category
    CodeState::Experimental,       // Code state
    Tags::CPU | Tag{"Layer"},      // Tags
    R"(Rescale a Layer to target dimensions using the chosen interpolation scheme. This only
       affects the resolution of the layer. The basis and offset will remain unchanged.)"_unindentHelp,
};

const ProcessorInfo& LayerResampling::getProcessorInfo() const { return processorInfo_; }

LayerResampling::LayerResampling()
    : Processor{}
    , inport_{"inport", "Input layer"_help}
    , outport_{"outport", "Resampled input layer"_help}
    , interpolationMode_{"interpolationMode",
                         "Interpolation",
                         "Interpolation scheme for up scaling and down scaling of the input layer"_help,
                         {{"nearest", "Nearest neighbor", cimgutil::InterpolationType::Nearest},
                          {"linear", "Linear", cimgutil::InterpolationType::Linear},
                          {"cubic", "Cubic", cimgutil::InterpolationType::Cubic},
                          {"lanczos", "Lanczos", cimgutil::InterpolationType::Lanczos},
                          {"movingAverage", "Moving average", cimgutil::InterpolationType::Moving},
                          {"grid", "Grid", cimgutil::InterpolationType::Grid}},
                         3}
    , sizing_{"sizing",
              "Sizing",
              "Sizing modes determining the dimensions of the resampled layer"_help,
              {{"sameAsInput", "Same as input", Sizing::SameAsInput},
               {"scalingFactor", "Scaling factor", Sizing::ScalingFactor},
               {"custom", "Custom size", Sizing::Custom}},
              1}
    , inputDimensions_{"inputDimensions", "Input Dimensions",
                       util::ordinalCount(size2_t{0}, size2_t{4096})
                           .set(PropertySemantics::Text)
                           .set("Size of the input layer (read-only)"_help)}
    , outputDimensions_{"outputDimensions", "Output Dimensions",
                        util::ordinalCount(size2_t{0}, size2_t{4096})
                            .set("Size of the resampled layer"_help)}
    , scaling_{"scaling", "Scaling",
               util::ordinalScale(1.0f, 4.0f)
                   .set("Scaling factor for Sizing Mode 'scaling factor'"_help)} {

    addPorts(inport_, outport_);
    addProperties(interpolationMode_, sizing_, inputDimensions_, outputDimensions_, scaling_);

    inputDimensions_.setReadOnly(true);

    outputDimensions_.readonlyDependsOn(
        sizing_, [](OptionProperty<Sizing>& p) { return p.get() != Sizing::Custom; });
    scaling_.readonlyDependsOn(
        sizing_, [](OptionProperty<Sizing>& p) { return p.get() != Sizing::ScalingFactor; });
    outputDimensions_.setReadOnly(sizing_.get() != Sizing::Custom);
    scaling_.setReadOnly(sizing_.get() != Sizing::ScalingFactor);
}

void LayerResampling::process() {
    const size2_t sourceDimensions{inport_.getData()->getDimensions()};
    inputDimensions_.set(sourceDimensions);

    switch (sizing_) {
        case Sizing::SameAsInput:
            outputDimensions_.set(sourceDimensions);
            break;
        case Sizing::Custom:
            break;
        case Sizing::ScalingFactor:
            outputDimensions_.set(ivec2{vec2{sourceDimensions} * scaling_.get()});
            break;
        default:
            outputDimensions_.set(sourceDimensions);
            break;
    }

    if (outputDimensions_.get() != sourceDimensions) {
        auto layer = std::make_shared<Layer>(*inport_.getData(), noData,
                                             LayerConfig{.dimensions = outputDimensions_.get()});

        const auto sourceLayerRam = inport_.getData()->getRepresentation<LayerRAM>();
        auto destLayerRam = layer->getEditableRepresentation<LayerRAM>();
        if (!cimgutil::rescaleLayerRamToLayerRam(sourceLayerRam, destLayerRam,
                                                 interpolationMode_.get(),
                                                 cimgutil::ConsiderAspectRatio::No)) {
            throw Exception(IVW_CONTEXT, "Rescaling layer failed.");
        }

        outport_.setData(layer);
    } else {
        outport_.setData(inport_.getData());
    }
}

}  // namespace inviwo
