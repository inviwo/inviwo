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
#include <inviwo/core/util/fileextension.h>

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
template <typename DataType, typename PortType>
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
    void load(bool deserialized);

    // Called when we load new data.
    virtual void dataLoaded(std::shared_ptr<DataType> data){};
    // Called when we deserialized old data.
    virtual void dataDeserialized(std::shared_ptr<DataType> data){};

    DataReaderFactory* rf_;
    PortType port_;
    std::shared_ptr<DataType> loadedData_;
    bool loadingFailed_ = false;

private:
    bool deserialized_ = false;
};

template <typename DataType, typename PortType>
DataSource<DataType, PortType>::DataSource(DataReaderFactory* rf,
                                           const std::filesystem::path& aFilePath,
                                           std::string_view content)
    : Processor()
    , filePath{"filename", "File", aFilePath, content}
    , extensions{"reader", "Data Reader"}
    , reload{"reload", "Reload data",
             [this]() {
                 loadingFailed_ = false;
                 isReady_.update();
             }}
    , rf_{rf}
    , port_{"data"} {

    addPort(port_);
    addProperties(filePath, extensions, reload);

    util::updateFilenameFilters<DataType>(*rf_, filePath, extensions);
    util::updateReaderFromFile(filePath, extensions);

    // make sure that we always process even if not connected
    isSink_.setUpdate([]() { return true; });
    isReady_.setUpdate([this]() {
        return !loadingFailed_ && std::filesystem::is_regular_file(filePath.get()) &&
               !extensions.getSelectedValue().empty();
    });
    filePath.onChange([this]() {
        loadingFailed_ = false;
        util::updateReaderFromFile(filePath, extensions);
        isReady_.update();
    });
    extensions.onChange([this]() {
        loadingFailed_ = false;
        isReady_.update();
    });
}

template <typename DataType, typename PortType>
void DataSource<DataType, PortType>::process() {
    if (filePath.isModified() || reload.isModified()) {
        load(deserialized_);
        deserialized_ = false;
    }
}

template <typename DataType, typename PortType>
void DataSource<DataType, PortType>::load(bool deserialized) {
    if (filePath.get().empty()) return;

    const auto sext = extensions.getSelectedValue();
    if (auto reader = rf_->template getReaderForTypeAndExtension<DataType>(sext, filePath.get())) {
        try {
            auto data = reader->readData(filePath.get());
            port_.setData(data);
            loadedData_ = data;
            if (deserialized) {
                dataDeserialized(data);
            } else {
                dataLoaded(data);
            }
        } catch (DataReaderException const& e) {
            loadingFailed_ = true;
            port_.detachData();
            isReady_.update();
            LogProcessorError("Could not load data: " << filePath.get() << ", " << e.getMessage());
        }
    } else {
        loadingFailed_ = true;
        port_.detachData();
        isReady_.update();
        LogProcessorError("Could not find a data reader for file: " << filePath.get());
    }
}

template <typename DataType, typename PortType>
void DataSource<DataType, PortType>::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    util::updateFilenameFilters<DataType>(*rf_, filePath, extensions);
    deserialized_ = true;
}

}  // namespace inviwo
