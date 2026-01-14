/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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
#pragma once

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/statecoordinator.h>
#include <inviwo/core/util/staticstring.h>
#include <inviwo/core/util/stringconversion.h>
#include <modules/base/processors/datasource.h>
#include <modules/base/properties/basisproperty.h>

#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <string>       // for operator==, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, opera...

#include <fmt/format.h>

namespace inviwo {
class DataReaderFactory;
class Deserializer;
class InviwoApplication;

// Conf struct example
// struct VolumeConf {
//    using Type = Volume;
//    using Sequence = DataSequence<Type>;
//    using Info = VolumeInformationProperty;
//    using Outport = VolumeSequenceOutport;
///   static constexpr auto name = DataTraits<Type>::dataName();
//    static constexpr auto plural = "s";
//    static constexpr size_t dim = 3;
//    static void updateForNew(Info& info, const Type& data, util::OverwriteState overwrite) {
//        info.updateForNewVolume(data, overwrite);
//    }
// };

/**
 * @brief Loads a vector of volumes
 */
template <typename Conf>
class SequenceSource : public Processor {
    enum class InputType : std::uint8_t { SingleFile, Folder };

public:
    using Type = typename Conf::Type;
    using Sequence = typename Conf::Sequence;
    static constexpr std::string_view fileMetaData = "filename";

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    explicit SequenceSource(InviwoApplication* app);
    SequenceSource(const SequenceSource&) = delete;
    SequenceSource(SequenceSource&&) = delete;
    SequenceSource& operator=(const SequenceSource&) = delete;
    SequenceSource& operator=(SequenceSource&&) = delete;
    virtual ~SequenceSource() = default;

    virtual void deserialize(Deserializer& d) override;
    virtual void process() override;

private:
    void load(bool deserialize = false);
    void loadFile(bool deserialize = false);
    void loadFolder(bool deserialize = false);

    void loadAndAddToSequence(const std::filesystem::path& file, Sequence& sequence);

    DataReaderFactory* rf_;
    std::shared_ptr<Sequence> sequence_;

    typename Conf::Outport outport_;

    OptionProperty<InputType> inputType_;
    FileProperty file_;
    DirectoryProperty folder_;
    StringProperty filter_;

    OptionProperty<FileExtension> reader_;
    ButtonProperty reload_;

    BasisProperty basis_;
    typename Conf::Info information_;

    bool deserialized_ = false;
    bool loadingFailed_ = false;
};

template <typename Conf>
struct ProcessorTraits<SequenceSource<Conf>> {
    static ProcessorInfo getProcessorInfo() {
        return {fmt::format("org.inviwo.{}VectorSource", Conf::name),  // Class identifier
                fmt::format("{} Sequence Source", Conf::name),         // Display name
                "Data Input",                                          // Category
                CodeState::Stable,                                     // Code state
                Tags::CPU,                                             // Tags
                Document{fmt::format(
                    "Loads a sequence of {}{} either from a {}D dataset or from a"
                    " selection of {}D datasets. The filename of the source data is available"
                    " via StringMetaData as '{}'",
                    Conf::name, Conf::plural, Conf::dim + 1, Conf::dim,
                    SequenceSource<Conf>::fileMetaData)}};
    }
};

template <typename Conf>
const ProcessorInfo& SequenceSource<Conf>::getProcessorInfo() const {
    static const ProcessorInfo info = ProcessorTraits<SequenceSource<Conf>>::getProcessorInfo();
    return info;
}

namespace util {

IVW_MODULE_BASE_API std::optional<std::filesystem::path> getFirstFileInFolder(
    const std::filesystem::path& folder, const std::string& filter);

}  // namespace util

template <typename Conf>
SequenceSource<Conf>::SequenceSource(InviwoApplication* app)
    : Processor()
    , rf_{util::getDataReaderFactory(app)}
    , outport_("data", Document{fmt::format("A sequence of {}{}", Conf::name, Conf::plural)})
    , inputType_(
          "inputType", "Input type",
          "Select the input type, either select a single file to a 4D dataset or use a folder"_help,
          {{"singlefile", "Single File", InputType::SingleFile},
           {"folder", "Folder", InputType::Folder}},
          1)
    , file_("filename", fmt::format("{} file", Conf::name),
            "If using single file mode, the file to load"_help)
    , folder_("folder", fmt::format("{} folder", Conf::name),
              "If using folder mode, the folder to look for data sets in"_help)
    , filter_(
          "filter_", "Filter",
          "If using folder mode, apply filter to the folder contents to find wanted data sets"_help,
          "*")
    , reader_("reader", "Data Reader")
    , reload_("reload", "Reload data")
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information") {

    file_.setContentType(toLower(Conf::name));
    folder_.setContentType(toLower(Conf::name));

    addPort(outport_);
    addProperties(inputType_, folder_, filter_, file_, reload_, information_, basis_);

    // It does not make sense to change these for an entire sequence
    information_.setReadOnly(true);
    basis_.setReadOnly(true);

    util::updateFilenameFilters<Sequence>(*rf_, file_, reader_);
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
            if (auto first = util::getFirstFileInFolder(folder_, filter_)) {
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

template <typename Conf>
void SequenceSource<Conf>::SequenceSource::load(bool deserialize) {
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

template <typename Conf>
void SequenceSource<Conf>::loadFile(bool deserialize) {
    if (file_.get().empty()) return;

    const auto sext = file_.getSelectedExtension();
    try {
        if (auto reader = rf_->getReaderForTypeAndExtension<Sequence>(sext, file_.get())) {
            sequence_ = reader->readData(file_.get(), this);
        } else {
            throw DataReaderException(SourceContext{}, "Could not find a data reader for file: {}",
                                      file_.get());
        }
    } catch (const DataReaderException& e) {
        log::exception(e);
        sequence_.reset();
        loadingFailed_ = true;
        isReady_.update();
    }

    if (sequence_ && !sequence_->empty() && (*sequence_)[0]) {
        basis_.updateForNewEntity(*(*sequence_)[0], deserialize);
        const auto overwrite = deserialized_ ? util::OverwriteState::Yes : util::OverwriteState::No;
        Conf::updateForNew(information_, *(*sequence_)[0], overwrite);
    }
}

template <typename Conf>
void SequenceSource<Conf>::loadAndAddToSequence(const std::filesystem::path& file,
                                                Sequence& sequence) {

    if (auto reader1 = rf_->getReaderForTypeAndExtension<Type>(file)) {
        auto data1 = reader1->readData(file, this);
        data1->template setMetaData<StringMetaData>(fileMetaData, file.generic_string());
        sequence_->push_back(data1);
    } else if (auto reader2 = rf_->getReaderForTypeAndExtension<Sequence>(file)) {
        auto tempSequence = reader2->readData(file, this);
        for (auto&& data2 : *tempSequence) {
            data2->template setMetaData<StringMetaData>(fileMetaData, file.generic_string());
            sequence_->push_back(data2);
        }
    } else {
        throw DataReaderException(SourceContext{}, "Could not find a data reader for file: {}",
                                  file);
    }
}

template <typename Conf>
void SequenceSource<Conf>::loadFolder(bool deserialize) {
    if (folder_.get().empty()) return;

    sequence_ = std::make_shared<Sequence>();

    const auto files = filesystem::getDirectoryContents(folder_.get());
    for (const auto& f : files) {
        auto file = folder_.get() / f;
        if (filesystem::wildcardStringMatch(filter_, file.generic_string())) {
            try {
                loadAndAddToSequence(file, *sequence_);
            } catch (const DataReaderException& e) {
                log::exception(e);
                sequence_.reset();
                loadingFailed_ = true;
                isReady_.update();
                break;
            }
        }
    }

    if (sequence_ && !sequence_->empty()) {
        // store filename in metadata
        for (auto data : *sequence_) {
            if (!data->template hasMetaData<StringMetaData>(fileMetaData)) {
                data->template setMetaData<StringMetaData>(fileMetaData,
                                                           file_.get().generic_string());
            }
        }

        // set basis of first data
        if ((*sequence_)[0]) {
            basis_.updateForNewEntity(*(*sequence_)[0], deserialize);
            const auto overwrite =
                deserialized_ ? util::OverwriteState::Yes : util::OverwriteState::No;
            Conf::updateForNew(information_, *(*sequence_)[0], overwrite);
        }
    } else {
        outport_.detachData();
    }
}

template <typename Conf>
void SequenceSource<Conf>::process() {
    if (file_.isModified() || reload_.isModified() || folder_.isModified() ||
        filter_.isModified() || reader_.isModified()) {
        load(deserialized_);
        deserialized_ = false;
    }

    if (sequence_ && !sequence_->empty()) {
        outport_.setData(sequence_);
    }
}

template <typename Conf>
void SequenceSource<Conf>::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    util::updateFilenameFilters<Sequence>(*rf_, file_, reader_);
    // It does not make sense to change these for an entire sequence
    information_.setReadOnly(true);
    basis_.setReadOnly(true);
    deserialized_ = true;
}

}  // namespace inviwo
