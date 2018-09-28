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

#include <modules/base/processors/volumesource.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/rawvolumereader.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/resourcemanager/resource.h>
#include <inviwo/core/resourcemanager/resourcemanager.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/metadata/metadata.h>

#include <cmath>

namespace inviwo {

const ProcessorInfo VolumeSource::processorInfo_{
    "org.inviwo.VolumeSource",  // Class identifier
    "Volume Source",            // Display name
    "Data Input",               // Category
    CodeState::Stable,          // Code state
    Tags::CPU,                  // Tags
};
const ProcessorInfo VolumeSource::getProcessorInfo() const { return processorInfo_; }

VolumeSource::VolumeSource(InviwoApplication* app, const std::string& file)
    : Processor()
    , app_(app)
    , outport_("data")
    , file_("filename", "File", file, "volume")
    , reload_("reload", "Reload data")
    , volumeIndex_("volumeindex", "Volume Index", 0, 0, 0, 1)
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information")
    , volumeSequence_("Sequence", "Sequence") {

    // make sure that we always process even if not connected
    isSink_.setUpdate([]() { return true; });

    file_.setContentType("volume");
    file_.setDisplayName("Volume file");

    volumeSequence_.setVisible(false);

    addFileNameFilters();

    addPort(outport_);

    addProperty(file_);
    addProperty(reload_);
    addProperty(volumeIndex_);
    addProperty(information_);
    addProperty(basis_);
    addProperty(volumeSequence_);
}

void VolumeSource::load(bool deserialize) {
    if (file_.get().empty()) return;

    auto rf = app_->getDataReaderFactory();
    auto rm = app_->getResourceManager();
    std::string ext = filesystem::getFileExtension(file_.get());

    // use resource unless the "Reload data"-button (reload_) was pressed,
    // Note: reload_ will be marked as modified when deserializing.
    bool checkResource = deserialized_ || !reload_.isModified();
    if (checkResource && rm->hasResource<VolumeSequence>(file_.get())) {
        volumes_ = rm->getResource<VolumeSequence>(file_.get());
    } else {
        try {
            if (auto volVecReader = rf->getReaderForTypeAndExtension<VolumeSequence>(ext)) {
                auto volumes = volVecReader->readData(file_.get(), this);
                std::swap(volumes, volumes_);
                rm->addResource(file_.get(), volumes_, reload_.isModified());
            } else if (auto volreader = rf->getReaderForTypeAndExtension<Volume>(ext)) {
                auto volume = volreader->readData(file_.get(), this);
                auto volumes = std::make_shared<VolumeSequence>();
                volumes->push_back(volume);
                std::swap(volumes, volumes_);
                rm->addResource(file_.get(), volumes_, reload_.isModified());
            } else {
                LogProcessorError("Could not find a data reader for file: " << file_.get());
            }
        } catch (DataReaderException const& e) {
            LogProcessorError(e.getMessage());
        }
    }

    if (volumes_ && !volumes_->empty() && (*volumes_)[0]) {
        for (auto& prop : volumeDescriptions_) {
            removeProperty(*prop);
        }
        volumeDescriptions_.clear();

        size_t volumeIdx{0};
        volumeDescriptions_.reserve(volumes_->size());
        center_slice_img_data_.resize(volumes_->size());

        // store filename in metadata
        for (auto volume : *volumes_) {
            if (!volume->hasMetaData<StringMetaData>("filename"))
                volume->setMetaData<StringMetaData>("filename", file_.get());

            const auto volume_dim = volume->getDimensions();
            const auto center_slice_idx = volume_dim.z / 2;
            const auto volumeRAM = volume->getRepresentation<VolumeRAM>();

            center_slice_img_data_[volumeIdx] =
                std::vector<unsigned char>(volume_dim.x * volume_dim.y);

            double min_value{std::numeric_limits<double>::max()}, max_value{0.0};
            double min_value_normalized{std::numeric_limits<double>::max()}, max_value_normalized{0.0};
            const size3_t step_size{1};
            for (size_t y{0}; y < volume_dim.y; y += step_size.y) {
                for (size_t x{0}; x < volume_dim.x; x += step_size.x) {
                    const size3_t pt{x, y, center_slice_idx};

                    const auto value = volumeRAM->getAsDouble(pt);
                    const auto value_normalized = volumeRAM->getAsNormalizedDouble(pt);

                    const auto img_value = static_cast<unsigned char>(value * 255.0);
                    const auto img_value_normalized = static_cast<unsigned char>(value_normalized * 255.0);

                    max_value = glm::max(max_value, value);
                    min_value = glm::min(min_value, value);
                    max_value_normalized = glm::max(max_value_normalized, value_normalized);
                    min_value_normalized = glm::min(min_value_normalized, value_normalized);

                    // something is wrong here, img_value_normalized should be the correct one, but the normalization does weird things
                    //center_slice_img_data_[volumeIdx][y * volume_dim.x + x] = img_value_normalized;
                    center_slice_img_data_[volumeIdx][y * volume_dim.x + x] = img_value;
                }
            }

            auto volumeDescription = std::make_shared<VolumeDesriptionProperty>(
                std::string("VolumeDescription") + std::to_string(volumeIdx),
                std::string("Volume Description ") + std::to_string(volumeIdx), volumeIdx,
                center_slice_img_data_[volumeIdx].data(), // sliceData,
                volume_dim);
            volumeDescriptions_.emplace_back(volumeDescription);
            auto prop = volumeDescriptions_.back();
            addProperty(*prop);
            prop->setCollapsed(true);
            prop->onChange([this, prop]() { volumeIndex_.set(prop->image_.getImageIdx()); });
            volumeIdx++;
        }

        basis_.updateForNewEntity(*(*volumes_)[0], deserialize);
        information_.updateForNewVolume(*(*volumes_)[0], deserialize);

        volumeSequence_.updateMax(volumes_->size());
        volumeSequence_.setVisible(volumes_->size() > 1);

        volumeIndex_.set(size_t{0});
        volumeIndex_.setMaxValue(glm::max(static_cast<size_t>(volumes_->size()) - 1, size_t{0}));
    }
}

void VolumeSource::addFileNameFilters() {
    auto rf = app_->getDataReaderFactory();

    file_.clearNameFilters();
    file_.addNameFilter(FileExtension("*", "All Files"));
    file_.addNameFilters(rf->getExtensionsForType<Volume>());
    file_.addNameFilters(rf->getExtensionsForType<VolumeSequence>());
}

void VolumeSource::process() {
    if (file_.isModified() || reload_.isModified()) {
        load(deserialized_);
        deserialized_ = false;
    }

    if (volumes_ && !volumes_->empty() && volumeIndex_ >= 0 && volumeIndex_ < volumes_->size()) {
        volumeSequence_.index_.set(volumeIndex_ + 1);

        const auto& vol = (*volumes_)[volumeIndex_];
        if (!vol) {
            return;
        }

        basis_.updateForNewEntity(*vol);
        information_.updateForNewVolume(*vol);

        outport_.setData(vol);

        // overrides volume data, but we want to set the basis and information from the volume
        /*basis_.updateEntity(*vol);
        information_.updateVolume(*vol);*/
    }
}

void VolumeSource::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    addFileNameFilters();
    deserialized_ = true;
}

}  // namespace inviwo
