/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include <inviwo/core/algorithm/markdown.h>                     // for operator""_help
#include <inviwo/core/common/factoryutil.h>                     // for getDataReaderFactory, get...
#include <inviwo/core/datastructures/volume/volume.h>           // for Volume
#include <inviwo/core/io/datareader.h>                          // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>                 // for DataReaderException
#include <inviwo/core/io/datareaderfactory.h>                   // for DataReaderFactory
#include <inviwo/core/metadata/metadata.h>                      // for StringMetaData
#include <inviwo/core/ports/volumeport.h>                       // for VolumeOutport
#include <inviwo/core/processors/processor.h>                   // for Processor
#include <inviwo/core/processors/processorinfo.h>               // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>              // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>               // for Tags, Tags::CPU
#include <inviwo/core/properties/buttonproperty.h>              // for ButtonProperty
#include <inviwo/core/properties/fileproperty.h>                // for FileProperty
#include <inviwo/core/properties/optionproperty.h>              // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>             // for IntSizeTProperty
#include <inviwo/core/properties/property.h>                    // for OverwriteState, Overwrite...
#include <inviwo/core/resourcemanager/resourcemanager.h>        // for ResourceManager
#include <inviwo/core/util/fileextension.h>                     // for FileExtension, operator==
#include <inviwo/core/util/filesystem.h>                        // for fileExists
#include <inviwo/core/util/logcentral.h>                        // for LogCentral, LogProcessorE...
#include <inviwo/core/util/statecoordinator.h>                  // for StateCoordinator
#include <modules/base/processors/datasource.h>                 // for updateFilenameFilters
#include <modules/base/properties/basisproperty.h>              // for BasisProperty
#include <modules/base/properties/sequencetimerproperty.h>      // for SequenceTimerProperty
#include <modules/base/properties/volumeinformationproperty.h>  // for VolumeInformationProperty

#include <algorithm>    // for min
#include <cstddef>      // for size_t
#include <map>          // for map, operator!=
#include <ostream>      // for operator<<
#include <type_traits>  // for remove_extent_t
#include <utility>      // for move

#include <fmt/core.h>  // for format

namespace inviwo {
class Deserializer;

const ProcessorInfo VolumeSource::processorInfo_{
    "org.inviwo.VolumeSource",  // Class identifier
    "Volume Source",            // Display name
    "Data Input",               // Category
    CodeState::Stable,          // Code state
    Tags::CPU,                  // Tags
    "Loads a Volume or Volume Sequence from a given file. "
    "If the 'Resource Manager' is active the Volume is cached and loaded from the manager if found."
    "Various information of the volume is displayed in the property pane. If a volume sequence is "
    "loaded a slider to select a volume is shown. "
    "The current filename is stored in the loaded volume as 'filename' MetaData."_help};

const ProcessorInfo VolumeSource::getProcessorInfo() const { return processorInfo_; }

VolumeSource::VolumeSource(InviwoApplication* app, std::string_view filePath)
    : Processor()
    , app_(app)
    , outport_("data", "The loaded volume"_help)
    , file_("filename", "Volume file", "File to load"_help, filePath, "volume")
    , reader_("reader", "Data Reader", "The selected reader used for loading the Volume"_help)
    , reload_("reload", "Reload data",
              "Reload the date from disk, will not use the resource manager"_help)
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information")
    , volumeSequence_("Sequence", "Sequence") {

    addPort(outport_);
    addProperties(file_, reader_, reload_, information_, basis_, volumeSequence_);
    volumeSequence_.setVisible(false);

    // The default state of a property is the one provided in the constructor (filePath). The
    // default state is not serialized. So, to ensure that a custom file path is serialied we set
    // the default state to ""
    if (!filePath.empty()) {
        Property::setStateAsDefault(file_, std::string{});
    }

    util::updateFilenameFilters<Volume, VolumeSequence>(*util::getDataReaderFactory(app), file_,
                                                        reader_);
    reader_.setCurrentStateAsDefault();

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

    auto rf = util::getDataReaderFactory(app_);
    auto rm = util::getResourceManager(app_);

    const auto sext = reader_.getSelectedValue();

    // use resource unless the "Reload data"-button (reload_) was pressed,
    // Note: reload_ will be marked as modified when deserializing.
    bool checkResource = deserialized_ || !reload_.isModified();
    if (checkResource && rm->hasResource<VolumeSequence>(file_.get())) {
        volumes_ = rm->getResource<VolumeSequence>(file_.get());
    } else {
        try {
            if (auto volVecReader =
                    rf->getReaderForTypeAndExtension<VolumeSequence>(sext, file_.get())) {
                auto volumes = volVecReader->readData(file_.get(), this);
                std::swap(volumes, volumes_);
                rm->addResource(file_.get(), volumes_, reload_.isModified());
            } else if (auto volreader =
                           rf->getReaderForTypeAndExtension<Volume>(sext, file_.get())) {
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
        information_.updateForNewVolume(
            *(*volumes_)[0], deserialize ? util::OverwriteState::Yes : util::OverwriteState::No);

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
    util::updateFilenameFilters<Volume, VolumeSequence>(*util::getDataReaderFactory(app_), file_,
                                                        reader_);
    deserialized_ = true;
}

}  // namespace inviwo
