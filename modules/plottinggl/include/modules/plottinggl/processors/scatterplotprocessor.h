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

#ifndef IVW_SCATTERPLOTPROCESSOR_H
#define IVW_SCATTERPLOTPROCESSOR_H

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <modules/plottinggl/plotters/scatterplotgl.h>
#include <inviwo/dataframe/properties/dataframeproperty.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

namespace inviwo {

namespace plot {

/** \docpage{org.inviwo.ScatterPlotProcessor, Scatter Plot}
 * ![](org.inviwo.ScatterPlotProcessor.png?classIdentifier=org.inviwo.ScatterPlotProcessor)
 * This processor plots a scatter plot for a given DataFrame.
 *
 * ### Inports
 *   * __DataFrame__  data input for plotting
 *   * __BrushingAndLinking__   inport for brushing & linking interactions
 *
 * ### Outports
 *   * __outport__   rendered image of the scatter plot
 *
 */

class IVW_MODULE_PLOTTINGGL_API ScatterPlotProcessor : public Processor {
public:
    ScatterPlotProcessor();
    virtual ~ScatterPlotProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataFrameInport dataFramePort_;
    BrushingAndLinkingInport brushingPort_;
    ImageInport backgroundPort_;
    ImageOutport outport_;

    ScatterPlotGL scatterPlot_;

    DataFrameColumnProperty xAxis_;
    DataFrameColumnProperty yAxis_;
    DataFrameColumnProperty colorCol_;
    DataFrameColumnProperty radiusCol_;

    void onXAxisChange();
    void onYAxisChange();
    void onColorChange();
    void onRadiusChange();

    using CallbackHandle = std::shared_ptr<std::function<void(PickingEvent*, size_t)>>;
    CallbackHandle tooltipCallBack_;

    using SelectionCallbackHandle =
        std::shared_ptr<std::function<void(const std::unordered_set<size_t>&)>>;
    SelectionCallbackHandle selectionChangedCallBack_;
};

}  // namespace plot

}  // namespace inviwo

#endif  // IVW_SCATTERPLOTPROCESSOR_H
