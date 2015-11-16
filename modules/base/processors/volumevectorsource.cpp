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

#include "volumevectorsource.h"
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeVectorSource::processorInfo_{
    "org.inviwo.VolumeVectorSource",  // Class identifier
    "Volume Vector Source",           // Display name
    "Data Input",                     // Category
    CodeState::Stable,                // Code state
    Tags::CPU,                        // Tags
};
const ProcessorInfo VolumeVectorSource::getProcessorInfo() const {
    return processorInfo_;
}

VolumeVectorSource::VolumeVectorSource()
    : Processor()
    , outport_("data")
    , file_("filename", "File")
    , reload_("reload", "Reload data")
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information")
    , isDeserializing_(false) {
    file_.setContentType("volume");
    file_.setDisplayName("Volume file");

    file_.onChange([this]() { load(); });
    reload_.onChange([this]() { load(); });

    addFileNameFilters();

    addPort(outport_);

    addProperty(file_);
    addProperty(reload_);
    addProperty(information_);
    addProperty(basis_);
}

void VolumeVectorSource::load(bool deserialize /*= false*/) {
    if (isDeserializing_ || file_.get().empty()) return;

    auto rf = InviwoApplication::getPtr()->getDataReaderFactory();
    std::string ext = filesystem::getFileExtension(file_.get());
    if (auto reader = rf->getReaderForTypeAndExtension<VolumeVector>(ext)) {
        try {
            volumes_ = reader->readData(file_.get());
        } catch (DataReaderException const& e) {
            LogProcessorError("Could not load data: " << file_.get() << ", " << e.getMessage());
        }
    } else {
        LogProcessorError("Could not find a data reader for file: " << file_.get());
    }

    if (volumes_ && !volumes_->empty() && (*volumes_)[0]) {
        basis_.updateForNewEntity(*(*volumes_)[0], deserialize);
        information_.updateForNewVolume(*(*volumes_)[0], deserialize);
    }
}

void VolumeVectorSource::addFileNameFilters() {
    auto rf = InviwoApplication::getPtr()->getDataReaderFactory();
    auto extensions = rf->getExtensionsForType<VolumeVector>();
    file_.clearNameFilters();
    file_.addNameFilter(FileExtension("*", "All Files"));
    for (auto& ext : extensions) {
        file_.addNameFilter(ext.description_ + " (*." + ext.extension_ + ")");
    }
}

void VolumeVectorSource::process() {
    if (!isDeserializing_ && volumes_ && !volumes_->empty()) {
        for (auto& vol : *volumes_) {
            basis_.updateEntity(*vol);
            information_.updateVolume(*vol);
        }
        outport_.setData(volumes_);
    }
}

void VolumeVectorSource::deserialize(Deserializer& d) {
    {
        isDeserializing_ = true;
        Processor::deserialize(d);
        addFileNameFilters();
        isDeserializing_ = false;
    }
    load(true);
}
}  // namespace

