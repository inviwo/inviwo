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

#include <modules/plotting/plottingmoduledefine.h>  // for IVW_MODULE_PLOTTING_API

#include <inviwo/core/ports/datainport.h>                      // for DataInport
#include <inviwo/core/ports/dataoutport.h>                     // for DataOutport
#include <inviwo/core/ports/outportiterable.h>                 // for OutportIterable
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/properties/transferfunctionproperty.h>   // for TransferFunctionProperty
#include <inviwo/core/util/glmvec.h>                           // for vec4, uvec3
#include <inviwo/core/util/staticstring.h>                     // for operator+
#include <inviwo/dataframe/properties/columnoptionproperty.h>  // for ColumnOptionProperty

#include <string>         // for operator+, string
#include <unordered_map>  // for operator!=
#include <vector>         // for vector

#include "glm/ext/vector_uint3.hpp"  // for uvec3
#include <fmt/core.h>                // for format, format_to, basic_s...
#include <glm/gtx/io.hpp>            // for operator<<

namespace inviwo {
class DataFrame;

namespace plot {

class IVW_MODULE_PLOTTING_API DataFrameColumnToColorVector : public Processor {
public:
    DataFrameColumnToColorVector();
    virtual ~DataFrameColumnToColorVector() = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataInport<DataFrame> dataFrame_;
    DataOutport<std::vector<vec4>> colors_;

    ColumnOptionProperty selectedColorAxis_;
    TransferFunctionProperty tf_;
};

}  // namespace plot

}  // namespace inviwo
