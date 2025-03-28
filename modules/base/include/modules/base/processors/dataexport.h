/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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
#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterexception.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filedialogstate.h>
#include <inviwo/core/common/factoryutil.h>

namespace inviwo {

/**
 * A base class for simple export processors.
 */
template <typename DataType, typename PortType = DataInport<DataType>>
class DataExport : public Processor {
public:
    DataExport(DataWriterFactory* wf = util::getDataWriterFactory(),
               const std::filesystem::path& file = {},
               std::string_view contentType = FileProperty::defaultContentType);
    DataExport(InviwoApplication* app, const std::filesystem::path& file = {},
               std::string_view contentType = FileProperty::defaultContentType);

    virtual ~DataExport() = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override = 0;

protected:
    void exportData();

    virtual const DataType* getData() = 0;

    DataWriterFactory* wf_;
    PortType port_;
    FileProperty file_;
    ButtonProperty export_;
    BoolProperty overwrite_;

    bool exportQueued_ = false;
};

template <typename DataType, typename PortType>
DataExport<DataType, PortType>::DataExport(InviwoApplication* app,
                                           const std::filesystem::path& file,
                                           std::string_view contentType)
    : DataExport{util::getDataWriterFactory(app), file, contentType} {}

template <typename DataType, typename PortType>
DataExport<DataType, PortType>::DataExport(DataWriterFactory* wf, const std::filesystem::path& file,
                                           std::string_view contentType)
    : Processor()
    , wf_{wf}
    , port_{"data", "The data to export"_help}
    , file_{"file", "File name", "The file path to save data to"_help, file, contentType}
    , export_{"export", "Export", "Save data to disk"_help}
    , overwrite_{"overwrite", "Overwrite", "Overwrite any existing data in file"_help, false,
                 InvalidationLevel::Valid} {

    for (auto& ext : wf_->getExtensionsForType<DataType>()) {
        file_.addNameFilter(ext.toString());
    }

    addPort(port_);
    addProperty(file_);
    file_.setAcceptMode(AcceptMode::Save);
    export_.onChange([&]() {
        if (port_.hasData()) {
            if (file_.get().empty()) {
                file_.requestFile();
                // file request got canceled, do nothing
                if (file_.get().empty()) return;
            }
            exportQueued_ = true;
        }
    });
    addProperty(export_);
    addProperty(overwrite_);
}

template <typename DataType, typename PortType>
void DataExport<DataType, PortType>::exportData() {
    auto data = getData();

    if (file_.get().empty()) {
        throw Exception("Please specify a file to write to");
    }
    if (!data) {
        throw Exception("Please connect a port to export");
    }

    auto writer = wf_->template getWriterForTypeAndExtension<DataType>(file_.getSelectedExtension(),
                                                                       file_.get());

    if (!writer) {
        throw Exception("Could not find a writer for the specified extension and data type");
    }

    writer->setOverwrite(overwrite_ ? Overwrite::Yes : Overwrite::No);
    writer->writeData(data, file_.get());
    log::info("Data exported to disk: {}", file_.get().string());

    // update widgets as the file might now exist
    file_.clearInitiatingWidget();
    file_.updateWidgets();
}

template <typename DataType, typename PortType>
void DataExport<DataType, PortType>::process() {
    if (exportQueued_) exportData();
    exportQueued_ = false;
}

}  // namespace inviwo
