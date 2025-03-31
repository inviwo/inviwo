/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/network/processornetworkevaluationobserver.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/transparentmaps.h>

#include <fstream>
#include <ranges>

#include <filesystem>
#include <optional>

#include <fmt/std.h>

namespace inviwo {

namespace detail {

IVW_MODULE_BASE_API std::string cacheState(Processor* p, ProcessorNetwork& net,
                                           const std::filesystem::path& refPath,
                                           std::pmr::string& xml);

template <typename... Types>
void updateFilenameFilters(const DataReaderFactory& rf, const DataWriterFactory& wf,
                           OptionProperty<FileExtension>& extensions) {

    auto readExts = rf.getExtensionsForTypesView<Types...>();
    auto writeExts = wf.getExtensionsForTypesView<Types...>();

    std::vector<FileExtension> intersection;
    std::ranges::set_intersection(readExts, writeExts, std::back_inserter(intersection));

    if (std::ranges::equal(extensions.getOptions(), intersection, std::ranges::equal_to{},
                           &OptionPropertyOption<FileExtension>::value_)) {
        return;
    }

    extensions.updateOptions([&](std::vector<OptionPropertyOption<FileExtension>>& opts) -> bool {
        bool modified = false;
        for (std::pair<OptionPropertyOption<FileExtension>&, const FileExtension&> item :
             std::views::zip(opts, intersection)) {
            auto&& [opt, ext] = item;
            if (opt.value_ != ext) {
                opt = ext;
                modified = true;
            }
        }
        const auto size = std::ranges::distance(intersection);
        if (std::ssize(opts) > size) {
            opts.erase(opts.begin() + size, opts.end());
            modified = true;
        }
        for (auto&& item : intersection | std::views::drop(opts.size())) {
            opts.emplace_back(item);
            modified = true;
        }
        return modified;
    });
}

}  // namespace detail

class IVW_MODULE_BASE_API CacheBase : public Processor, public ProcessorNetworkEvaluationObserver {
public:
    CacheBase(InviwoApplication* app);
    virtual bool isConnectionActive(Inport* inport, Outport* outport) const override;
    virtual void onProcessorNetworkEvaluationBegin() override;

    struct Cache {
        std::string key;
    };

    virtual bool hasCache(std::string_view key) = 0;

protected:
    void writeXML() const;

    BoolProperty enabled_;
    DirectoryProperty cacheDir_;
    DirectoryProperty refDir_;
    StringProperty currentKey_;

    bool isCached_ = false;
    std::string key_;
    std::pmr::string xml_;
};

template <typename DataType>
struct ReaderWriter {
    ReaderWriter(InviwoApplication* app)
        : extensions{"readerWriter", "Data Reader And Writer"}
        , rf{*app->getDataReaderFactory()}
        , wf{*app->getDataWriterFactory()} {

        detail::updateFilenameFilters<DataType>(rf, wf, extensions);
        extensions.setCurrentStateAsDefault();
    }

    std::unique_ptr<DataReaderType<DataType>> getReader() {
        if (extensions.empty()) return nullptr;

        const auto& sext = extensions.getSelectedValue().extension_;
        return rf.template getReaderForTypeAndExtension<DataType>(sext);
    }
    std::unique_ptr<DataWriterType<DataType>> getWriter() {
        if (extensions.empty()) return nullptr;

        const auto& sext = extensions.getSelectedValue().extension_;
        return wf.template getWriterForTypeAndExtension<DataType>(sext);
    }

    OptionProperty<FileExtension> extensions;
    const DataReaderFactory& rf;
    const DataWriterFactory& wf;
};

template <typename DataType>
struct RamCache {
    RamCache()
        : size{"size", "Max items to cache in RAM", util::ordinalCount(size_t{0}, size_t{100})} {}

    struct Item {
        std::chrono::system_clock::time_point used;
        DataType data;
    };

    IntSizeTProperty size;
    UnorderedStringMap<Item> cache;

    void trim(size_t maxSize) {
        while (cache.size() > maxSize) {
            const auto it = std::ranges::min_element(
                cache, std::ranges::less{}, [](const auto& pair) { return pair.second.used; });
            cache.erase(it);
        }
    }

    void add(std::string_view key, DataType data) {
        if (size.get() == 0) return;

        trim(size.get() - 1);

        const auto now = std::chrono::system_clock::now();
        cache.try_emplace(std::string{key}, Item{now, std::move(data)});
    }

    bool has(std::string_view key) const { return cache.contains(key); }

    std::optional<DataType> get(std::string_view key) {
        if (auto it = cache.find(key); it != cache.end()) {
            it->second.used = std::chrono::system_clock::now();
            return it->second.data;
        } else {
            return std::nullopt;
        }
    }
};

template <typename DataType, typename InportType = DataInport<DataType>,
          typename OutportType = DataOutport<DataType>>
class FileCache : public CacheBase {
public:
    explicit FileCache(InviwoApplication* app);
    FileCache(const FileCache&) = delete;
    FileCache(FileCache&&) = delete;
    FileCache& operator=(const FileCache&) = delete;
    FileCache& operator=(FileCache&&) = delete;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    std::optional<std::filesystem::path> pathForKey(std::string_view key) {
        if (rw_.extensions.empty() || cacheDir_.get().empty()) return std::nullopt;
        return cacheDir_.get() /
               fmt::format("{}.{}", key, rw_.extensions.getSelectedValue().extension_);
    }

    virtual bool hasCache(std::string_view key) override {
        return ram_.has(key) || pathForKey(key)
                                    .transform([](const std::filesystem::path& path) -> bool {
                                        return std::filesystem::exists(path);
                                    })
                                    .value_or(false);
    }

    InportType inport_;
    OutportType outport_;
    ReaderWriter<DataType> rw_;
    RamCache<std::shared_ptr<const DataType>> ram_;

    std::string loadedKey_;
};

template <typename DataType, typename InportType, typename OutportType>
struct ProcessorTraits<FileCache<DataType, InportType, OutportType>> {
    static ProcessorInfo getProcessorInfo() {
        return {
            fmt::format("{}.FileCache",
                        DataTraits<DataType>::classIdentifier()),            // Class identifier
            fmt::format("{} File Cache", DataTraits<DataType>::dataName()),  // Display name
            "Cache",                                                         // Category
            CodeState::Stable,                                               // Code state
            Tags::CPU,                                                       // Tags
            R"(Provides a file cache for data)"_unindentHelp,
        };
    }
};

template <typename DataType, typename InportType, typename OutportType>
const ProcessorInfo& FileCache<DataType, InportType, OutportType>::getProcessorInfo() const {
    static const ProcessorInfo info =
        ProcessorTraits<FileCache<DataType, InportType, OutportType>>::getProcessorInfo();
    return info;
}

template <typename DataType, typename InportType, typename OutportType>
FileCache<DataType, InportType, OutportType>::FileCache(InviwoApplication* app)
    : CacheBase{app}
    , inport_{"inport", "data to cache"_help}
    , outport_{"outport", "cached data"_help}
    , rw_{app}
    , ram_{} {

    addPorts(inport_, outport_);
    addProperties(enabled_, cacheDir_, refDir_, currentKey_, rw_.extensions, ram_.size);
}

template <typename DataType, typename InportType, typename OutportType>
void FileCache<DataType, InportType, OutportType>::process() {
    if (loadedKey_ == key_) return;

    if (isCached_) {
        if (auto ramData = ram_.get(key_)) {
            outport_.setData(*ramData);
            loadedKey_ = key_;
        } else if (auto reader = rw_.getReader()) {
            if (auto maybePath = pathForKey(key_)) {
                auto diskData = reader->readData(*maybePath);
                ram_.add(key_, diskData);
                outport_.setData(diskData);
                loadedKey_ = key_;
            } else {
                throw Exception("No file found");
            }
        } else {
            throw Exception("No reader found");
        }
    } else if (auto data = inport_.getData()) {
        if (auto maybePath = pathForKey(key_)) {
            if (auto writer = rw_.getWriter()) {
                writer->writeData(data.get(), *maybePath);
            } else {
                throw Exception("No writer found");
            }
        }
        writeXML();
        ram_.add(key_, data);
        outport_.setData(data);
        loadedKey_ = key_;
    } else {
        throw Exception("Port had no data");
    }
}

}  // namespace inviwo
