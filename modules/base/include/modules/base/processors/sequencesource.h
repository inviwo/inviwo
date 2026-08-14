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

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/processors/poolprocessor.h>
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
#include <inviwo/core/util/fileextensionutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/statecoordinator.h>
#include <inviwo/core/util/staticstring.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/raiiutils.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>

namespace inviwo {
class DataReaderFactory;
class Deserializer;
class InviwoApplication;

/**
 * @brief Loads a sequence of data
 *
 * Conf struct example
 * struct VolumeConf {
 *     using Type = Volume;
 *     using Sequence = DataSequence<Type>;
 *     using Outport = VolumeSequenceOutport;
 *     static constexpr auto name = DataTraits<Type>::dataName();
 *     static constexpr auto plural = "s";
 *     static constexpr size_t dim = 3;
 *
 *     struct Info {
 *         BasisProperty basis{"Basis", "Basis and offset"};
 *         VolumeInformationProperty info{"Information", "Data information"};
 *     };
 *     static void add(Info& info, auto& processor) {
 *         processor.addProperties(info.basis, info.info);
 *         info.basis.setReadOnly(true);
 *         info.info.setReadOnly(true);
 *     }
 *     static void updateForNew(Info& info, const Type& data, util::OverwriteState overwrite) {
 *         info.info.updateForNewVolume(data, overwrite);
 *         info.basis.updateForNewEntity(data, overwrite == util::OverwriteState::Yes);
 *     }
 * };
 */
template <typename Conf>
class SequenceSource : public PoolProcessor {
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
    static auto loadSequence(const std::filesystem::path& file, const FileExtension& ext,
                             DataReaderFactory& rf, MetaDataOwner* md) -> Sequence;
    static void addMetaData(Type& data, const std::filesystem::path& path);

    DataReaderFactory* rf_;

    typename Conf::Outport outport_;

    OptionProperty<InputType> inputType_;
    FileProperty file_;
    DirectoryProperty folder_;
    StringProperty filter_;

    OptionProperty<FileExtension> reader_;
    ButtonProperty reload_;

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
    : PoolProcessor()
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
    , information_{} {

    file_.setContentType(toLower(Conf::name));
    folder_.setContentType(toLower(Conf::name));

    addPort(outport_);
    addProperties(inputType_, folder_, filter_, file_, reload_);

    Conf::add(information_, *this);

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
            [[fallthrough]];
        default:
            loadFile(deserialize);
            break;
    }
}

template <typename Conf>
auto SequenceSource<Conf>::loadSequence(const std::filesystem::path& file, const FileExtension& ext,
                                        DataReaderFactory& rf, MetaDataOwner* mdo) -> Sequence {

    if (auto reader1 = rf.getReaderForTypeAndExtension<Type>(ext, file)) {
        auto data = reader1->readData(file, mdo);
        addMetaData(*data, file);
        Sequence sequence;
        sequence.push_back(data);
        return sequence;
    } else if (auto reader2 = rf.getReaderForTypeAndExtension<Sequence>(ext, file)) {
        auto sequence = reader2->readData(file, mdo);
        for (auto&& data : *sequence) {
            addMetaData(*data, file);
        }
        return *std::move(sequence);
    } else {
        throw DataReaderException(SourceContext{}, "Could not find a data reader for file: {}",
                                  file);
    }
}

template <typename Conf>
void SequenceSource<Conf>::loadFile(bool deserialize) {
    if (file_.get().empty()) return;

    auto loader = [rf = rf_, ext = file_.getSelectedExtension(), file = file_.get(),
                   mdo = static_cast<MetaDataOwner*>(this)](pool::Stop stop,
                                                            pool::Progress progress) -> Sequence {
        if (stop) return {};
        progress(0.0);
        util::OnScopeExit done{[&]() { progress(1.0); }};
        return loadSequence(file, ext, *rf, mdo);
    };
    dispatchOne(loader, [this, deserialize](Sequence sequence) {
        if (!sequence.empty() && sequence[0]) {
            const auto overwrite =
                deserialize ? util::OverwriteState::Yes : util::OverwriteState::No;
            Conf::updateForNew(information_, *sequence[0], overwrite);
        }
        outport_.setData(std::make_shared<Sequence>(std::move(sequence)));
        newResults();
    });
}

template <typename Conf>
void SequenceSource<Conf>::loadFolder(bool deserialize) {
    if (folder_.get().empty()) return;

    auto sequence = std::make_shared<Sequence>();

    std::vector<std::function<Sequence(pool::Stop, pool::Progress)>> loaders;

    const auto files = filesystem::getDirectoryContents(folder_.get());
    for (const auto& f : files) {
        auto file = folder_.get() / f;
        if (filesystem::wildcardStringMatch(filter_, file.generic_string())) {
            loaders.emplace_back([rf = rf_, file, mdo = static_cast<MetaDataOwner*>(this)](
                                     pool::Stop stop, pool::Progress progress) -> Sequence {
                if (stop) return {};
                progress(0.0);
                util::OnScopeExit done{[&]() { progress(1.0); }};
                return loadSequence(file, FileExtension{}, *rf, mdo);
            });
        }
    }

    dispatchMany(loaders, [this, deserialize](std::vector<Sequence> sequences) {
        auto result = std::make_shared<Sequence>(sequences | std::views::join);
        if (!result->empty() && (*result)[0]) {
            const auto overwrite =
                deserialize ? util::OverwriteState::Yes : util::OverwriteState::No;
            Conf::updateForNew(information_, *(*result)[0], overwrite);
        }
        outport_.setData(result);
        newResults();
    });
}

template <typename Conf>
void SequenceSource<Conf>::addMetaData(Type& data, const std::filesystem::path& path) {
    if constexpr (std::derived_from<Type, MetaDataOwner>) {
        if (!data.template hasMetaData<StringMetaData>(fileMetaData)) {
            data.template setMetaData<StringMetaData>(fileMetaData, path.generic_string());
        }
    }
}

template <typename Conf>
void SequenceSource<Conf>::process() {
    if (file_.isModified() || reload_.isModified() || folder_.isModified() ||
        filter_.isModified() || reader_.isModified()) {
        load(deserialized_);
        deserialized_ = false;
    }
}

template <typename Conf>
void SequenceSource<Conf>::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    util::updateFilenameFilters<Sequence>(*rf_, file_, reader_);
    deserialized_ = true;
}

}  // namespace inviwo
