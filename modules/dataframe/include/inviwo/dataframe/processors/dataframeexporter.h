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

#ifndef IVW_DATAFRAMETOCSVEXPORTER_H
#define IVW_DATAFRAMETOCSVEXPORTER_H

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/dataframe/datastructures/dataframe.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/util/fileextension.h>

namespace inviwo {

/** \docpage{org.inviwo.DataFrameExporter, DataFrame Exporter}
 * ![](org.inviwo.DataFrameExporter.png?classIdentifier=org.inviwo.DataFrameExporter)
 * This processor exports a DataFrame into a CSV or XML file.
 *
 * ### Inports
 *   * __<Inport>__ source DataFrame which is saved as CSV or XML file
 *
 */

class IVW_MODULE_DATAFRAME_API DataFrameExporter : public Processor {
public:
    DataFrameExporter();
    virtual ~DataFrameExporter() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    void exportNow();

private:
    void exportAsCSV(bool separateVectorTypesIntoColumns = true);
    void exportAsXML();

    DataInport<DataFrame> dataFrame_;

    FileProperty exportFile_;
    ButtonProperty exportButton_;
    BoolProperty overwrite_;
    BoolProperty exportIndexCol_;
    BoolProperty separateVectorTypesIntoColumns_;
    BoolProperty quoteStrings_;
    StringProperty delimiter_;

    static FileExtension csvExtension_;
    static FileExtension xmlExtension_;

    bool export_;
};

}  // namespace inviwo

#endif  // IVW_DATAFRAMETOCSVEXPORTER_H
