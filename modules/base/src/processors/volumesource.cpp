/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <fmt/format.h>  // for format
#include <fmt/std.h>

namespace inviwo {
class Deserializer;

namespace {

template <typename T>
auto getReaderFor(const FileProperty& filePath, OptionProperty<FileExtension>& extensions,
                  const DataReaderFactory& rf) {

    const auto& opts = extensions.getOptions();
    const auto it =
        std::find_if(opts.begin(), opts.end(), [&](const OptionPropertyOption<FileExtension>& opt) {
            if (opt.value_.matches(filePath.get())) {
                return rf.hasReaderForTypeAndExtension<T>(opt.value_);
            }
            return false;
        });
    return it;
}

template <typename... Types>
inline void updateReaderFromFileAndType(const FileProperty& filePath,
                                        OptionProperty<FileExtension>& extensions,
                                        const DataReaderFactory& rf) {
    if (extensions.empty()) return;

    if ((filePath.getSelectedExtension() == FileExtension::all() &&
         !extensions.getSelectedValue().matches(filePath)) ||
        filePath.getSelectedExtension().empty()) {

        for (auto& it : {getReaderFor<Types>(filePath, extensions, rf)...}) {
            if (it != extensions.getOptions().end()) {
                extensions.setSelectedValue(it->value_);
                return;
            }
        }
        extensions.setSelectedValue(FileExtension{});
    } else {
        extensions.setSelectedValue(filePath.getSelectedExtension());
    }
}
}  // namespace

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

const ProcessorInfo& VolumeSource::getProcessorInfo() const { return processorInfo_; }

VolumeSource::VolumeSource(InviwoApplication* app, const std::filesystem::path& filePath)
    : Processor()
    , app_(app)
    , outport_("data", "The loaded volume"_help)
    , file_("filename", "Volume file", "File to load"_help, filePath, AcceptMode::Open,
            FileMode::ExistingFile, "volume")
    , reader_("reader", "Data Reader", "The selected reader used for loading the Volume"_help)
    , reload_("reload", "Reload data",
              "Reload the date from disk, will not use the resource manager"_help)
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information")
    , volumeSequence_("Sequence", "Sequence") {

    addPort(outport_);
    addProperties(file_, reader_, reload_, information_, basis_, volumeSequence_);
    volumeSequence_.setVisible(false);

    util::updateFilenameFilters<VolumeSequence, Volume>(*util::getDataReaderFactory(app), file_,
                                                        reader_);
    reader_.setCurrentStateAsDefault();

    auto* rf = util::getDataReaderFactory(app_);
    updateReaderFromFileAndType<VolumeSequence, Volume>(file_, reader_, *rf);

    // make sure that we always process even if not connected
    isSink_.setUpdate([]() { return true; });
    isReady_.setUpdate([this]() -> ProcessorStatus {
        if (!error_.empty()) {
            return {ProcessorStatus::Error, error_};
        } else if (file_.get().empty()) {
            static constexpr std::string_view reason{"File not set"};
            return {ProcessorStatus::NotReady, reason};
        } else if (!std::filesystem::is_regular_file(file_.get())) {
            static constexpr std::string_view reason{"Invalid or missing file"};
            return {ProcessorStatus::Error, reason};
        } else if (reader_.getSelectedValue().empty()) {
            static constexpr std::string_view reason{"No reader found for file"};
            return {ProcessorStatus::Error, reason};
        } else {
            return ProcessorStatus::Ready;
        }
    });
    file_.onChange([this]() {
        auto* rf = util::getDataReaderFactory(app_);
        updateReaderFromFileAndType<VolumeSequence, Volume>(file_, reader_, *rf);
        error_.clear();
        isReady_.update();
    });
    reader_.onChange([this]() {
        error_.clear();
        isReady_.update();
    });
}

void VolumeSource::load(bool deserialize) {
    if (file_.get().empty()) return;

    auto rf = util::getDataReaderFactory(app_);
    const auto sext = reader_.getSelectedValue();

    try {
        if (auto volumeSequenceReader =
                rf->getReaderForTypeAndExtension<VolumeSequence>(sext, file_.get())) {
            auto volumes = volumeSequenceReader->readData(file_.get(), this);
            std::swap(volumes, volumes_);
        } else if (auto volumeReader =
                       rf->getReaderForTypeAndExtension<Volume>(sext, file_.get())) {
            auto volume = volumeReader->readData(file_.get(), this);
            auto volumes = std::make_shared<VolumeSequence>();
            volumes->push_back(volume);
            std::swap(volumes, volumes_);
        } else {
            volumes_.reset();
            error_ = fmt::format("Could not find a data reader for file: {}", file_.get());
            isReady_.update();
            log::report(LogLevel::Error, error_);
        }
    } catch (const DataReaderException& e) {
        volumes_.reset();
        error_ = e.getMessage();
        isReady_.update();
        log::report(LogLevel::Error, error_);
    }

    if (volumes_ && !volumes_->empty() && (*volumes_)[0]) {
        // store filename in metadata
        for (auto volume : *volumes_) {
            if (!volume->hasMetaData<StringMetaData>("filename")) {
                volume->setMetaData<StringMetaData>("filename", file_.get().generic_string());
            }
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
        outport_.clear();
    }
}

void VolumeSource::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    util::updateFilenameFilters<VolumeSequence, Volume>(*util::getDataReaderFactory(app_), file_,
                                                        reader_);
    deserialized_ = true;
}

}  // namespace inviwo
