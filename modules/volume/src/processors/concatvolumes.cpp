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

#include <inviwo/volume/processors/concatvolumes.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

size_t ConcatVolumes::VolumeSlabProperty::ID = 0;
const std::string ConcatVolumes::VolumeSlabProperty::classIdentifier_ = "ConcatSlabVolumeProperty";

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ConcatVolumes::processorInfo_{
    "org.inviwo.ConcatVolumes",  // Class identifier
    "Concat Volumes",            // Display name
    "Undefined",                 // Category
    CodeState::Experimental,     // Code state
    Tags::None,                  // Tags
};
const ProcessorInfo ConcatVolumes::getProcessorInfo() const { return processorInfo_; }

// VolumeOutport outVolume_;
// VolumeSlabProperty baseVolume_;
// IntSize2Property xyDimensions_;
// ListProperty appendVolumes_;
ConcatVolumes::ConcatVolumes()
    : Processor()
    , outVolume_("concatVolume")
    , xyDimensions_("xyDimensions", "X-Y Dimensions", {0, 0}, {{0, 0}, ConstraintBehavior::Ignore},
                    {{1, 1}, ConstraintBehavior::Ignore}, {1, 1}, InvalidationLevel::Valid,
                    PropertySemantics::Text)
    , baseVolume_("baseVolume", "Base Volume")
    , appendVolumes_("appendVolumes", "Volumes to Append") {

    baseVolume_.addParent(*this);
    auto prefab = std::make_unique<VolumeSlabProperty>("appendVolume", "Volume");
    prefab->parent_ = this;
    appendVolumes_.addPrefab(std::move(prefab));

    xyDimensions_.setReadOnly(true);

    addPort(outVolume_);
    addProperties(xyDimensions_, baseVolume_, appendVolumes_);

    // baseVolume_.volume_.onChange([&]() {

    // });
}

void ConcatVolumes::process() {
    size3_t size{0};
    if (baseVolume_.volume_.hasData()) {
        size = baseVolume_.volume_.getData()->getDimensions();
    }
    xyDimensions_.set({size.x, size.y});

    if (!baseVolume_.volume_.hasData()) {
        outVolume_.clear();
        baseVolume_.setChecked(false);
        return;
    }

    baseVolume_.setChecked(true);

    if (appendVolumes_.size() == 0) return;

    const auto baseVolumeData = baseVolume_.volume_.getData();
    DataFormatId baseFormat = baseVolumeData->getDataFormat()->getId();

    bool canProcess = true;
    size_t totalSizeZ = baseVolume_.zRange_.get();
    for (auto* prop : appendVolumes_.getPropertiesByType<VolumeSlabProperty>()) {
        bool valid = true;
        if (prop->volume_.hasData()) {
            const auto vol = prop->volume_.getData();
            DataFormatId appendFormat = vol->getDataFormat()->getId();
            size3_t dim = vol->getDimensions();
            totalSizeZ += prop->zRange_.get();
            if (dim.x != xyDimensions_.get().x || dim.y != xyDimensions_.get().y) valid = false;
            if (baseFormat != appendFormat) valid = false;
            prop->zRange_.setMinValue(1);
            prop->zRange_.setMaxValue(dim.z);
        } else
            valid = false;

        prop->setChecked(valid);
        if (!valid) {
            canProcess = false;
        }
    }
    if (!canProcess) {
        outVolume_.clear();
        return;
    }

    baseVolumeData->getRepresentation<VolumeRAM>()->dispatch<void>([&](auto ram) -> void {
        using T = util::PrecisionValueType<decltype(ram)>;

        size3_t volumeSize{xyDimensions_.get().x, xyDimensions_.get().y,
                           std::max(totalSizeZ, size_t{32})};
        size_t xyCells = volumeSize.x * volumeSize.y;
        const T* ramData = ram->getDataTyped();
        // if (totalSizeZ < 32) std::fill_n(ramData+xyCells*totalSizeZ, )

        auto ramVolumeOut = std::make_shared<VolumeRAMPrecision<T>>(volumeSize);
        T* const ramDataOut = ramVolumeOut->getDataTyped();
        T* currentHead = ramDataOut;

        size_t numElements = xyCells * baseVolume_.zRange_.get();
        memcpy(currentHead, ramData, numElements * sizeof(T));
        currentHead += numElements;

        auto range = baseVolumeData->dataMap_.dataRange;

        // Append other volumes.
        for (auto* prop : appendVolumes_.getPropertiesByType<VolumeSlabProperty>()) {
            const auto vol = prop->volume_.getData();
            ram = static_cast<const VolumeRAMPrecision<T>*>(vol->getRepresentation<VolumeRAM>());
            ramData = ram->getDataTyped();

            numElements = xyCells * prop->zRange_.get();
            memcpy(currentHead, ramData, numElements * sizeof(T));
            currentHead += numElements;
        }

        // Make a volume.
        auto volumeOut = std::make_shared<inviwo::Volume>(ramVolumeOut);
        volumeOut->dataMap_.dataRange = range;
        volumeOut->dataMap_.valueRange = range;

        volumeOut->setModelMatrix(baseVolumeData->getModelMatrix());
        outVolume_.setData(volumeOut);
    });
}

void ConcatVolumes::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    std::vector<Outport*> connections;
    for (auto* inport : getInports()) {
        connections.push_back(inport->getConnectedOutport());
        removePort(inport);
    }

    baseVolume_.addParent(*this);
    for (auto* prop : appendVolumes_.getPropertiesByType<VolumeSlabProperty>()) {
        prop->addParent(*this);
    }

    for (auto& prefab : appendVolumes_.getPrefabs()) {
        dynamic_cast<VolumeSlabProperty*>(prefab.get())->parent_ = this;
    }

    for (auto&& [inport, outport] : util::zip(getInports(), connections)) {
        inport->connectTo(outport);
    }
}

ConcatVolumes::VolumeSlabProperty::VolumeSlabProperty(const std::string& identifier,
                                                      const std::string& displayName)
    : BoolCompositeProperty(identifier, displayName)
    , zRange_("zRange", "zRange")
    , volume_(fmt::format("{}_{}", identifier, ID++)) {
    addProperty(zRange_);
}

ConcatVolumes::VolumeSlabProperty::~VolumeSlabProperty() {
    if (parent_) parent_->removePort(&volume_);
}

void ConcatVolumes::VolumeSlabProperty::addParent(ConcatVolumes& parent) {
    parent_ = &parent;
    parent_->addPort(volume_);
}

BoolCompositeProperty* ConcatVolumes::VolumeSlabProperty::clone() const {
    if (!parent_) return nullptr;
    auto copy = new VolumeSlabProperty(getIdentifier(), getDisplayName());
    copy->addParent(*parent_);
    return copy;
}

std::string ConcatVolumes::VolumeSlabProperty::getClassIdentifier() const {
    return classIdentifier_;
}

}  // namespace inviwo
