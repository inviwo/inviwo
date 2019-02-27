/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <modules/base/processors/volumesequencesource.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeSequenceSource::processorInfo_{
    "org.inviwo.VolumeVectorSource",  // Class identifier
    "Volume Sequence Source",         // Display name
    "Data Input",                     // Category
    CodeState::Stable,                // Code state
    Tags::CPU,                        // Tags
};
const ProcessorInfo VolumeSequenceSource::getProcessorInfo() const { return processorInfo_; }

VolumeSequenceSource::VolumeSequenceSource(InviwoApplication* app)
    : Processor()
    , rf_{app->getDataReaderFactory()}
    , outport_("data")
    , inputType_("inputType", "Input type",
                 {{"singlefile", "Single File", InputType::SingleFile},
                  {"folder", "Folder", InputType::Folder}},
                 0)
    , file_("filename", "Volume file")
    , folder_("folder", "Volume folder")
    , filter_("filter_", "Filter", "*.*")
    , reload_("reload", "Reload data")
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information") {
    file_.setContentType("volume");
    folder_.setContentType("volume");

    // make sure that we always process even if not connected
    isSink_.setUpdate([]() { return true; });

    addFileNameFilters();

    addPort(outport_);

    addProperty(inputType_);
    addProperty(folder_);
    addProperty(filter_);
    addProperty(file_);
    addProperty(reload_);
    addProperty(information_);
    addProperty(basis_);

    // It does not make sense to change these for an entire sequence
    information_.setReadOnly(true);
    basis_.setReadOnly(true);

    auto updateVisible = [&]() {
        file_.setVisible(inputType_.get() == InputType::SingleFile);
        folder_.setVisible(inputType_.get() == InputType::Folder);
        filter_.setVisible(inputType_.get() == InputType::Folder);
    };

    inputType_.onChange(updateVisible);
    updateVisible();
}

void VolumeSequenceSource::load(bool deserialize) {
    switch (inputType_.get()) {
        case InputType::Folder:
            loadFolder(deserialize);
            break;
        case InputType::SingleFile:
        default:
            loadFile(deserialize);
            break;
    }
}

void VolumeSequenceSource::loadFile(bool deserialize) {
    if (file_.get().empty()) return;

    const auto sext = file_.getSelectedExtension();
    const auto fext = filesystem::getFileExtension(file_.get());
    if (auto reader = rf_->getReaderForTypeAndExtension<VolumeSequence>(sext, fext)) {
        try {
            volumes_ = reader->readData(file_.get(), this);
        } catch (DataReaderException const& e) {
            LogProcessorError(e.getMessage());
        }
    } else {
        LogProcessorError("Could not find a data reader for file: " << file_.get());
    }

    if (volumes_ && !volumes_->empty() && (*volumes_)[0]) {
        basis_.updateForNewEntity(*(*volumes_)[0], deserialize);
        information_.updateForNewVolume(*(*volumes_)[0], deserialize);
    }
}

void VolumeSequenceSource::loadFolder(bool deserialize) {
    if (folder_.get().empty()) return;

    volumes_ = std::make_shared<VolumeSequence>();

    auto files = filesystem::getDirectoryContents(folder_.get());
    for (auto f : files) {
        auto file = folder_.get() + "/" + f;
        if (filesystem::wildcardStringMatch(filter_, file)) {
            std::string ext = filesystem::getFileExtension(file);
            try {
                if (auto reader1 = rf_->getReaderForTypeAndExtension<Volume>(ext)) {
                    auto volume = reader1->readData(file, this);
                    volume->setMetaData<StringMetaData>("filename", file);
                    volumes_->push_back(volume);

                } else if (auto reader2 = rf_->getReaderForTypeAndExtension<VolumeSequence>(ext)) {
                    auto volumes = reader2->readData(file, this);
                    for (auto volume : *volumes) {
                        volume->setMetaData<StringMetaData>("filename", file);
                        volumes_->push_back(volume);
                    }
                } else {
                    LogProcessorError("Could not find a data reader for file: " << file);
                }
            } catch (DataReaderException const& e) {
                LogProcessorError(e.getMessage());
            }
        }
    }

    if (volumes_ && !volumes_->empty()) {
        // store filename in metadata
        for (auto volume : *volumes_) {
            if (!volume->hasMetaData<StringMetaData>("filename"))
                volume->setMetaData<StringMetaData>("filename", file_.get());
        }

        // set basis of first volume
        if ((*volumes_)[0]) {
            basis_.updateForNewEntity(*(*volumes_)[0], deserialize);
            information_.updateForNewVolume(*(*volumes_)[0], deserialize);
        }
    }
}

void VolumeSequenceSource::addFileNameFilters() {
    file_.clearNameFilters();
    file_.addNameFilter(FileExtension::all());
    file_.addNameFilters(rf_->getExtensionsForType<VolumeSequence>());
}

void VolumeSequenceSource::process() {
    if (file_.isModified() || reload_.isModified() || folder_.isModified() ||
        filter_.isModified()) {
        load(deserialized_);
        deserialized_ = false;
    }

    if (volumes_ && !volumes_->empty()) {
        outport_.setData(volumes_);
    }
}

void VolumeSequenceSource::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    addFileNameFilters();
    // It does not make sense to change these for an entire sequence
    information_.setReadOnly(true);
    basis_.setReadOnly(true);
    deserialized_ = true;
}
}  // namespace inviwo
