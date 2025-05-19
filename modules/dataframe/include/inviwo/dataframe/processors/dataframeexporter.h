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

#include <inviwo/dataframe/dataframemoduledefine.h>  // for IVW_MODULE_DATAFRAME_API

#include <inviwo/core/ports/datainport.h>           // for DataInport
#include <inviwo/core/ports/outportiterable.h>      // for OutportIterable
#include <inviwo/core/processors/processor.h>       // for Processor
#include <inviwo/core/processors/processorinfo.h>   // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>    // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>  // for ButtonProperty
#include <inviwo/core/properties/fileproperty.h>    // for FileProperty
#include <inviwo/core/properties/stringproperty.h>  // for StringProperty
#include <inviwo/core/util/glmvec.h>                // for uvec3

#include <string>         // for string
#include <unordered_map>  // for operator!=
#include <vector>         // for vector

#include <fmt/core.h>  // for format_to, basic_string_view, format

namespace inviwo {
class DataFrame;
class FileExtension;

class IVW_MODULE_DATAFRAME_API DataFrameExporter : public Processor {
public:
    DataFrameExporter();
    virtual ~DataFrameExporter() = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    void exportData();

private:
    void exportAsCSV();
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

    bool exportQueued_;
};

}  // namespace inviwo
