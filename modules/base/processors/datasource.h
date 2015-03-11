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
    virtual void dataLoaded(DataType* data) {};
    // Called when we deserialized old data.
    virtual void dataDeserialized(DataType* data) {};

    virtual void invalidateOutput();

    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);

    PortType port_;
    FileProperty file_;
    ButtonProperty reload_;
    DataType* loadedData_;

private:
    bool isDeserializing_;
};


template <typename DataType, typename PortType>
DataSource<DataType, PortType>::DataSource()
    : Processor()
    , port_("data")
    , file_("filename", "File")
    , reload_("reload", "Reload data")
    , loadedData_(nullptr)
    , isDeserializing_(false) {
    
    addPort(port_);
    file_.onChange(this, &DataSource::load);
    std::vector<FileExtension> ext = DataReaderFactory::getPtr()->getExtensionsForType<DataType>();

    for (std::vector<FileExtension>::const_iterator it = ext.begin(); it != ext.end(); ++it) {
        std::stringstream ss;
        ss << it->description_ << " (*." << it->extension_ << ")";
        file_.addNameFilter(ss.str());
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
    invalidate(INVALID_OUTPUT);
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
 * load is called when the filename changes, and after the deserialisation
 */
template <typename DataType, typename PortType>
void DataSource<DataType, PortType>::load(bool deserialized) {
    if (isDeserializing_ || file_.get() == "") {
        return;
    }

    std::string fileExtension = filesystem::getFileExtension(file_.get());
    DataReaderType<DataType>* reader =
        DataReaderFactory::getPtr()->getReaderForTypeAndExtension<DataType>(fileExtension);

    if (reader) {
        try {
            DataType* data = reader->readMetaData(file_.get());
            port_.setData(data, true);
            loadedData_ = data;
            if (deserialized) {
                dataDeserialized(data);
            } else {
                dataLoaded(data);
            }
        } catch (DataReaderException const& e) {
            LogProcessorError("Could not load data: " << file_.get() << ", " << e.getMessage());
            file_.set("");
        }
        delete reader;
    } else {
        LogProcessorError("Could not find a data reader for file: " << file_.get());
        file_.set("");
    }
}

template <typename DataType, typename PortType>
void inviwo::DataSource<DataType, PortType>::serialize(IvwSerializer& s) const {
    Processor::serialize(s);
}
/**
 * Deserialize everything first then load the data
 */
template <typename DataType, typename PortType>
void inviwo::DataSource<DataType, PortType>::deserialize(IvwDeserializer& d) {
    isDeserializing_ = true;
    Processor::deserialize(d);
    isDeserializing_ = false;
    load(true);
}

}  // namespace

#endif  // IVW_DATASOURCE_H
