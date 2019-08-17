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

#ifndef IVW_DATAFRAMECOLUMNTOCOLORVECTOR_H
#define IVW_DATAFRAMECOLUMNTOCOLORVECTOR_H

#include <modules/plotting/plottingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/properties/dataframeproperty.h>

namespace inviwo {

namespace plot {

/** \docpage{org.inviwo.DataFrameColumnToColorVector, DataFrame Column To Color Vector}
 * ![](org.inviwo.DataFrameColumnToColorVector.png?classIdentifier=org.inviwo.DataFrameColumnToColorVector)
 * This processor maps column values of a DataFrame to colors using a 1D transfer function.
 *
 * ### Inports
 *   * __dataFrame__ input data
 *
 * ### Outports
 *   * __colors__   resulting vector of colors matching the selected DataFrame column
 *
 * ### Properties
 *   * __Selected Color Axis__   selects DataFrame column
 *   * __Color Mapping__   mapping data values to colors via a transfer function
 */

class IVW_MODULE_PLOTTING_API DataFrameColumnToColorVector : public Processor {
public:
    DataFrameColumnToColorVector();
    virtual ~DataFrameColumnToColorVector() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataInport<DataFrame> dataFrame_;
    DataOutport<std::vector<vec4>> colors_;

    DataFrameColumnProperty selectedColorAxis_;
    TransferFunctionProperty tf_;
};

}  // namespace plot

}  // namespace inviwo

#endif  // IVW_DATAFRAMECOLUMNTOCOLORVECTOR_H
