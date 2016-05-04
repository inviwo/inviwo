/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

namespace inviwo {

template <typename DataType, typename PortType>
class DataSource : public Processor {
public:
    DataSource();
    virtual ~DataSource() = default;

    virtual bool isReady() const override;
    virtual void process() override;
    virtual void deserialize(Deserializer& d) override;

protected:
    void load(bool deserialized);
    virtual bool isSink() const override;

    // Called when we load new data.
    virtual void dataLoaded(std::shared_ptr<DataType> data){};
    // Called when we deserialized old data.
    virtual void dataDeserialized(std::shared_ptr<DataType> data){};

    PortType port_;
    FileProperty file_;
    ButtonProperty reload_;
    std::shared_ptr<DataType> loadedData_;

private:
    bool deserialized_ = false;
};

template <typename DataType, typename PortType>
DataSource<DataType, PortType>::DataSource()
    : Processor()
    , port_("data")
    , file_("filename", "File")
    , reload_("reload", "Reload data") {

    addPort(port_);

    addProperty(file_);
    addProperty(reload_);

    auto rf = InviwoApplication::getPtr()->getDataReaderFactory();
    file_.clearNameFilters();
    file_.addNameFilter(FileExtension("*", "All Files"));
    file_.addNameFilters(rf->template getExtensionsForType<DataType>());
}

template <typename DataType, typename PortType>
void inviwo::DataSource<DataType, PortType>::process() {
    if (file_.isModified() || reload_.isModified()) {
        load(deserialized_);
        deserialized_ = false;
    }
}

template <typename DataType, typename PortType>
bool inviwo::DataSource<DataType, PortType>::isSink() const {
    return true;
}

template <typename DataType, typename PortType>
bool DataSource<DataType, PortType>::isReady() const {
    return filesystem::fileExists(file_.get());
}

template <typename DataType, typename PortType>
void DataSource<DataType, PortType>::load(bool deserialized) {
    if (file_.get().empty() ) return;
   
    std::string ext = filesystem::getFileExtension(file_.get());
    auto rf = InviwoApplication::getPtr()->getDataReaderFactory();
    if (auto reader = rf->template getReaderForTypeAndExtension<DataType>(ext)) {
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
            LogProcessorError("Could not load data: " << file_.get() << ", " << e.getMessage());
        }
    } else {
        LogProcessorError("Could not find a data reader for file: " << file_.get());
    }
}

template <typename DataType, typename PortType>
void inviwo::DataSource<DataType, PortType>::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    auto rf = InviwoApplication::getPtr()->getDataReaderFactory();
    file_.clearNameFilters();
    file_.addNameFilter(FileExtension("*", "All Files"));
    file_.addNameFilters(rf->template getExtensionsForType<DataType>());
    deserialized_ = true;
}

}  // namespace

#endif  // IVW_DATASOURCE_H
