/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <modules/base/processors/datasource.h>
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

#include <algorithm>
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
    , file_("filename", "Volume file", file, "volume")
    , reader_("reader", "Data Reader")
    , reload_("reload", "Reload data")
	, volumeIndex_("volumeindex", "Volume Index", 0, 0, 0, 1)
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information")
    , volumeSequence_("Sequence", "Sequence") {

    addPort(outport_);
    addProperties(file_, reader_, reload_, volumeIndex_, information_, basis_, volumeSequence_);
    volumeSequence_.setVisible(false);

    util::updateFilenameFilters<Volume, VolumeSequence>(*app_->getDataReaderFactory(), file_,
                                                        reader_);
    util::updateReaderFromFile(file_, reader_);

    // make sure that we always process even if not connected
    isSink_.setUpdate([]() { return true; });
    isReady_.setUpdate([this]() {
        return !loadingFailed_ && filesystem::fileExists(file_.get()) &&
               !reader_.getSelectedValue().empty();
    });
    file_.onChange([this]() {
        loadingFailed_ = false;
        util::updateReaderFromFile(file_, reader_);
        isReady_.update();
    });
    reader_.onChange([this]() {
        loadingFailed_ = false;
        isReady_.update();
    });
}

void VolumeSource::load(bool deserialize) {
    if (file_.get().empty()) return;

    auto rf = app_->getDataReaderFactory();
    auto rm = app_->getResourceManager();

    const auto sext = reader_.getSelectedValue();
    const auto fext = filesystem::getFileExtension(file_.get());

    // use resource unless the "Reload data"-button (reload_) was pressed,
    // Note: reload_ will be marked as modified when deserializing.
    bool checkResource = deserialized_ || !reload_.isModified();
    if (checkResource && rm->hasResource<VolumeSequence>(file_.get())) {
        volumes_ = rm->getResource<VolumeSequence>(file_.get());
    } else {
        try {
            if (auto volVecReader = rf->getReaderForTypeAndExtension<VolumeSequence>(sext, fext)) {
                auto volumes = volVecReader->readData(file_.get(), this);
                std::swap(volumes, volumes_);
                rm->addResource(file_.get(), volumes_, reload_.isModified());
            } else if (auto volreader = rf->getReaderForTypeAndExtension<Volume>(sext, fext)) {
                auto volume = volreader->readData(file_.get(), this);
                auto volumes = std::make_shared<VolumeSequence>();
                volumes->push_back(volume);
                std::swap(volumes, volumes_);
                rm->addResource(file_.get(), volumes_, reload_.isModified());
            } else {
                LogProcessorError("Could not find a data reader for file: " << file_.get());
                volumes_.reset();
                loadingFailed_ = true;
                isReady_.update();
            }
        } catch (DataReaderException const& e) {
            LogProcessorError(e.getMessage());
            volumes_.reset();
            loadingFailed_ = true;
            isReady_.update();
        }
    }

    if (volumes_ && !volumes_->empty() && (*volumes_)[0]) {
		for (auto& prop : volumeDescriptions_) {
			removeProperty(*prop);
		}
		volumeDescriptions_.clear();

		size_t volumeIdx{ 0 };
		volumeDescriptions_.reserve(volumes_->size());
		center_slice_img_data_.resize(volumes_->size());

        // store filename in metadata
        for (auto volume : *volumes_) {
            if (!volume->hasMetaData<StringMetaData>("filename"))
                volume->setMetaData<StringMetaData>("filename", file_.get());

			//TODO take code from de9005af878a3ccb9169d5affea5803143549e73

			// extract center slice to show in image property
			const auto volume_dim = volume->getDimensions();
			const auto center_slice_idx = volume_dim.z / 2;
			const auto volumeRAM = volume->getRepresentation<VolumeRAM>();

			center_slice_img_data_[volumeIdx] =
				std::vector<unsigned char>(volume_dim.x * volume_dim.y);

			double min_value{ std::numeric_limits<double>::max() }, max_value{ 0.0 };
			double min_value_normalized_manually{ std::numeric_limits<double>::max() },
				max_value_normalized_manually{ 0.0 };

			const size3_t step_size{ 1 };
			for (size_t z{ 0 }; z < volume_dim.z; z += step_size.z) {
				for (size_t y{ 0 }; y < volume_dim.y; y += step_size.y) {
					for (size_t x{ 0 }; x < volume_dim.x; x += step_size.x) {
						const size3_t pt{ x, y, z };

						const auto value = volumeRAM->getAsDouble(pt); // in data range

						const auto value_normalized_manually =
							(value - volume->dataMap_.dataRange.x) /
							(volume->dataMap_.dataRange.y - volume->dataMap_.dataRange.x)
							* volume->dataMap_.rescaleSlope + volume->dataMap_.rescaleIntercept;

						const auto img_value =
							static_cast<unsigned char>(value_normalized_manually * 255.0);

						max_value = glm::max(max_value, value);
						min_value = glm::min(min_value, value);

						max_value_normalized_manually =
							glm::max(max_value_normalized_manually, value_normalized_manually);
						min_value_normalized_manually =
							glm::min(min_value_normalized_manually, value_normalized_manually);
					}
				}
			}

			LogInfo("############");
			LogInfo("value range: " << volume->dataMap_.valueRange);
			LogInfo("data range:  " << volume->dataMap_.dataRange);
			LogInfo("slope:       " << volume->dataMap_.rescaleSlope);
			LogInfo("intercept:   " << volume->dataMap_.rescaleIntercept);
			LogInfo("min/max:     " << glm::dvec2(min_value, max_value));
			LogInfo("min/max(nm): " << glm::dvec2(min_value_normalized_manually,
				max_value_normalized_manually));

			unsigned char min_value_img{ std::numeric_limits<unsigned char>::max() },
				max_value_img{ 0 };
			for (size_t y{ 0 }; y < volume_dim.y; y += step_size.y) {
				for (size_t x{ 0 }; x < volume_dim.x; x += step_size.x) {
					const size3_t pt{ x, y, center_slice_idx };

					const auto value = volumeRAM->getAsDouble(pt); // in data range


					const auto value_normalized_manually =
						(value - volume->dataMap_.dataRange.x) /
						(volume->dataMap_.dataRange.y - volume->dataMap_.dataRange.x);// * volume->dataMap_.rescaleSlope + volume->dataMap_.rescaleIntercept;

					const auto img_value =
						static_cast<unsigned char>(value_normalized_manually * 255.0);

					max_value_img = glm::max(max_value_img, img_value);
					min_value_img = glm::min(min_value_img, img_value);



					center_slice_img_data_[volumeIdx][y * volume_dim.x + x] = img_value;
				}
			}

			LogInfo("min/max(img):" << glm::dvec2(min_value_img, max_value_img));

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

		volumeIndex_.set(size_t{ 0 });
		volumeIndex_.setMaxValue(glm::max(static_cast<size_t>(volumes_->size()) - 1, size_t{ 0 }));
    }
}

void VolumeSource::process() {
    if (file_.isModified() || reload_.isModified() || reader_.isModified()) {
        load(deserialized_);
        deserialized_ = false;
    }

	if (volumes_ && !volumes_->empty() && volumeIndex_ >= 0 && volumeIndex_ < volumes_->size()) {
		volumeSequence_.index_.set(volumeIndex_ + 1);

		const auto& vol = (*volumes_)[volumeIndex_];
		if (!vol) {
			return;
		}

		basis_.updateForNewEntity(*vol, deserialized_);
		information_.updateForNewVolume(*vol);

		outport_.setData(vol);

    } else {
        outport_.detachData();
    }
}

void VolumeSource::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    util::updateFilenameFilters<Volume, VolumeSequence>(*app_->getDataReaderFactory(), file_,
                                                        reader_);
    deserialized_ = true;
}

}  // namespace inviwo
