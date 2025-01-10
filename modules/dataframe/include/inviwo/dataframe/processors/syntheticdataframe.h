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

#include <inviwo/core/ports/dataoutport.h>              // for DataOutport
#include <inviwo/core/processors/processor.h>           // for Processor
#include <inviwo/core/processors/processorinfo.h>       // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>        // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>   // for CompositeProperty
#include <inviwo/core/properties/ordinalproperty.h>     // for IntProperty, IntSizeTProperty
#include <inviwo/dataframe/datastructures/dataframe.h>  // for DataFrame

#include <string>  // for operator+, string

#include <fmt/core.h>  // for format

namespace inviwo {

/** \docpage{org.inviwo.SyntheticDataFrame, Synthetic DataFrame}
 * ![](org.inviwo.SyntheticDataFrame.png?classIdentifier=org.inviwo.SyntheticDataFrame)
 * This processor generates a DataFrame filled with random values.
 *
 * ### Outports
 *   * __Outport__   resulting DataFrame containing n rows with random values
 *
 * ### Properties
 *   * __Number of Rows__  defines the size of the generated DataFrame
 */

class IVW_MODULE_DATAFRAME_API SyntheticDataFrame : public Processor {
public:
    SyntheticDataFrame();
    virtual ~SyntheticDataFrame() = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataOutport<DataFrame> dataFrame_;

    IntSizeTProperty numRow_;
    BoolProperty includeSingleValueColumnsFloat_;
    BoolProperty includeSingleValueColumnsInt_;

    CompositeProperty randomParams_;
    BoolProperty useSameSeed_;  ///< Use the same seed for each call to process.
    IntProperty seed_;          ///<  The seed used to initialize the random sequence
};

}  // namespace inviwo
