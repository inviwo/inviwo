/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/base/processors/layersequencesource.h>

#include <inviwo/core/common/factoryutil.h>                    // for getDataReaderFactory
#include <inviwo/core/io/datareader.h>                         // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>                // for DataReaderException
#include <inviwo/core/io/datareaderfactory.h>                  // for DataReaderFactory
#include <inviwo/core/metadata/metadata.h>                     // for StringMetaData
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>             // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>              // for Tags, Tags::CPU
#include <inviwo/core/properties/buttonproperty.h>             // for ButtonProperty
#include <inviwo/core/properties/directoryproperty.h>          // for DirectoryProperty
#include <inviwo/core/properties/fileproperty.h>               // for FileProperty
#include <inviwo/core/properties/optionproperty.h>             // for OptionProperty, OptionPro...
#include <inviwo/core/properties/property.h>                   // for OverwriteState, Overwrite...
#include <inviwo/core/properties/stringproperty.h>             // for StringProperty
#include <inviwo/core/util/fileextension.h>                    // for FileExtension, operator==
#include <inviwo/core/util/filesystem.h>                       // for fileExists, getDirectoryC...
#include <inviwo/core/util/logcentral.h>                       // for LogCentral, LogProcessorE...
#include <inviwo/core/util/statecoordinator.h>                 // for StateCoordinator
#include <inviwo/core/util/staticstring.h>                     // for operator+
#include <modules/base/processors/datasource.h>                // for updateFilenameFilters
#include <modules/base/properties/basisproperty.h>             // for BasisProperty
#include <modules/base/properties/layerinformationproperty.h>  // for VolumeInformationProperty

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerSequenceSource::processorInfo_{
    "org.inviwo.LayerSequenceSource",  // Class identifier
    "Layer Sequence Source",           // Display name
    "Undefined",                       // Category
    CodeState::Experimental,           // Code state
    Tags::None,                        // Tags
    R"(<Explanation of how to use the processor.>)"_unindentHelp};

const ProcessorInfo LayerSequenceSource::getProcessorInfo() const { return processorInfo_; }

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

LayerSequenceSource::LayerSequenceSource(InviwoApplication* app)
    : Processor()
    , rf_{util::getDataReaderFactory(app)}
    , outport_("data", "A sequence of layers"_help)
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

    file_.setContentType("layer");
    folder_.setContentType("layer");

    addPort(outport_);
    addProperties(inputType_, folder_, filter_, file_, reload_, information_, basis_);

    // It does not make sense to change these for an entire sequence
    information_.setReadOnly(true);
    basis_.setReadOnly(true);

    util::updateFilenameFilters<LayerSequence>(*rf_, file_, reader_);
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

void LayerSequenceSource::load(bool deserialize) {
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

void LayerSequenceSource::loadFile(bool deserialize) {
    if (file_.get().empty()) return;

    const auto sext = file_.getSelectedExtension();
    if (auto reader = rf_->getReaderForTypeAndExtension<LayerSequence>(sext, file_.get())) {
        try {
            layers_ = reader->readData(file_.get(), this);
        } catch (const DataReaderException& e) {
            LogProcessorError(e.getMessage());
            layers_.reset();
            loadingFailed_ = true;
            isReady_.update();
        }
    } else {
        LogProcessorError("Could not find a data reader for file: " << file_.get());
        layers_.reset();
        loadingFailed_ = true;
        isReady_.update();
    }

    if (layers_ && !layers_->empty() && (*layers_)[0]) {
        basis_.updateForNewEntity(*(*layers_)[0], deserialize);
        const auto overwrite = deserialized_ ? util::OverwriteState::Yes : util::OverwriteState::No;
        information_.updateForNewLayer(*(*layers_)[0], overwrite);
    }
}

void LayerSequenceSource::loadFolder(bool deserialize) {
    if (folder_.get().empty()) return;

    layers_ = std::make_shared<LayerSequence>();

    auto files = filesystem::getDirectoryContents(folder_.get());
    for (auto f : files) {
        auto file = folder_.get() / f;
        if (filesystem::wildcardStringMatch(filter_, file.generic_string())) {
            try {
                if (auto reader1 = rf_->getReaderForTypeAndExtension<Layer>(file)) {
                    auto layer = reader1->readData(file, this);
                    layers_->push_back(layer);

                } else if (auto reader2 = rf_->getReaderForTypeAndExtension<LayerSequence>(file)) {
                    auto layers = reader2->readData(file, this);
                    for (auto layer : *layers) {
                        layers_->push_back(layer);
                    }
                } else {
                    LogProcessorError("Could not find a data reader for file: " << file);
                    layers_.reset();
                    loadingFailed_ = true;
                    isReady_.update();
                }
            } catch (const DataReaderException& e) {
                LogProcessorError(e.getMessage());
                layers_.reset();
                loadingFailed_ = true;
                isReady_.update();
            }
        }
    }

    if (layers_ && !layers_->empty()) {
        // set basis of first layer
        if ((*layers_)[0]) {
            basis_.updateForNewEntity(*(*layers_)[0], deserialize);
            const auto overwrite =
                deserialized_ ? util::OverwriteState::Yes : util::OverwriteState::No;
            information_.updateForNewLayer(*(*layers_)[0], overwrite);
        }
    } else {
        outport_.detachData();
    }
}

void LayerSequenceSource::process() {
    if (file_.isModified() || reload_.isModified() || folder_.isModified() ||
        filter_.isModified() || reader_.isModified()) {
        load(deserialized_);
        deserialized_ = false;
    }

    if (layers_ && !layers_->empty()) {
        outport_.setData(layers_);
    }
}

void LayerSequenceSource::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    util::updateFilenameFilters<LayerSequence>(*rf_, file_, reader_);
    // It does not make sense to change these for an entire sequence
    information_.setReadOnly(true);
    basis_.setReadOnly(true);
    deserialized_ = true;
}

}  // namespace inviwo
