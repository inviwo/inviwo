/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_PERSISTENCEDIAGRAMPLOTPROCESSOR_H
#define IVW_PERSISTENCEDIAGRAMPLOTPROCESSOR_H

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>

#include <inviwo/dataframe/datastructures/dataframe.h>
#include <modules/plottinggl/plotters/persistencediagramplotgl.h>
#include <inviwo/dataframe/properties/dataframeproperty.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

#include <set>

namespace inviwo {

class PickingEvent;

namespace plot {

/** \docpage{org.inviwo.PersistenceDiagramPlotProcessor, Persistence Diagram Plot Processor}
 * ![](org.inviwo.PersistenceDiagramPlotProcessor.png?classIdentifier=org.inviwo.PersistenceDiagramPlotProcessor)
 * Plots a persistence diagram of extremum-saddle pairs. It uses x-y pairs and draws vertical lines
 * from y_low(x) = x to y_high(x) = y. Thus, the x coordinate of each pair corresponds
 * to the birth of the extremum pair as well as the lower y coordinate. The higher y coordinate
 * matches the point of death.
 *
 * ### Inports
 *   * __DataFrame__  DataFrame with at least two columns corresponding to birth and death of
 *                    extremum-saddle pairs
 *   * __BrushingAndLinking__   inport for brushing & linking interactions
 *
 * ### Outports
 *   * __outport__    rendered image of the persistence diagram
 */

/**
 * \class PersistenceDiagramPlotProcessor
 * \brief plots a persistence diagram of extremum-saddle pairs with vertical lines
 */
class IVW_MODULE_PLOTTINGGL_API PersistenceDiagramPlotProcessor : public Processor {
public:
    PersistenceDiagramPlotProcessor();
    virtual ~PersistenceDiagramPlotProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void onXAxisChange();
    void onYAxisChange();
    void onColorChange();

    DataFrameInport dataFrame_;
    BrushingAndLinkingInport brushingPort_;
    ImageInport backgroundPort_;
    ImageOutport outport_;

    PersistenceDiagramPlotGL persistenceDiagramPlot_;

    DataFrameColumnProperty xAxis_;
    DataFrameColumnProperty yAxis_;
    DataFrameColumnProperty colorCol_;

    using CallbackHandle = std::shared_ptr<std::function<void(PickingEvent*, size_t)>>;
    CallbackHandle tooltipCallBack_;

    using SelectionCallbackHandle =
        std::shared_ptr<std::function<void(const std::unordered_set<size_t>&)>>;
    SelectionCallbackHandle selectionChangedCallBack_;
};

}  // namespace plot

}  // namespace inviwo

#endif  // IVW_PERSISTENCEDIAGRAMPLOTPROCESSOR_H
