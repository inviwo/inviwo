/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_DATASOURCE_H
#define IVW_DATASOURCE_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwo.h>
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
inline void updateReaderFromFile(const FileProperty& file,
                                 TemplateOptionProperty<FileExtension>& reader) {
    if ((file.getSelectedExtension() == FileExtension::all() &&
         !reader.getSelectedValue().matches(file)) ||
        file.getSelectedExtension().empty()) {
        const auto& opts = reader.getOptions();
        const auto it = std::find_if(opts.begin(), opts.end(),
                                     [&](const OptionPropertyOption<FileExtension>& opt) {
                                         return opt.value_.matches(file.get());
                                     });
        reader.setSelectedValue(it != opts.end() ? it->value_ : FileExtension{});
    } else {
        reader.setSelectedValue(file.getSelectedExtension());
    }
}

template <typename... Types>
void updateFilenameFilters(const DataReaderFactory& rf, FileProperty& file,
                           TemplateOptionProperty<FileExtension>& reader) {
    std::vector<FileExtension> extensions;

    util::append(extensions, rf.getExtensionsForType<Types>()...);

    std::sort(extensions.begin(), extensions.end());

    std::vector<OptionPropertyOption<FileExtension>> options;
    std::transform(extensions.begin(), extensions.end(), std::back_inserter(options), [](auto& fe) {
        return OptionPropertyOption<FileExtension>{fe.toString(), fe.toString(), fe};
    });

    options.emplace_back("noreader", "No available reader", FileExtension{});

    file.clearNameFilters();
    file.addNameFilter(FileExtension::all());
    file.addNameFilters(extensions);
    reader.replaceOptions(options);
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
     * @param app An InviwoApplication.
     * @param file A filename passed into the FileProperty
     * @param content A content type passed into the FileProperty, usually 'volume', 'image' etc.
     * @see FileProperty
     */
    DataSource(InviwoApplication* app = InviwoApplication::getPtr(), const std::string& file = "",
               const std::string& content = "");
    virtual ~DataSource() = default;

    virtual void process() override;
    virtual void deserialize(Deserializer& d) override;

protected:
    void load(bool deserialized);

    // Called when we load new data.
    virtual void dataLoaded(std::shared_ptr<DataType> data){};
    // Called when we deserialized old data.
    virtual void dataDeserialized(std::shared_ptr<DataType> data){};

    DataReaderFactory* rf_;
    PortType port_;
    FileProperty file_;
    TemplateOptionProperty<FileExtension> reader_;
    ButtonProperty reload_;
    std::shared_ptr<DataType> loadedData_;
    bool loadingFailed_ = false;

private:
    bool deserialized_ = false;
};

template <typename DataType, typename PortType>
DataSource<DataType, PortType>::DataSource(InviwoApplication* app, const std::string& file,
                                           const std::string& content)
    : Processor()
    , rf_(app->getDataReaderFactory())
    , port_("data")
    , file_("filename", "File", file, content)
    , reader_("reader", "Data Reader")
    , reload_("reload", "Reload data") {

    addPort(port_);
    addProperties(file_, reader_, reload_);

    util::updateFilenameFilters<DataType>(*rf_, file_, reader_);
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

template <typename DataType, typename PortType>
void DataSource<DataType, PortType>::process() {
    if (file_.isModified() || reload_.isModified()) {
        load(deserialized_);
        deserialized_ = false;
    }
}

template <typename DataType, typename PortType>
void DataSource<DataType, PortType>::load(bool deserialized) {
    if (file_.get().empty()) return;

    const auto sext = reader_.getSelectedValue();
    const auto fext = filesystem::getFileExtension(file_.get());
    if (auto reader = rf_->template getReaderForTypeAndExtension<DataType>(sext, fext)) {
        try {
            auto data = reader->readData(file_.get());
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
            LogProcessorError("Could not load data: " << file_.get() << ", " << e.getMessage());
        }
    } else {
        loadingFailed_ = true;
        port_.detachData();
        isReady_.update();
        LogProcessorError("Could not find a data reader for file: " << file_.get());
    }
}

template <typename DataType, typename PortType>
void DataSource<DataType, PortType>::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    util::updateFilenameFilters<DataType>(*rf_, file_, reader_);
    deserialized_ = true;
}

}  // namespace inviwo

#endif  // IVW_DATASOURCE_H
