/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#ifndef IVW_SYNTHETICDATAFRAME_H
#define IVW_SYNTHETICDATAFRAME_H

#include <modules/plotting/plottingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/ports/dataoutport.h>
#include <modules/plotting/datastructures/dataframe.h>

namespace inviwo {

namespace plot {

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

class IVW_MODULE_PLOTTING_API SyntheticDataFrame : public Processor {
public:
    SyntheticDataFrame();
    virtual ~SyntheticDataFrame() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataOutport<DataFrame> dataFrame_;

    IntSizeTProperty numRow_;

    CompositeProperty randomParams_;
    BoolProperty useSameSeed_;  ///< Use the same seed for each call to process.
    IntProperty seed_;          ///<  The seed used to initialize the random sequence
};

}  // namespace plot

}  // namespace inviwo

#endif  // IVW_SYNTHETICDATAFRAME_H
