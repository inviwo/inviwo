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
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/network/processornetworkevaluationobserver.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/processornetworkevaluator.h>

#include <fstream>
#include <ranges>

#include <filesystem>

namespace inviwo {

template <typename DataType, typename InportType = DataInport<DataType>,
          typename OutportType = DataOutport<DataType>>
class FileCache : public Processor, public ProcessorNetworkEvaluationObserver {
public:
    FileCache(InviwoApplication* app);

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual bool isConnectionActive(Inport* inport, Outport* outport) const override;

    virtual void onProcessorNetworkEvaluationBegin() override;

private:
    InportType inport_;
    OutportType outport_;
    DirectoryProperty cacheDir_;
    OptionProperty<FileExtension> extensions_;

    const DataReaderFactory& rf;
    const DataWriterFactory& wf;
    struct Cache {
        std::string key;
        std::filesystem::path file;
    };
    std::pmr::string xml;
    std::optional<Cache> cache_ = std::nullopt;
    bool isCached_ = false;
    std::string loadedKey_;
};

namespace detail {

IVW_MODULE_BASE_API std::string cacheState(Processor* p, ProcessorNetwork& net,
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
    : Processor{}
    , inport_{"inport", "data to cache"_help}
    , outport_{"outport", "cached data"_help}

    , cacheDir_{"cacheDir", "Cache Dir"}
    , extensions_{"readerWriter", "Data Reader And Writer"}
    , rf{*app->getDataReaderFactory()}
    , wf{*app->getDataWriterFactory()} {

    addPorts(inport_, outport_);
    addProperties(cacheDir_, extensions_);

    detail::updateFilenameFilters<DataType>(rf, wf, extensions_);
    extensions_.setCurrentStateAsDefault();

    isReady_.setUpdate([this]() {
        return isCached_ ||
               (inport_.isConnected() && util::all_of(inport_.getConnectedOutports(),
                                                      [](Outport* p) { return p->isReady(); }));
    });

    app->getProcessorNetworkEvaluator()->addObserver(this);
}

template <typename DataType, typename InportType, typename OutportType>
void FileCache<DataType, InportType, OutportType>::onProcessorNetworkEvaluationBegin() {
    if (extensions_.empty()) return;

    const auto key = detail::cacheState(this, *getNetwork(), xml);

    const auto cf =
        cacheDir_.get() / fmt::format("{}.{}", key, extensions_.getSelectedValue().extension_);
    const auto isCached = std::filesystem::exists(cf);

    if (isCached_ != isCached) {
        isCached_ = isCached;
        isReady_.update();
        notifyObserversActiveConnectionsChange(this);
    }
    cache_ = Cache{std::move(key), std::move(cf)};
}

template <typename DataType, typename InportType, typename OutportType>
bool FileCache<DataType, InportType, OutportType>::isConnectionActive(Inport* inport,
                                                                      Outport*) const {
    if (inport == &inport_) {
        return !isCached_;
    } else {
        return true;
    }
}

template <typename DataType, typename InportType, typename OutportType>
void FileCache<DataType, InportType, OutportType>::process() {
    if (extensions_.empty()) {
        throw Exception("No reader and writer found");
    }
    if (cacheDir_.get().empty()) {
        throw Exception("No cache dir set");
    }

    const auto sext = extensions_.getSelectedValue();

    if (cache_ && loadedKey_ == cache_->key) {
        return;
    } else if (cache_ && isCached_) {
        if (auto reader = rf.template getReaderForTypeAndExtension<DataType>(sext, cache_->file)) {
            outport_.setData(reader->readData(cache_->file));
            loadedKey_ = cache_->key;
        } else {
            throw Exception("No reader found");
        }
    } else if (cache_ && !isCached_) {
        if (auto data = inport_.getData()) {

            if (auto writer =
                    wf.template getWriterForTypeAndExtension<DataType>(sext, cache_->file)) {
                writer->writeData(data.get(), cache_->file);
            } else {
                throw Exception("No writer found");
            }
            if (auto f = std::ofstream(cacheDir_.get() / fmt::format("{}.inv", cache_->key))) {
                f << xml;
            } else {
                throw Exception(SourceContext{}, "Could not write to xml file: {}/{}.inv",
                                cacheDir_.get(), cache_->key);
            }

            outport_.setData(data);
            loadedKey_ = cache_->key;
        } else {
            throw Exception("Port had no data");
        }
    } else {
        outport_.setData(inport_.getData());
    }
}

}  // namespace inviwo
