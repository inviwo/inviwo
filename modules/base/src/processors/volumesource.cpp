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
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information")
    , volumeSequence_("Sequence", "Sequence") {

    addPort(outport_);
    addProperties(file_, reader_, reload_, information_, basis_, volumeSequence_);
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
        // store filename in metadata
        for (auto volume : *volumes_) {
            if (!volume->hasMetaData<StringMetaData>("filename"))
                volume->setMetaData<StringMetaData>("filename", file_.get());
        }

        basis_.updateForNewEntity(*(*volumes_)[0], deserialize);
        information_.updateForNewVolume(*(*volumes_)[0], deserialize);

        volumeSequence_.updateMax(volumes_->size());
        volumeSequence_.setVisible(volumes_->size() > 1);
    }
}

void VolumeSource::process() {
    if (file_.isModified() || reload_.isModified() || reader_.isModified()) {
        load(deserialized_);
        deserialized_ = false;
    }

    if (volumes_ && !volumes_->empty()) {
        const size_t index = std::min(volumes_->size(), volumeSequence_.index_.get()) - 1;

        if (!(*volumes_)[index]) return;

        basis_.updateEntity(*(*volumes_)[index]);
        information_.updateVolume(*(*volumes_)[index]);

        outport_.setData((*volumes_)[index]);
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
