/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "volumeexport.h"
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/fileextension.h>

namespace inviwo {

ProcessorClassIdentifier(VolumeExport, "org.inviwo.VolumeExport");
ProcessorDisplayName(VolumeExport,  "Volume Export");
ProcessorTags(VolumeExport, Tags::CPU);
ProcessorCategory(VolumeExport, "Data Output");
ProcessorCodeState(VolumeExport, CODE_STATE_STABLE);

VolumeExport::VolumeExport()
    : Processor()
    , volumePort_("volume")
    , volumeFile_("volumeFileName", "Volume file name",
                  InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_VOLUMES, "/newvolume.dat") , "volume")
    , exportVolumeButton_("snapshot", "Export Volume", VALID)
    , overwrite_("overwrite", "Overwrite", false) {
    std::vector<FileExtension> ext = DataWriterFactory::getPtr()->getExtensionsForType<Volume>();

    for (std::vector<FileExtension>::const_iterator it = ext.begin();
         it != ext.end(); ++it) {
        std::stringstream ss;
        ss << it->description_ << " (*." << it->extension_ << ")";
        volumeFile_.addNameFilter(ss.str());
    }

    addPort(volumePort_);
    addProperty(volumeFile_);
    volumeFile_.setAcceptMode(FileProperty::AcceptMode::Save);
    exportVolumeButton_.onChange(this, &VolumeExport::exportVolume);
    addProperty(exportVolumeButton_);
    addProperty(overwrite_);
}

VolumeExport::~VolumeExport() {}

void VolumeExport::initialize() {
    Processor::initialize();
}

void VolumeExport::deinitialize() {
    Processor::deinitialize();
}

void VolumeExport::exportVolume() {
    auto volume = volumePort_.getData();

    if (volume && !volumeFile_.get().empty()) {
        std::string fileExtension = filesystem::getFileExtension(volumeFile_.get());
        auto writer =
            DataWriterFactory::getPtr()->getWriterForTypeAndExtension<Volume>(fileExtension);

        if (writer) {
            try {
                writer->setOverwrite(overwrite_.get());
                writer->writeData(volume.get(), volumeFile_.get());
                LogInfo("Volume exported to disk: " << volumeFile_.get());
            } catch (DataWriterException const& e) {
                util::log(e.getContext(), e.getMessage(), LogLevel::Error);
            }
        } else {
            LogError("Error: Cound not find a writer for the specified extension and data type");
        }
    } else if (volumeFile_.get().empty()) {
        LogWarn("Error: Please specify a file to write to");
    } else if (!volume) {
        LogWarn("Error: Please connect a volume to export");
    }
}

void VolumeExport::process() {}

} // namespace
