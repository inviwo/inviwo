/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2026 Inviwo Foundation
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

#include <modules/base/processors/volumeinformation.h>

#include <inviwo/core/algorithm/markdown.h>
#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/properties/valuewrapper.h>
#include <inviwo/core/util/foreacharg.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/metadatatoproperty.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/zip.h>
#include <modules/base/algorithm/algorithmoptions.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/base/algorithm/volume/volumesignificantvoxels.h>
#include <modules/base/properties/volumeinformationproperty.h>

#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include <glm/geometric.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeInformation::processorInfo_{
    "org.inviwo.VolumeInformation",  // Class identifier
    "Volume Information",            // Display name
    "Information",                   // Category
    CodeState::Stable,               // Code state
    "CPU, Volume, Information",      // Tags
    R"(
    Shows available information provided by input volume including metadata.
    )"_unindentHelp};
const ProcessorInfo& VolumeInformation::getProcessorInfo() const { return processorInfo_; }

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

VolumeInformation::VolumeInformation()
    : Processor{}
    , volume_{"volume", "Input volume"_help}
    , volumeInfo_{"dataInformation", "Data Information"}
    , significantVoxels_{"significantVoxels",
                         "Significant Voxels",
                         0,
                         0,
                         std::numeric_limits<size_t>::max(),
                         1,
                         InvalidationLevel::Valid,
                         PropertySemantics::Text}
    , significantVoxelsRatio_{"significantVoxelsRatio",
                              "Significant Voxels (ratio)",
                              0.0f,
                              0.0f,
                              1.0f,
                              0.0001f,
                              InvalidationLevel::Valid,
                              PropertySemantics::Text}
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
    , voxelSize_{"voxelSize",
                 "Voxel size",
                 dvec3(0),
                 dvec3(std::numeric_limits<float>::lowest()),
                 dvec3(std::numeric_limits<float>::max()),
                 dvec3(0.0001),
                 InvalidationLevel::Valid,
                 PropertySemantics::Text}
    , modelMatrix_{"modelMatrix",
                   "Model Matrix",
                   mat4(1.0f),
                   util::filled<mat4>(std::numeric_limits<float>::lowest()),
                   util::filled<mat4>(std::numeric_limits<float>::max()),
                   util::filled<mat4>(0.001f),
                   InvalidationLevel::Valid}
    , worldMatrix_{"worldTransform",
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
    , perVoxelProperties_{"minmaxValues", "Aggregated per Voxel", false}
    , transformations_{"transformations", "Transformations"}
    , metaDataProperty_{
          "metaData", "Meta Data",
          "Composite property listing all the metadata stored in the input Image"_help} {

    addPort(volume_);

    volumeInfo_.setReadOnly(true);
    volumeInfo_.setSerializationMode(PropertySerializationMode::None);
    addProperty(volumeInfo_);

    addProperty(perVoxelProperties_);
    perVoxelProperties_.setCollapsed(true);
    util::for_each_argument(
        [&](auto& p) {
            p.setReadOnly(true);
            p.setSerializationMode(PropertySerializationMode::None);
            perVoxelProperties_.addProperty(p);
        },
        significantVoxels_, significantVoxelsRatio_, minMax_[0], minMax_[1], minMax_[2],
        minMax_[3]);

    addProperty(transformations_);
    transformations_.setCollapsed(true);
    util::for_each_argument(
        [&](auto& p) {
            p.setReadOnly(true);
            p.setSerializationMode(PropertySerializationMode::None);
            transformations_.addProperty(p);
        },
        basis_, offset_, voxelSize_, modelMatrix_, worldMatrix_, indexMatrix_);

    for (auto& transform : spaceTransforms_) {
        transform.setReadOnly(true);
        transformations_.addProperty(transform);
    }

    addProperty(metaDataProperty_);

    setAllPropertiesCurrentStateAsDefault();
}

void VolumeInformation::process() {
    using enum util::OverwriteState;

    auto volume = volume_.getData();

    volumeInfo_.updateForNewVolume(*volume, Yes);

    const auto dim = volume->getDimensions();
    const auto numVoxels = dim.x * dim.y * dim.z;

    const auto& trans = volume->getCoordinateTransformer();

    util::updateDefaultState(modelMatrix_, volume->getModelMatrix(), Yes);
    util::updateDefaultState(worldMatrix_, volume->getWorldMatrix(), Yes);
    util::updateDefaultState(indexMatrix_, volume->getIndexMatrix(), Yes);
    util::updateDefaultState(basis_, volume->getBasis(), Yes);
    util::updateDefaultState(offset_, volume->getOffset(), Yes);

    auto m = trans.getTextureToWorldMatrix();
    vec4 dx = m * vec4(1.0f / dim.x, 0, 0, 0);
    vec4 dy = m * vec4(0, 1.0f / dim.y, 0, 0);
    vec4 dz = m * vec4(0, 0, 1.0f / dim.z, 0);
    vec3 vs = {glm::length(dx), glm::length(dy), glm::length(dz)};
    voxelSize_.set(vs);

    for (auto&& [index, transform] : util::enumerate(spaceTransforms_)) {
        auto [from, to] = transforms[index];
        transform.set(trans.getMatrix(from, to));
    }

    if (perVoxelProperties_.isChecked()) {
        const auto* volumeRAM = volume->getRepresentation<VolumeRAM>();
        const auto channels = volume->getDataFormat()->getComponents();

        auto sigVoxels = util::volumeSignificantVoxels(volumeRAM, IgnoreSpecialValues::Yes);
        significantVoxels_.set(sigVoxels);
        significantVoxelsRatio_.set(static_cast<double>(sigVoxels) /
                                    static_cast<double>(numVoxels));

        auto&& [min, max] = util::volumeMinMax(volumeRAM);
        for (size_t i = 0; i < 4; ++i) {
            minMax_[i].setVisible(channels >= i + 1);
            minMax_[i].set({min[i], max[i]});
        }
    }

    metaDataProps_.updateProperty(metaDataProperty_, volume->getMetaDataMap());
}

}  // namespace inviwo
