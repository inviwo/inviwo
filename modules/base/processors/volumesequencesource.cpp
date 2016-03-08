/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "volumesequencesource.h"
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeSequenceSource::processorInfo_{
    "org.inviwo.VolumeVectorSource",  // Class identifier
    "Volume Vector Source",           // Display name
    "Data Input",                     // Category
    CodeState::Stable,                // Code state
    Tags::CPU,                        // Tags
};
const ProcessorInfo VolumeSequenceSource::getProcessorInfo() const {
    return processorInfo_;
}

VolumeSequenceSource::VolumeSequenceSource()
    : Processor()
    , outport_("data")
    , inputType_("inputType","Input type")
    , file_("filename", "File")
    , folder_("folder", "Folder")
    , filter_("filter_","Filter","*.*")
    , reload_("reload", "Reload data")
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information")
    , isDeserializing_(false) {
    file_.setContentType("volume");
    file_.setDisplayName("Volume file");
    folder_.setContentType("volume");
    folder_.setDisplayName("Volume folder");

    file_.onChange([this]() { load(); });
    reload_.onChange([this]() { load(); });
    folder_.onChange([this]() { load(); });
    folder_.onChange([this]() { load(); });
    filter_.onChange([this]() { load(); });
    

    addFileNameFilters();

    addPort(outport_);

    addProperty(inputType_);
    addProperty(folder_);
    addProperty(filter_);
    addProperty(file_);
    addProperty(reload_);
    addProperty(information_);
    addProperty(basis_);

    inputType_.addOption("singlefile", "SingleFile", InputType::SingleFile);
    inputType_.addOption("folder", "Folder", InputType::Folder);
    inputType_.setCurrentStateAsDefault();
    inputType_.onChange([&](){
        file_.setVisible(inputType_.get() == InputType::SingleFile);
        folder_.setVisible(inputType_.get() == InputType::Folder);
        filter_.setVisible(inputType_.get() == InputType::Folder);
    });

    file_.setVisible(inputType_.get() == InputType::SingleFile);
    folder_.setVisible(inputType_.get() == InputType::Folder);
    filter_.setVisible(inputType_.get() == InputType::Folder);
}

void VolumeSequenceSource::load(bool deserialize) {
    if (isDeserializing_) return;
    switch (inputType_.get()) {
    case InputType::Folder:
        loadFolder();
        break;
    case InputType::SingleFile:
    default:
        loadFile();
        break;
    }
}

void VolumeSequenceSource::loadFile(bool deserialize) {
    if (file_.get().empty()) return;

    auto rf = InviwoApplication::getPtr()->getDataReaderFactory();
    std::string ext = filesystem::getFileExtension(file_.get());
    if (auto reader = rf->getReaderForTypeAndExtension<VolumeSequence>(ext)) {
        try {
            volumes_ = reader->readData(file_.get());
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
    auto rf = InviwoApplication::getPtr()->getDataReaderFactory();

    auto files = filesystem::getDirectoryContents(folder_.get());
    for (auto f : files) {
        auto file = folder_.get() + "/" + f;
        if (filesystem::wildcardStringMatch(filter_, file)) {
            std::string ext = filesystem::getFileExtension(file);
            if (auto reader1 = rf->getReaderForTypeAndExtension<Volume>(ext)) {
                try {
                    auto volume = reader1->readData(file);
                    volumes_->push_back(volume);
                }
                catch (DataReaderException const& e) {
                    LogProcessorError(e.getMessage());
                }
            }
            else if (auto reader2 = rf->getReaderForTypeAndExtension<VolumeSequence>(ext)) {
                try {
                    auto volumes = reader2->readData(file);
                    for (auto volume : *volumes) {
                        volumes_->push_back(volume);
                    }
                }
                catch (DataReaderException const& e) {
                    LogProcessorError(e.getMessage());
                }
            }
            else {
                LogProcessorError("Could not find a data reader for file: " << file);
            }
        }
    }

    if (volumes_ && !volumes_->empty() && (*volumes_)[0]) {
        basis_.updateForNewEntity(*(*volumes_)[0], deserialize);
        information_.updateForNewVolume(*(*volumes_)[0], deserialize);
    }
}

void VolumeSequenceSource::addFileNameFilters() {
    auto rf = InviwoApplication::getPtr()->getDataReaderFactory();
    auto extensions = rf->getExtensionsForType<VolumeSequence>();
    file_.clearNameFilters();
    file_.addNameFilter(FileExtension("*", "All Files"));
    for (auto& ext : extensions) {
        file_.addNameFilter(ext.description_ + " (*." + ext.extension_ + ")");
    }
}

void VolumeSequenceSource::process() {
    if (!isDeserializing_ && volumes_ && !volumes_->empty()) {
        for (auto& vol : *volumes_) {
            basis_.updateEntity(*vol);
            information_.updateVolume(*vol);
        }
        outport_.setData(volumes_);
    }
}

void VolumeSequenceSource::deserialize(Deserializer& d) {
    {
        isDeserializing_ = true;
        Processor::deserialize(d);
        addFileNameFilters();
        isDeserializing_ = false;
    }
    load(true);
}
}  // namespace

