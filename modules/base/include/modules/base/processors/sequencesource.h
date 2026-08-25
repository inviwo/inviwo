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
#include <inviwo/core/properties/ordinalproperty.h>
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
#include <inviwo/core/util/utfutils.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <regex>
#include <expected>

#include <fmt/format.h>

namespace inviwo {
class DataReaderFactory;
class Deserializer;
class InviwoApplication;

/**
 * @brief Loads a sequence of data
 *
 * Conf struct example
 * @code{.cpp}
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
 *     static auto getReaderConfig(Info& info) -> std::function<void(DataReader&)> {
 *         return [](DataReader&) {};
 * };
 * @endcode
 */
template <typename Conf>
class SequenceSource : public PoolProcessor {
    enum class InputType : std::uint8_t { SingleFile, Folder };

public:
    using Type = Conf::Type;
    using Sequence = Conf::Sequence;
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
                             DataReaderFactory& rf, MetaDataOwner* md,
                             const std::function<void(DataReader&)>& configReader) -> Sequence;
    static void addMetaData(Type& data, const std::filesystem::path& path);

    DataReaderFactory* rf_;

    typename Conf::Outport outport_;

    OptionProperty<InputType> inputType_;
    FileProperty file_;
    DirectoryProperty folder_;
    StringProperty include_;
    StringProperty exclude_;
    OrdinalProperty<size_t> max_;

    OptionProperty<FileExtension> reader_;
    ButtonProperty reload_;

    typename Conf::Info information_;

    bool deserialized_ = false;
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

IVW_MODULE_BASE_API std::expected<std::vector<std::filesystem::path>, std::string_view>
getFilesInFolder(const std::filesystem::path& folder, std::string_view include,
                 std::string_view exclude);

IVW_MODULE_BASE_API std::expected<std::filesystem::path, std::string_view> getFirstFileInFolder(
    const std::filesystem::path& folder, std::string_view include, std::string_view exclude);

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

    , include_{"include", "include",
               R"(Any path that matches this regular expression will be included.
               If the string is empty, all paths will be included)"_unindentHelp,
               ""}
    , exclude_{"exclude", "exclude",
               R"(Any path that matches this regular expression will be excluded.
               If the string is empty, all paths will be included)"_unindentHelp,
               ""}
    , max_{"max", "max", util::ordinalCount(0uz)}

    , reader_("reader", "Data Reader")
    , reload_("reload", "Reload data")
    , information_{} {

    file_.setContentType(toLower(Conf::name));
    folder_.setContentType(toLower(Conf::name));

    addPort(outport_);
    addProperties(inputType_, folder_, include_, exclude_, max_, file_, reload_);

    Conf::add(information_, *this);

    util::updateFilenameFilters<Sequence>(*rf_, file_, reader_);
    util::updateReaderFromFile(file_, reader_);

    auto singleFileCallback = [](auto& p) { return p.get() == InputType::SingleFile; };
    auto folderCallback = [](auto& p) { return p.get() == InputType::Folder; };

    file_.visibilityDependsOn(inputType_, singleFileCallback);
    reader_.visibilityDependsOn(inputType_, singleFileCallback);
    folder_.visibilityDependsOn(inputType_, folderCallback);
    include_.visibilityDependsOn(inputType_, folderCallback);
    exclude_.visibilityDependsOn(inputType_, folderCallback);
    max_.visibilityDependsOn(inputType_, folderCallback);

    // make sure that we always process even if not connected
    isSink_.setUpdate([]() { return true; });
    isReady_.setUpdate([this]() -> ProcessorStatus {
        static constexpr std::string_view noFileReason{"No valid input file"};
        if (error()) {
            return {ProcessorStatus::Error, error().value()};
        }
        if (inputType_ == InputType::SingleFile) {
            if (std::filesystem::is_regular_file(file_.get()) &&
                !reader_.getSelectedValue().empty()) {
                return ProcessorStatus::Ready;
            } else {
                return {ProcessorStatus::NotReady, noFileReason};
            }
        } else {
            if (auto first =
                    util::getFirstFileInFolder(folder_.get(), include_.get(), exclude_.get())) {
                return ProcessorStatus::Ready;
            } else {
                return {ProcessorStatus::Error, first.error()};
            }
        }
    });

    file_.onChange([this]() { util::updateReaderFromFile(file_, reader_); });
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
                                        DataReaderFactory& rf, MetaDataOwner* mdo,
                                        const std::function<void(DataReader&)>& configReader)
    -> Sequence {

    if (auto reader1 = rf.getReaderForTypeAndExtension<Type>(ext, file)) {
        configReader(*reader1);
        auto data = reader1->readData(file, mdo);
        addMetaData(*data, file);
        Sequence sequence;
        sequence.push_back(data);
        return sequence;
    } else if (auto reader2 = rf.getReaderForTypeAndExtension<Sequence>(ext, file)) {
        configReader(*reader2);
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

    auto loader = [rf = rf_, ext = reader_.getSelectedValue(), file = file_.get(),
                   mdo = static_cast<MetaDataOwner*>(this),
                   configReader = Conf::getReaderConfig(information_)](
                      pool::Stop stop, pool::Progress progress) -> Sequence {
        if (stop) return {};
        progress(0.0);
        util::OnScopeExit done{[&]() { progress(1.0); }};
        return loadSequence(file, ext, *rf, mdo, configReader);
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

    const auto files = util::getFilesInFolder(folder_.get(), include_.get(), exclude_.get());
    if (!files) {
        throw Exception{files.error(), SourceContext{}};
    }

    const auto max = max_.get() != 0 ? max_.get() : std::numeric_limits<size_t>::max();

    const auto loaders =
        files.value() | std::views::take(max) |
        std::views::transform([&](const std::filesystem::path& path)
                                  -> std::function<Sequence(pool::Stop, pool::Progress)> {
            return [rf = rf_, path, mdo = static_cast<MetaDataOwner*>(this),
                    configReader = Conf::getReaderConfig(information_)](
                       pool::Stop stop, pool::Progress progress) -> Sequence {
                if (stop) return {};
                progress(0.0);
                util::OnScopeExit done{[&]() { progress(1.0); }};
                return loadSequence(path, FileExtension{}, *rf, mdo, configReader);
            };
        }) |
        std::ranges::to<std::vector>();

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
        include_.isModified() || exclude_.isModified() || max_.isModified() ||
        reader_.isModified()) {
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
