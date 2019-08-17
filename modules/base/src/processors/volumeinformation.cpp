/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/util/stringconversion.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/base/algorithm/volume/volumesignificantvoxels.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeInformation::processorInfo_{
    "org.inviwo.VolumeInformation",  // Class identifier
    "Volume Information",            // Display name
    "Information",                   // Category
    CodeState::Stable,               // Code state
    "CPU, Volume, Information",      // Tags
};
const ProcessorInfo VolumeInformation::getProcessorInfo() const { return processorInfo_; }

VolumeInformation::VolumeInformation()
    : Processor()
    , volume_("volume")
    , volumeInfo_("dataInformation", "Data Information")
    , significantVoxels_("significantVoxels", "Significant Voxels", 0, 0,
                         std::numeric_limits<size_t>::max(), 1, InvalidationLevel::Valid,
                         PropertySemantics::Text)
    , significantVoxelsRatio_("significantVoxelsRatio", "Significant Voxels (ratio)", 0.0f, 0.0f,
                              1.0f, 0.0001f, InvalidationLevel::Valid, PropertySemantics::Text)
    , minMaxChannel1_("minMaxChannel1_", "Min/Max (Channel 1)", 0.0, 255.0, -DataFloat64::max(),
                      DataFloat64::max(), 0.0, 0.0, InvalidationLevel::Valid,
                      PropertySemantics::Text)
    , minMaxChannel2_("minMaxChannel2_", "Min/Max (Channel 2)", 0.0, 255.0, -DataFloat64::max(),
                      DataFloat64::max(), 0.0, 0.0, InvalidationLevel::Valid,
                      PropertySemantics::Text)
    , minMaxChannel3_("minMaxChannel3_", "Min/Max (Channel 3)", 0.0, 255.0, -DataFloat64::max(),
                      DataFloat64::max(), 0.0, 0.0, InvalidationLevel::Valid,
                      PropertySemantics::Text)
    , minMaxChannel4_("minMaxChannel4_", "Min/Max (Channel 4)", 0.0, 255.0, -DataFloat64::max(),
                      DataFloat64::max(), 0.0, 0.0, InvalidationLevel::Valid,
                      PropertySemantics::Text)
    , worldTransform_("worldTransform_", "World Transform", mat4(1.0f),
                      util::filled<mat3>(std::numeric_limits<float>::lowest()),
                      util::filled<mat3>(std::numeric_limits<float>::max()),
                      util::filled<mat3>(0.001f), InvalidationLevel::Valid)
    , basis_("basis", "Basis", mat3(1.0f), util::filled<mat3>(std::numeric_limits<float>::lowest()),
             util::filled<mat3>(std::numeric_limits<float>::max()), util::filled<mat3>(0.001f),
             InvalidationLevel::Valid)
    , offset_("offset", "Offset", vec3(0.0f), vec3(std::numeric_limits<float>::lowest()),
              vec3(std::numeric_limits<float>::max()), vec3(0.001f), InvalidationLevel::Valid,
              PropertySemantics::Text)
    , perVoxelProperties_("minmaxValues", "Aggregated per Voxel", true)
    , transformations_("transformations", "Transformations")
    , metaDataProperty_("metaData", "Meta Data")
    , voxelSize_("voxelSize", "Voxel size", dvec3(0), dvec3(std::numeric_limits<float>::lowest()),
                 dvec3(std::numeric_limits<float>::max()), dvec3(0.0001), InvalidationLevel::Valid,
                 PropertySemantics::Text) {
    addPort(volume_);

    volumeInfo_.setReadOnly(true);
    volumeInfo_.setSerializationMode(PropertySerializationMode::None);
    addProperty(volumeInfo_);

    addProperty(perVoxelProperties_);
    perVoxelProperties_.setCollapsed(true);
    util::for_each_argument(
        [&](auto& p) {
            p.setReadOnly(true);
            perVoxelProperties_.addProperty(p);
        },
        significantVoxels_, significantVoxelsRatio_, minMaxChannel1_, minMaxChannel2_,
        minMaxChannel3_, minMaxChannel4_);

    addProperty(transformations_);
    transformations_.setCollapsed(true);
    util::for_each_argument(
        [&](auto& p) {
            p.setReadOnly(true);
            transformations_.addProperty(p);
        },
        worldTransform_, basis_, offset_, voxelSize_);

    addProperty(metaDataProperty_);

    setAllPropertiesCurrentStateAsDefault();
}

void VolumeInformation::process() {
    auto volume = volume_.getData();

    volumeInfo_.updateForNewVolume(*volume);

    auto volumeRAM = volume->getRepresentation<VolumeRAM>();

    const auto dim = volume->getDimensions();
    const auto c = volume->getDataFormat()->getComponents();
    const auto numVoxels = dim.x * dim.y * dim.z;

    worldTransform_.set(volume->getWorldMatrix());
    basis_.set(volume->getBasis());
    offset_.set(volume->getOffset());

    auto m = volume_.getData()->getCoordinateTransformer().getTextureToWorldMatrix();
    vec4 dx = m * vec4(1.0f / dim.x, 0, 0, 0);
    vec4 dy = m * vec4(0, 1.0f / dim.y, 0, 0);
    vec4 dz = m * vec4(0, 0, 1.0f / dim.z, 0);
    vec3 vs = {glm::length(dx), glm::length(dy), glm::length(dz)};

    voxelSize_.set(vs);

    if (perVoxelProperties_.isChecked()) {

        auto sigVoxels = util::volumeSignificantVoxels(volumeRAM, IgnoreSpecialValues::Yes);
        significantVoxels_.set(sigVoxels);
        significantVoxelsRatio_.set(static_cast<double>(sigVoxels) /
                                    static_cast<double>(numVoxels));

        auto minMax = util::volumeMinMax(volumeRAM);
        dvec2 minMaxA(minMax.first.x, minMax.second.x);
        dvec2 minMaxB(minMax.first.y, minMax.second.y);
        dvec2 minMaxC(minMax.first.z, minMax.second.z);
        dvec2 minMaxD(minMax.first.w, minMax.second.w);

        minMaxChannel1_.setVisible(c >= 1);
        minMaxChannel2_.setVisible(c >= 2);
        minMaxChannel3_.setVisible(c >= 3);
        minMaxChannel4_.setVisible(c >= 4);

        minMaxChannel1_.set(minMaxA);
        minMaxChannel2_.set(minMaxB);
        minMaxChannel3_.set(minMaxC);
        minMaxChannel4_.set(minMaxD);
    }

    metaDataProps_.updateProperty(metaDataProperty_, volume->getMetaDataMap());
}

}  // namespace inviwo
