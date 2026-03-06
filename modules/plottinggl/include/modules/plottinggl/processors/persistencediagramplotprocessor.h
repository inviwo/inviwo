/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2026 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>

#include <inviwo/core/datastructures/bitset.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/properties/columnoptionproperty.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/plottinggl/plotters/persistencediagramplotgl.h>

#include <cstddef>
#include <functional>
#include <memory>

namespace inviwo {

class PickingEvent;

namespace plot {

/**
 * @brief plots a persistence diagram of extremum-saddle pairs with vertical lines
 */
class IVW_MODULE_PLOTTINGGL_API PersistenceDiagramPlotProcessor : public Processor {
public:
    PersistenceDiagramPlotProcessor();
    virtual ~PersistenceDiagramPlotProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
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

    ColumnOptionProperty birth_;
    ColumnOptionProperty death_;
    ColumnOptionProperty persistence_;
    ColumnOptionProperty colorCol_;

    using CallbackHandle = std::shared_ptr<std::function<void(PickingEvent*, size_t)>>;
    CallbackHandle tooltipCallBack_;

    using SelectionCallbackHandle = std::shared_ptr<std::function<void(const BitSet&)>>;
    SelectionCallbackHandle selectionChangedCallBack_;
};

}  // namespace plot

}  // namespace inviwo
