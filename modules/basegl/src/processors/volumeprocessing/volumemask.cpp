/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#include <modules/basegl/processors/volumeprocessing/volumemask.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/indexmapper.h>

#include <fstream>
#include <limits>

namespace inviwo {

const ProcessorInfo VolumeMask::processorInfo_{
    "org.inviwo.VolumeMask",  // Class identifier
    "Volume Mask",            // Display name
    "Mesh Rendering",         // Category
    CodeState::Experimental,  // Code state
    Tags::CPU,                // Tags
};
const ProcessorInfo VolumeMask::getProcessorInfo() const { return processorInfo_; }

VolumeMask::VolumeMask()
    : Processor()
    , volumeInport_("volume_inport")
    , volumeAnnotationInport_("volume_annotation_inport")
    , volumeOutport_("volume_outport")
    , enableMasking_("enableMasking", "Enable Masking", true)
    , idx_("idx", "idx", 0, 0, 10000000000, 1, InvalidationLevel::Valid)
    , addIdx_("addIdx", "Add Index")
    , removeIdx_("removeIdx", "Remove Index")
    , clearIdxList_("clearIdxList", "Clear Index List")
    , idxList_("idxList", "Index List")
    , idxTableFile_("idxTableFile", "Idx File") {

    addPort(volumeInport_);
    addPort(volumeAnnotationInport_);
    addPort(volumeOutport_);

    addProperty(enableMasking_);
    addProperty(idx_);

    addProperty(addIdx_);
    addIdx_.onChange([this]() {
        bool contains{false};
        for (const auto& idx : idxList_.getValues()) {
            if (idx == idx_.get()) {
                contains = true;
                break;
            }
        }

        if (!contains) {
            const auto str = std::to_string(idx_);
            idxList_.addOption(str, str, idx_);
        }
    });

    addProperty(removeIdx_);
    removeIdx_.onChange([this]() {
        idxList_.removeOption(idxList_.getSelectedIndex());
    });

    addProperty(clearIdxList_);
    clearIdxList_.onChange([this]() {
        idxList_.clearOptions();
        idxList_.propertyModified();
    });

    idxList_.setCurrentStateAsDefault();
    addProperty(idxList_);

    idxTableFile_.onChange([this]() {
        std::ifstream file_stream(idxTableFile_);
        if (file_stream.is_open()) {
            idxList_.clearOptions();
            size_t idx;
            while (file_stream >> idx) {
                const auto str = std::to_string(idx);
                idxList_.addOption(str, str, idx);
            }
        }
    });
    addProperty(idxTableFile_);
}

void VolumeMask::process() {
    if (enableMasking_.get()) {
        const auto volumeIn = volumeInport_.getData();
        auto volumeOut = volumeIn->clone();

        const auto vInRAM = volumeIn->getRepresentation<VolumeRAM>();
        auto vOutRAM = volumeOut->getEditableRepresentation<VolumeRAM>();
        const auto annoInRAM = volumeAnnotationInport_.getData()->getRepresentation<VolumeRAM>();

        annoInRAM->dispatch<void>([this, &vInRAM, &vOutRAM](auto annoVol) {
            using ValueType = util::PrecsionValueType<decltype(annoVol)>;
            using P = typename util::same_extent<ValueType, unsigned int>::type;

            // pre-convert, saves a lot of time
            std::vector<P> refIdxList;
            refIdxList.reserve(idxList_.getValues().size());
            for (const auto& idx : idxList_.getValues()) {
                refIdxList.push_back(P(idx));
            }

            const size3_t dims{vInRAM->getDimensions()};
            util::IndexMapper3D indexMapper(dims);
            const auto anno = annoVol->getDataTyped();

            size_t numMatchingVoxels{0};
#pragma omp parallel for
            for (long z = 0; z < dims.z; ++z) {
                for (size_t y = 0; y < dims.y; ++y) {
                    for (size_t x = 0; x < dims.x; ++x) {
                        P annoValue = anno[indexMapper(x, y, z)];

                        const size3_t pos{x, y, z};
                        bool contains{false};
                        for (const auto& refIdx : refIdxList) {
                            if (glm::all(annoValue == refIdx)) {
                                contains = true;
                                break;
                            }
                        }

                        if (contains) {
                            vOutRAM->setFromDVec4(pos, vInRAM->getAsDVec4(pos));
                            numMatchingVoxels++;
                        } else {
                            vOutRAM->setFromDouble(pos, 0.0);
                        }
                    }
                }
            }

            LogInfo(numMatchingVoxels << " voxels match index, avg value");
        });

        volumeOutport_.setData(volumeOut);
    } else {
        volumeOutport_.setData(volumeInport_.getData());
    }
}

}  // namespace inviwo
