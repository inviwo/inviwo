/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <modules/base/processors/layerinformation.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/zip.h>
#include <modules/base/algorithm/dataminmax.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerInformation::processorInfo_{
    "org.inviwo.LayerInformation",                  // Class identifier
    "Layer Information",                            // Display name
    "Information",                                  // Category
    CodeState::Stable,                              // Code state
    Tags::CPU | Tag{"Layer"} | Tag{"Information"},  // Tags
    R"(Shows available information provided by the input layer.)"_unindentHelp};

const ProcessorInfo& LayerInformation::getProcessorInfo() const { return processorInfo_; }

namespace {

constexpr auto transforms = util::generateTransforms(std::array{
    CoordinateSpace::Data, CoordinateSpace::Model, CoordinateSpace::World, CoordinateSpace::Index});

std::array<FloatMat4Property, 12> transformProps() {
    return util::make_array<12>([](auto index) {
        auto [from, to] = transforms[index];
        return FloatMat4Property(fmt::format("{}2{}", from, to), fmt::format("{} To {}", from, to),
                                 mat4(1.0f),
                                 util::filled<mat4>(std::numeric_limits<float>::lowest()),
                                 util::filled<mat4>(std::numeric_limits<float>::max()),
                                 util::filled<mat4>(0.001f), InvalidationLevel::Valid);
    });
}

std::array<DoubleMinMaxProperty, 4> minMaxProps() {
    return util::make_array<4>([](auto index) {
        return DoubleMinMaxProperty{fmt::format("minMaxChannel{}", index),
                                    fmt::format("Min/Max (Channel {})", index),
                                    0.0,
                                    255.0,
                                    -DataFloat64::max(),
                                    DataFloat64::max(),
                                    0.001,
                                    0.0,
                                    InvalidationLevel::Valid,
                                    PropertySemantics::Text};
    });
}

}  // namespace

LayerInformation::LayerInformation()
    : Processor{}
    , layer_{"layer", "Input layer"_help}
    , layerInfo_{"dataInformation", "Data Information"}
    , minMax_{minMaxProps()}
    , basis_{"basis",
             "Basis",
             mat3(1.0f),
             util::filled<mat3>(std::numeric_limits<float>::lowest()),
             util::filled<mat3>(std::numeric_limits<float>::max()),
             util::filled<mat3>(0.001f),
             InvalidationLevel::Valid}
    , offset_{"offset",
              "Offset",
              vec3(0.0f),
              vec3(std::numeric_limits<float>::lowest()),
              vec3(std::numeric_limits<float>::max()),
              vec3(0.001f),
              InvalidationLevel::Valid,
              PropertySemantics::Text}
    , texelSize_{"texelSize",
                 "Texel size",
                 dvec2(0),
                 dvec2(std::numeric_limits<float>::lowest()),
                 dvec2(std::numeric_limits<float>::max()),
                 dvec2(0.0001),
                 InvalidationLevel::Valid,
                 PropertySemantics::Text}
    , modelMatrix_{"modelMatrix",
                   "Model Matrix",
                   mat4(1.0f),
                   util::filled<mat4>(std::numeric_limits<float>::lowest()),
                   util::filled<mat4>(std::numeric_limits<float>::max()),
                   util::filled<mat4>(0.001f),
                   InvalidationLevel::Valid}
    , worldMatrix_{"worldTransform_",
                   "World Matrix",
                   mat4(1.0f),
                   util::filled<mat4>(std::numeric_limits<float>::lowest()),
                   util::filled<mat4>(std::numeric_limits<float>::max()),
                   util::filled<mat4>(0.001f),
                   InvalidationLevel::Valid}
    , indexMatrix_{"indexMatrix",
                   "Index Matrix",
                   mat4(1.0f),
                   util::filled<mat4>(std::numeric_limits<float>::lowest()),
                   util::filled<mat4>(std::numeric_limits<float>::max()),
                   util::filled<mat4>(0.001f),
                   InvalidationLevel::Valid}
    , spaceTransforms_{transformProps()}
    , perTexelProperties_{"minmaxValues", "Aggregated per Texel", false}
    , transformations_{"transformations", "Transformations"} {

    addPort(layer_);

    layerInfo_.setReadOnly(true);
    layerInfo_.setSerializationMode(PropertySerializationMode::None);
    addProperties(layerInfo_);

    addProperty(perTexelProperties_);
    perTexelProperties_.setCollapsed(true);
    util::for_each_argument(
        [&](auto& p) {
            p.setReadOnly(true);
            p.setSerializationMode(PropertySerializationMode::None);
            perTexelProperties_.addProperty(p);
        },
        minMax_[0], minMax_[1], minMax_[2], minMax_[3]);

    addProperty(transformations_);
    transformations_.setCollapsed(true);
    util::for_each_argument(
        [&](auto& p) {
            p.setReadOnly(true);
            p.setSerializationMode(PropertySerializationMode::None);
            transformations_.addProperty(p);
        },
        basis_, offset_, texelSize_, modelMatrix_, worldMatrix_, indexMatrix_);

    for (auto& transform : spaceTransforms_) {
        transform.setReadOnly(true);
        transformations_.addProperty(transform);
    }

    setAllPropertiesCurrentStateAsDefault();
}

void LayerInformation::process() {
    using enum util::OverwriteState;

    auto layer = layer_.getData();
    layerInfo_.updateForNewLayer(*layer, Yes);

    const auto dim = layer->getDimensions();

    const auto& trans = layer->getCoordinateTransformer();

    util::updateDefaultState(modelMatrix_, layer->getModelMatrix(), Yes);
    util::updateDefaultState(worldMatrix_, layer->getWorldMatrix(), Yes);
    util::updateDefaultState(indexMatrix_, layer->getIndexMatrix(), Yes);
    util::updateDefaultState(basis_, layer->getBasis(), Yes);
    util::updateDefaultState(offset_, layer->getOffset(), Yes);

    auto m = trans.getTextureToWorldMatrix();
    vec4 dx = m * vec4(1.0f / dim.x, 0, 0, 0);
    vec4 dy = m * vec4(0, 1.0f / dim.y, 0, 0);
    vec2 ts = {glm::length(dx), glm::length(dy)};
    texelSize_.set(ts);

    for (auto&& [index, transform] : util::enumerate(spaceTransforms_)) {
        auto [from, to] = transforms[index];
        transform.set(trans.getMatrix(from, to));
    }

    if (perTexelProperties_.isChecked()) {
        auto layerRAM = layer->getRepresentation<LayerRAM>();
        const auto channels = layer->getDataFormat()->getComponents();

        auto&& [min, max] = util::layerMinMax(layerRAM);
        for (size_t i = 0; i < 4; ++i) {
            minMax_[i].setVisible(channels >= i + 1);
            minMax_[i].set({min[i], max[i]});
        }
    }
}

}  // namespace inviwo
