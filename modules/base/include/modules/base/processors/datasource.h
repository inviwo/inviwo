/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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
#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/util/fileextension.h>

#include <fmt/std.h>

namespace inviwo {

namespace util {
inline void updateReaderFromFile(const FileProperty& filePath,
                                 OptionProperty<FileExtension>& extensions) {
    if ((filePath.getSelectedExtension() == FileExtension::all() &&
         !extensions.getSelectedValue().matches(filePath)) ||
        filePath.getSelectedExtension().empty()) {
        const auto& opts = extensions.getOptions();
        const auto it = std::find_if(opts.begin(), opts.end(),
                                     [&](const OptionPropertyOption<FileExtension>& opt) {
                                         return opt.value_.matches(filePath.get());
                                     });
        extensions.setSelectedValue(it != opts.end() ? it->value_ : FileExtension{});
    } else {
        extensions.setSelectedValue(filePath.getSelectedExtension());
    }
}

template <typename... Types>
void updateFilenameFilters(const DataReaderFactory& rf, FileProperty& filePath,
                           OptionProperty<FileExtension>& optionProperty) {
    std::vector<FileExtension> extensions;

    util::append(extensions, rf.getExtensionsForType<Types>()...);

    std::sort(extensions.begin(), extensions.end());

    std::vector<OptionPropertyOption<FileExtension>> options;
    std::transform(extensions.begin(), extensions.end(), std::back_inserter(options), [](auto& fe) {
        return OptionPropertyOption<FileExtension>{fe.toString(), fe.toString(), fe};
    });

    options.emplace_back("noreader", "No available reader", FileExtension{});

    filePath.clearNameFilters();
    filePath.addNameFilter(FileExtension::all());
    filePath.addNameFilters(extensions);
    optionProperty.replaceOptions(options);
}

}  // namespace util

/**
 * A base class for simple source processors.
 * Two functions to customize the behavior are available, dataLoaded and dataDeserialized.
 */
template <typename DataType, typename PortType = DataOutport<DataType>,
          typename ReaderType = DataType>
class DataSource : public Processor {
public:
    /**
     * Construct a DataSource
     * @param rf An DataReaderFactory.
     * @param filePath A file path passed into the FileProperty
     * @param contentType A content type passed into the FileProperty, usually 'volume', 'image',
     * 'geometry', etc.
     * @see FileProperty
     */
    DataSource(DataReaderFactory* rf = util::getDataReaderFactory(),
               const std::filesystem::path& filePath = {},
               std::string_view contentType = FileProperty::defaultContentType);
    virtual ~DataSource() = default;

    virtual void process() override;
    virtual void deserialize(Deserializer& d) override;

    FileProperty filePath;
    OptionProperty<FileExtension> extensions;
    ButtonProperty reload;

protected:
    void load();
    void handleError(std::string_view error);

    // Called to transform the loaded data of ReaderType to the DataType expected by the port
    virtual std::shared_ptr<DataType> transform(std::shared_ptr<ReaderType> data);

    // Called when we load new data.
    virtual void dataLoaded(std::shared_ptr<DataType> data){};
    // Called when we deserialized old data.
    virtual void dataDeserialized(std::shared_ptr<DataType> data){};

    DataReaderFactory* rf_;
    PortType port_;
    std::string error_;
    bool deserialized_ = false;
};

template <typename DataType, typename PortType, typename ReaderType>
DataSource<DataType, PortType, ReaderType>::DataSource(DataReaderFactory* rf,
                                                       const std::filesystem::path& aFilePath,
                                                       std::string_view content)
    : Processor()
    , filePath{"filename", "File", aFilePath, content}
    , extensions{"reader", "Data Reader"}
    , reload{"reload", "Reload data",
             [this]() {
                 error_.clear();
                 isReady_.update();
             }}
    , rf_{rf}
    , port_{"data"} {

    addPort(port_);
    addProperties(filePath, extensions, reload);

    util::updateFilenameFilters<ReaderType>(*rf_, filePath, extensions);
    util::updateReaderFromFile(filePath, extensions);

    // make sure that we always process even if not connected
    isSink_.setUpdate([]() { return true; });
    isReady_.setUpdate([this]() -> ProcessorStatus {
        if (!error_.empty()) {
            return {ProcessorStatus::Error, error_};
        } else if (filePath.get().empty()) {
            static constexpr std::string_view reason{"File not set"};
            return {ProcessorStatus::NotReady, reason};
        } else if (!std::filesystem::is_regular_file(filePath.get())) {
            static constexpr std::string_view reason{"Invalid or missing file"};
            return {ProcessorStatus::Error, reason};
        } else if (extensions.getSelectedValue().empty()) {
            static constexpr std::string_view reason{"No reader found for file"};
            return {ProcessorStatus::Error, reason};
        } else {
            return ProcessorStatus::Ready;
        }
    });
    filePath.onChange([this]() {
        error_.clear();
        util::updateReaderFromFile(filePath, extensions);
        isReady_.update();
    });
    extensions.onChange([this]() {
        error_.clear();
        isReady_.update();
    });
}

template <typename DataType, typename PortType, typename ReaderType>
void DataSource<DataType, PortType, ReaderType>::process() {
    if (filePath.isModified() || reload.isModified()) {
        load();
        deserialized_ = false;
    }
}

template <typename DataType, typename PortType, typename ReaderType>
void DataSource<DataType, PortType, ReaderType>::handleError(std::string_view error) {
    error_ = error;
    port_.clear();
    isReady_.update();
    LogProcessorError(error_);
}

template <typename DataType, typename PortType, typename ReaderType>
std::shared_ptr<DataType> DataSource<DataType, PortType, ReaderType>::transform(
    std::shared_ptr<ReaderType> data) {

    if constexpr (std::is_same_v<DataType, ReaderType>) {
        return data;
    } else {
        throw Exception(
            IVW_CONTEXT,
            "Derived DataSource class needs to override DataSource::transform, to convert the "
            "'ReaderType' to the 'DataType'");
    }
}

template <typename DataType, typename PortType, typename ReaderType>
void DataSource<DataType, PortType, ReaderType>::load() {
    if (filePath.get().empty()) return;

    const auto sext = extensions.getSelectedValue();
    if (auto reader =
            rf_->template getReaderForTypeAndExtension<ReaderType>(sext, filePath.get())) {
        try {
            auto data = transform(reader->readData(filePath.get()));
            port_.setData(data);
            if (deserialized_) {
                dataDeserialized(data);
            } else {
                dataLoaded(data);
            }
        } catch (DataReaderException const& e) {
            handleError(fmt::format("Could not load: {}.\n{}", filePath.get(), e.getMessage()));
        }
    } else {
        handleError(fmt::format("Could not find a data reader for file: {}", filePath.get()));
    }
}

template <typename DataType, typename PortType, typename ReaderType>
void DataSource<DataType, PortType, ReaderType>::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    util::updateFilenameFilters<ReaderType>(*rf_, filePath, extensions);
    deserialized_ = true;
}

}  // namespace inviwo
