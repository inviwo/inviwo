/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/metadata/metadata.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/base/algorithm/volume/volumesignificantvoxels.h>

namespace inviwo {

template <typename T, int N, int M>
static void addMetaDataToCompositeProperty(CompositeProperty& prop,
                                           const MetaDataPrimitiveType<T, N, M>* metadata,
                                           const std::string& metaDataId) {
    LogWarnCustom("VolumeInformation::addMetaDataToCompositeProperty",
                  "Meta data type not supported: " << metadata->getClassIdentifier());
    LogWarnCustom("VolumeInformation::addMetaDataToCompositeProperty",
                  "\tMetaData id: " << metaDataId);
}

std::string toString(bool value) {
    std::ostringstream stream;
    stream << std::boolalpha << value;
    return stream.str();
}

template <typename T>
static void addMetaDataToCompositeProperty(CompositeProperty& prop,
                                           const MetaDataPrimitiveType<T, 0, 0>* metadata,
                                           const std::string& metaDataId) {
    auto classID = metadata->getClassIdentifier();
    auto propertyClassID = classID;
    replaceInString(propertyClassID, "MetaData", "Property");

    auto theProperty = prop.getPropertyByIdentifier(metaDataId);
    if (!theProperty) {
        auto ptr =
            util::make_unique<StringProperty>(metaDataId, metaDataId, "", InvalidationLevel::Valid);
        ptr->setSerializationMode(PropertySerializationMode::All);
        ptr->setCurrentStateAsDefault();
        theProperty = ptr.release();
        prop.addProperty(theProperty, true);
    }

    if (auto typed = dynamic_cast<StringProperty*>(theProperty)) {
        typed->set(toString(metadata->get()));
    }

    theProperty->setVisible(true);
    theProperty->setReadOnly(true);
}

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeInformation::processorInfo_{
    "org.inviwo.VolumeInformation",  // Class identifier
    "Volume Information",            // Display name
    "Information",                   // Category
    CodeState::Stable,               // Code state
    "CPU, Volume",                   // Tags
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
                      PropertySemantics("Text"))
    , minMaxChannel2_("minMaxChannel2_", "Min/Max (Channel 2)", 0.0, 255.0, -DataFloat64::max(),
                      DataFloat64::max(), 0.0, 0.0, InvalidationLevel::Valid,
                      PropertySemantics("Text"))
    , minMaxChannel3_("minMaxChannel3_", "Min/Max (Channel 3)", 0.0, 255.0, -DataFloat64::max(),
                      DataFloat64::max(), 0.0, 0.0, InvalidationLevel::Valid,
                      PropertySemantics("Text"))
    , minMaxChannel4_("minMaxChannel4_", "Min/Max (Channel 4)", 0.0, 255.0, -DataFloat64::max(),
                      DataFloat64::max(), 0.0, 0.0, InvalidationLevel::Valid,
                      PropertySemantics("Text"))
    , worldTransform_("worldTransform_", "World Transform", mat4(1.0f),
                      mat4(std::numeric_limits<float>::lowest()),
                      mat4(std::numeric_limits<float>::max()), mat4(0.001f),
                      InvalidationLevel::Valid)
    , basis_("basis", "Basis", mat3(1.0f), mat3(std::numeric_limits<float>::lowest()),
             mat3(std::numeric_limits<float>::max()), mat3(0.001f), InvalidationLevel::Valid)
    , offset_("offset", "Offset", vec3(0.0f), vec3(std::numeric_limits<float>::lowest()),
              vec3(std::numeric_limits<float>::max()), vec3(0.001f), InvalidationLevel::Valid,
              PropertySemantics("Text"))
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
    perVoxelProperties_.addProperty(significantVoxels_);
    perVoxelProperties_.addProperty(significantVoxelsRatio_);

    perVoxelProperties_.addProperty(minMaxChannel1_);
    perVoxelProperties_.addProperty(minMaxChannel2_);
    perVoxelProperties_.addProperty(minMaxChannel3_);
    perVoxelProperties_.addProperty(minMaxChannel4_);

    perVoxelProperties_.onChange([this]() {
        bool state = perVoxelProperties_.isChecked();
        minMaxChannel1_.setReadOnly(!state);
        minMaxChannel2_.setReadOnly(!state);
        minMaxChannel3_.setReadOnly(!state);
        minMaxChannel4_.setReadOnly(!state);
    });

    addProperty(transformations_);
    transformations_.setCollapsed(true);
    transformations_.addProperty(worldTransform_);
    transformations_.addProperty(basis_);
    transformations_.addProperty(offset_);
    transformations_.addProperty(voxelSize_);

    addProperty(metaDataProperty_);

    offset_.setSemantics(PropertySemantics("Text"));

    minMaxChannel2_.setVisible(false);
    minMaxChannel3_.setVisible(false);
    minMaxChannel4_.setVisible(false);

    for (auto p : getProperties()) {
        p->setInvalidationLevel(InvalidationLevel::Valid);
    }

    setAllPropertiesCurrentStateAsDefault();
}

void VolumeInformation::process() {
    auto volume = volume_.getData();

    volumeInfo_.updateForNewVolume(*(volume.get()));

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

    auto metaMap = volume->getMetaDataMap();
    if (metaMap->empty()) {
        while (metaDataProperty_.getProperties().size() != 0) {
            metaDataProperty_.removeProperty(metaDataProperty_.getProperties()[0]);
        }
    } else {
        for (auto p : metaDataProperty_.getProperties()) {
            p->setVisible(false);  // Hide all, the addMetaDataToCompositeProperty will show it if
                                   // it still exists
        }

        auto keys = metaMap->getKeys();
        for (auto key : keys) {
            auto meta = metaMap->get(key);
            auto classIdentifier = meta->getClassIdentifier();
            if (auto typed1 = dynamic_cast<const BoolMetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed1, key);
            } else if (auto typed2 = dynamic_cast<const IntMetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed2, key);
            } else if (auto typed3 = dynamic_cast<const FloatMetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed3, key);
            } else if (auto typed4 = dynamic_cast<const DoubleMetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed4, key);
            } else if (auto typed5 = dynamic_cast<const StringMetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed5, key);
            } else if (auto typed6 = dynamic_cast<const FloatVec2MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed6, key);
            } else if (auto typed7 = dynamic_cast<const FloatVec3MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed7, key);
            } else if (auto typed8 = dynamic_cast<const FloatVec4MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed8, key);
            } else if (auto typed9 = dynamic_cast<const DoubleVec2MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed9, key);
            } else if (auto typed10 = dynamic_cast<const DoubleVec3MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed10, key);
            } else if (auto typed11 = dynamic_cast<const DoubleVec4MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed11, key);
            } else if (auto typed12 = dynamic_cast<const IntVec2MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed12, key);
            } else if (auto typed13 = dynamic_cast<const IntVec3MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed13, key);
            } else if (auto typed14 = dynamic_cast<const IntVec4MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed14, key);
            } else if (auto typed15 = dynamic_cast<const UIntVec2MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed15, key);
            } else if (auto typed16 = dynamic_cast<const UIntVec3MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed16, key);
            } else if (auto typed17 = dynamic_cast<const UIntVec4MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed17, key);
            } else if (auto typed18 = dynamic_cast<const FloatMat2MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed18, key);
            } else if (auto typed19 = dynamic_cast<const FloatMat3MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed19, key);
            } else if (auto typed20 = dynamic_cast<const FloatMat4MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed20, key);
            } else if (auto typed21 = dynamic_cast<const DoubleMat2MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed21, key);
            } else if (auto typed22 = dynamic_cast<const DoubleMat3MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed22, key);
            } else if (auto typed23 = dynamic_cast<const DoubleMat4MetaData*>(meta)) {
                addMetaDataToCompositeProperty(metaDataProperty_, typed23, key);
            } else {
                LogError("Unsupported MetaData type");
            }
        }
    }
}

}  // namespace inviwo
