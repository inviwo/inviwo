/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_DATAEXPORT_H
#define IVW_DATAEXPORT_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
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

namespace inviwo {

/**
 * A base class for simple export processors.
 */
template <typename DataType, typename PortType = DataInport<DataType>>
class DataExport : public Processor {
public:
    DataExport();
    virtual ~DataExport() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override = 0;

protected:
    void exportData();

    virtual const DataType* getData() = 0;

    PortType port_;
    FileProperty file_;
    ButtonProperty export_;
    BoolProperty overwrite_;

    bool exportQueued_ = false;
};

template <typename DataType, typename PortType>
DataExport<DataType, PortType>::DataExport()
    : Processor()
    , port_("data")
    , file_("file", "File name", "", "mesh")
    , export_("export", "Export")
    , overwrite_("overwrite", "Overwrite", false) {

    for (auto& ext :
         InviwoApplication::getPtr()->getDataWriterFactory()->getExtensionsForType<DataType>()) {
        file_.addNameFilter(ext.toString());
    }

    addPort(port_);
    addProperty(file_);
    file_.setAcceptMode(AcceptMode::Save);
    export_.onChange([&]() { exportQueued_ = true; });
    addProperty(export_);
    addProperty(overwrite_);
}

template <typename DataType, typename PortType>
void DataExport<DataType, PortType>::exportData() {
    auto data = getData();

    if (file_.get().empty()) file_.requestFile();

    if (data && !file_.get().empty()) {
        auto factory = getNetwork()->getApplication()->getDataWriterFactory();

        auto writer = factory->template getWriterForTypeAndExtension<DataType>(
            file_.getSelectedExtension(), filesystem::getFileExtension(file_.get()));

        if (!writer) {
            LogProcessorError(
                "Error: Cound not find a writer for the specified extension and data type");
            return;
        }

        try {
            writer->setOverwrite(overwrite_.get());
            writer->writeData(data, file_.get());
            util::log(IVW_CONTEXT, "Data exported to disk: " + file_.get(), LogLevel::Info,
                      LogAudience::User);
        } catch (DataWriterException const& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error, LogAudience::User);
        }

    } else if (file_.get().empty()) {
        LogProcessorWarn("Error: Please specify a file to write to");
    } else if (!data) {
        LogProcessorWarn("Error: Please connect a port to export");
    }
}

template <typename DataType, typename PortType>
void DataExport<DataType, PortType>::process() {
    if (exportQueued_) exportData();
    exportQueued_ = false;
}

}  // namespace inviwo

#endif  // IVW_DATAEXPORT_H
