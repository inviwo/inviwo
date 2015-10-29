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
    virtual ~DataSource();

    virtual bool isReady() const;
    void load();
    
protected:
    void load(bool deserialized);
    bool isDeserializing() const;

    // Called when we load new data.
    virtual void dataLoaded(std::shared_ptr<DataType> data) {};
    // Called when we deserialized old data.
    virtual void dataDeserialized(std::shared_ptr<DataType> data) {};

    virtual void invalidateOutput();

    virtual void deserialize(IvwDeserializer& d);

    PortType port_;
    FileProperty file_;
    ButtonProperty reload_;
    std::shared_ptr<DataType> loadedData_;

private:
    bool isDeserializing_;
};


template <typename DataType, typename PortType>
DataSource<DataType, PortType>::DataSource()
    : Processor()
    , port_("data")
    , file_("filename", "File")
    , reload_("reload", "Reload data")
    , loadedData_()
    , isDeserializing_(false) {
    
    addPort(port_);
    file_.onChange(this, &DataSource::load);

    auto extensions = DataReaderFactory::getPtr()->getExtensionsForType<DataType>();
    for (auto& ext : extensions) {
        file_.addNameFilter(ext.description_ + " (*." + ext.extension_ + ")");
    }

    reload_.onChange(this, &DataSource::load);

    addProperty(file_);
    addProperty(reload_);
}

template <typename DataType, typename PortType>
DataSource<DataType, PortType>::~DataSource() {}

template <typename DataType, typename PortType>
bool DataSource<DataType, PortType>::isDeserializing() const {
    return isDeserializing_;
}

template <typename DataType, typename PortType>
void DataSource<DataType, PortType>::invalidateOutput() {
    invalidate(InvalidationLevel::InvalidOutput);
}

template <typename DataType, typename PortType>
bool DataSource<DataType, PortType>::isReady() const {
    return filesystem::fileExists(file_.get());
}

template <typename DataType, typename PortType>
void DataSource<DataType, PortType>::load() {
    load(false);
}

/**
 * load is called when the filename changes, and after the deserialization
 */
template <typename DataType, typename PortType>
void DataSource<DataType, PortType>::load(bool deserialized) {
    if (isDeserializing_ || file_.get() == "") {
        return;
    }

    std::string ext = filesystem::getFileExtension(file_.get());  
    if (auto reader = DataReaderFactory::getPtr()->getReaderForTypeAndExtension<DataType>(ext)) {
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

/**
 * Deserialize everything first then load the data
 */
template <typename DataType, typename PortType>
void inviwo::DataSource<DataType, PortType>::deserialize(IvwDeserializer& d) {
    isDeserializing_ = true;
    Processor::deserialize(d);
    auto extensions = DataReaderFactory::getPtr()->getExtensionsForType<DataType>();
    file_.clearNameFilters();
    file_.addNameFilter(FileExtension("*", "All Files"));
    for (auto& ext : extensions) {
        file_.addNameFilter(ext.description_ + " (*." + ext.extension_ + ")");
    }
    isDeserializing_ = false;
    load(true);
}

}  // namespace

#endif  // IVW_DATASOURCE_H
