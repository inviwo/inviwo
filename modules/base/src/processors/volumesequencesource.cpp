/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <inviwo/core/common/factoryutil.h>                     // for getDataReaderFactory
#include <inviwo/core/datastructures/volume/volume.h>           // for VolumeSequence, Volume
#include <inviwo/core/io/datareader.h>                          // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>                 // for DataReaderException
#include <inviwo/core/io/datareaderfactory.h>                   // for DataReaderFactory
#include <inviwo/core/metadata/metadata.h>                      // for StringMetaData
#include <inviwo/core/ports/volumeport.h>                       // for VolumeSequenceOutport
#include <inviwo/core/processors/processor.h>                   // for Processor
#include <inviwo/core/processors/processorinfo.h>               // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>              // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>               // for Tags, Tags::CPU
#include <inviwo/core/properties/buttonproperty.h>              // for ButtonProperty
#include <inviwo/core/properties/directoryproperty.h>           // for DirectoryProperty
#include <inviwo/core/properties/fileproperty.h>                // for FileProperty
#include <inviwo/core/properties/optionproperty.h>              // for OptionProperty, OptionPro...
#include <inviwo/core/properties/property.h>                    // for OverwriteState, Overwrite...
#include <inviwo/core/properties/stringproperty.h>              // for StringProperty
#include <inviwo/core/util/fileextension.h>                     // for FileExtension, operator==
#include <inviwo/core/util/filesystem.h>                        // for fileExists, getDirectoryC...
#include <inviwo/core/util/logcentral.h>                        // for LogCentral, LogProcessorE...
#include <inviwo/core/util/statecoordinator.h>                  // for StateCoordinator
#include <inviwo/core/util/staticstring.h>                      // for operator+
#include <modules/base/processors/datasource.h>                 // for updateFilenameFilters
#include <modules/base/properties/basisproperty.h>              // for BasisProperty
#include <modules/base/properties/volumeinformationproperty.h>  // for VolumeInformationProperty

#include <map>          // for map, operator!=
#include <ostream>      // for operator<<
#include <type_traits>  // for remove_extent_t
#include <utility>      // for move

namespace inviwo {
class Deserializer;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeSequenceSource::processorInfo_{
    "org.inviwo.VolumeVectorSource",  // Class identifier
    "Volume Sequence Source",         // Display name
    "Data Input",                     // Category
    CodeState::Stable,                // Code state
    Tags::CPU,                        // Tags
    R"(Loads a sequence of volume either from a 4D dataset or from a selection of 3D datasets. The
       filename of the source data is available via MetaData.)"_unindentHelp};

const ProcessorInfo VolumeSequenceSource::getProcessorInfo() const { return processorInfo_; }

namespace {

std::optional<std::filesystem::path> getFirstFileInFolder(const std::filesystem::path& folder,
                                                          const std::string& filter) {
    auto files = filesystem::getDirectoryContents(folder);
    for (auto f : files) {
        auto file = folder / f;
        if (filesystem::wildcardStringMatch(filter, file.generic_string())) {
            return file;
        }
    }
    return std::nullopt;
}

}  // namespace

VolumeSequenceSource::VolumeSequenceSource(InviwoApplication* app)
    : Processor()
    , rf_{util::getDataReaderFactory(app)}
    , outport_("data", "A sequence of volumes"_help)
    , inputType_(
          "inputType", "Input type",
          "Select the input type, either select a single file to a 4D dataset or use a folder"_help,
          {{"singlefile", "Single File", InputType::SingleFile},
           {"folder", "Folder", InputType::Folder}},
          1)
    , file_("filename", "Volume file", "If using single file mode, the file to load"_help)
    , folder_("folder", "Volume folder",
              "If using folder mode, the folder to look for data sets in"_help)
    , filter_(
          "filter_", "Filter",
          "If using folder mode, apply filter to the folder contents to find wanted data sets"_help,
          "*")
    , reader_("reader", "Data Reader")
    , reload_("reload", "Reload data")
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information") {
    file_.setContentType("volume");
    folder_.setContentType("volume");

    addPort(outport_);
    addProperties(inputType_, folder_, filter_, file_, reload_, information_, basis_);

    // It does not make sense to change these for an entire sequence
    information_.setReadOnly(true);
    basis_.setReadOnly(true);

    util::updateFilenameFilters<VolumeSequence>(*rf_, file_, reader_);
    util::updateReaderFromFile(file_, reader_);

    auto singlefileCallback = [](auto& p) { return p.get() == InputType::SingleFile; };
    auto folderCallback = [](auto& p) { return p.get() == InputType::Folder; };

    file_.visibilityDependsOn(inputType_, singlefileCallback);
    reader_.visibilityDependsOn(inputType_, singlefileCallback);
    folder_.visibilityDependsOn(inputType_, folderCallback);
    filter_.visibilityDependsOn(inputType_, folderCallback);

    // make sure that we always process even if not connected
    isSink_.setUpdate([]() { return true; });
    isReady_.setUpdate([this]() {
        if (inputType_ == InputType::SingleFile) {
            return !loadingFailed_ && std::filesystem::is_regular_file(file_.get()) &&
                   !reader_.getSelectedValue().empty();
        } else {
            if (auto first = getFirstFileInFolder(folder_, filter_)) {
                return !loadingFailed_ && std::filesystem::is_regular_file(*first);
            } else {
                return false;
            }
        }
    });

    auto change = [this]() {
        loadingFailed_ = false;
        isReady_.update();
    };
    file_.onChange([this, change]() {
        util::updateReaderFromFile(file_, reader_);
        change();
    });
    reader_.onChange(change);
    folder_.onChange(change);
    filter_.onChange(change);
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
    if (auto reader = rf_->getReaderForTypeAndExtension<VolumeSequence>(sext, file_.get())) {
        try {
            volumes_ = reader->readData(file_.get(), this);
        } catch (DataReaderException const& e) {
            LogProcessorError(e.getMessage());
            volumes_.reset();
            loadingFailed_ = true;
            isReady_.update();
        }
    } else {
        LogProcessorError("Could not find a data reader for file: " << file_.get());
        volumes_.reset();
        loadingFailed_ = true;
        isReady_.update();
    }

    if (volumes_ && !volumes_->empty() && (*volumes_)[0]) {
        basis_.updateForNewEntity(*(*volumes_)[0], deserialize);
        const auto overwrite = deserialized_ ? util::OverwriteState::Yes : util::OverwriteState::No;
        information_.updateForNewVolume(*(*volumes_)[0], overwrite);
    }
}

void VolumeSequenceSource::loadFolder(bool deserialize) {
    if (folder_.get().empty()) return;

    volumes_ = std::make_shared<VolumeSequence>();

    auto files = filesystem::getDirectoryContents(folder_.get());
    for (auto f : files) {
        auto file = folder_.get() / f;
        if (filesystem::wildcardStringMatch(filter_, file.generic_string())) {
            try {
                if (auto reader1 = rf_->getReaderForTypeAndExtension<Volume>(file)) {
                    auto volume = reader1->readData(file, this);
                    volume->setMetaData<StringMetaData>("filename", file.generic_string());
                    volumes_->push_back(volume);

                } else if (auto reader2 = rf_->getReaderForTypeAndExtension<VolumeSequence>(file)) {
                    auto volumes = reader2->readData(file, this);
                    for (auto volume : *volumes) {
                        volume->setMetaData<StringMetaData>("filename", file.generic_string());
                        volumes_->push_back(volume);
                    }
                } else {
                    LogProcessorError("Could not find a data reader for file: " << file);
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
    }

    if (volumes_ && !volumes_->empty()) {
        // store filename in metadata
        for (auto volume : *volumes_) {
            if (!volume->hasMetaData<StringMetaData>("filename"))
                volume->setMetaData<StringMetaData>("filename", file_.get().generic_string());
        }

        // set basis of first volume
        if ((*volumes_)[0]) {
            basis_.updateForNewEntity(*(*volumes_)[0], deserialize);
            const auto overwrite =
                deserialized_ ? util::OverwriteState::Yes : util::OverwriteState::No;
            information_.updateForNewVolume(*(*volumes_)[0], overwrite);
        }
    } else {
        outport_.detachData();
    }
}

void VolumeSequenceSource::process() {
    if (file_.isModified() || reload_.isModified() || folder_.isModified() ||
        filter_.isModified() || reader_.isModified()) {
        load(deserialized_);
        deserialized_ = false;
    }

    if (volumes_ && !volumes_->empty()) {
        outport_.setData(volumes_);
    }
}

void VolumeSequenceSource::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    util::updateFilenameFilters<VolumeSequence>(*rf_, file_, reader_);
    // It does not make sense to change these for an entire sequence
    information_.setReadOnly(true);
    basis_.setReadOnly(true);
    deserialized_ = true;
}

}  // namespace inviwo
