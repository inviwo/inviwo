/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/ports/imageport.h>

#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/properties/filterlistproperty.h>
#include <inviwo/dataframe/util/filters.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

namespace inviwo {

/** \docpage{org.inviwo.DataFrameFilter, DataFrame Filter}
 * ![](org.inviwo.DataFrameFilter.png?classIdentifier=org.inviwo.DataFrameFilter)
 * Creates a new DataFrame by filtering the input DataFrame using either filters or Brushing and
 * Linking. Filtered rows are optionally propagated using Brushing and Linking.
 *
 * ### Inports
 *   * __inport__   filters are applied to this source DataFrame
 *   * __brushing__   inport for brushing & linking filtering
 *
 * ### Outports
 *   * __outport__ filtered DataFrame
 *
 * ### Properties
 *   * __Brushing Mode__ Determines how Brushing and Linking is considered when filtering.
 *      - @c None         no brushing and linking
 *      - @c FilterOnly   filtered rows are propagated to B&L, B&L filter state is not considered
 *      - @c ApplyOnly    apply B&L filter state to DataFrame, no brushing actions are sent
 *      - @c FilterApply  propagate filtered rows and apply B&L filter state
 */
class IVW_MODULE_DATAFRAME_API DataFrameFilter : public Processor {
public:
    enum class BrushingMode {
        None,        ///< no brushing and linking
        FilterOnly,  ///< filtered rows are propagated to B&L, B&L filter state is not considered
        ApplyOnly,   ///< apply B&L filter state to DataFrame, no brushing actions are sent
        FilterApply  ///< propagate filtered rows and apply B&L filter state
    };

    DataFrameFilter();
    virtual ~DataFrameFilter() override = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    dataframefilters::Filters createFilters() const;

    DataFrameInport inport_;
    BrushingAndLinkingInport brushing_;
    DataFrameOutport outport_;

    BoolProperty enabled_;
    OptionProperty<BrushingMode> brushingMode_;
    FilterListProperty includeFilters_;
    FilterListProperty excludeFilters_;
};

}  // namespace inviwo
