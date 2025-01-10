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

#include <modules/plottinggl/plottingglmoduledefine.h>  // for IVW_MODULE_PLOTTIN...

#include <inviwo/core/datastructures/bitset.h>                         // for BitSet
#include <inviwo/core/ports/imageport.h>                               // for ImageOutport
#include <inviwo/core/processors/processor.h>                          // for Processor
#include <inviwo/core/processors/processorinfo.h>                      // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>                       // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>                  // for CompositeProperty
#include <inviwo/core/properties/eventproperty.h>                      // for EventProperty
#include <inviwo/core/properties/ordinalproperty.h>                    // for IntProperty, Float...
#include <inviwo/core/properties/transferfunctionproperty.h>           // for TransferFunctionPr...
#include <inviwo/dataframe/datastructures/dataframe.h>                 // for DataFrameInport
#include <inviwo/dataframe/properties/columnoptionproperty.h>          // for ColumnOptionProperty
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>  // for BrushingAndLinking...
#include <modules/fontrendering/properties/fontfaceoptionproperty.h>   // for FontFaceOptionProp...
#include <modules/fontrendering/textrenderer.h>                        // for TextRenderer
#include <modules/opengl/rendering/texturequadrenderer.h>              // for TextureQuadRenderer
#include <modules/plottinggl/plotters/scatterplotgl.h>                 // for ScatterPlotGL, Sca...

#include <cstddef>        // for size_t
#include <cstdint>        // for uint32_t
#include <memory>         // for shared_ptr, unique...
#include <unordered_map>  // for unordered_map
#include <vector>         // for vector

namespace inviwo {
class Column;
class Texture2D;

namespace plot {

/** \docpage{org.inviwo.ScatterPlotMatrixProcessor, Scatter Plot Matrix}
 * ![](org.inviwo.ScatterPlotMatrixProcessor.png?classIdentifier=org.inviwo.ScatterPlotMatrixProcessor)
 * This processor creates a scatter plot matrix for a given DataFrame.
 *
 * ### Inports
 *   * __DataFrame__  data input for plotting
 *   * __BrushingAndLinking__   inport for brushing & linking interactions
 *
 * ### Outports
 *   * __outport__   rendered image of the scatter plot matrix
 *
 */

class IVW_MODULE_PLOTTINGGL_API ScatterPlotMatrixProcessor : public Processor {
public:
    ScatterPlotMatrixProcessor();
    virtual ~ScatterPlotMatrixProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataFrameInport dataFrame_;
    BrushingAndLinkingInport brushing_;
    ImageOutport outport_;

    std::vector<std::unique_ptr<ScatterPlotGL>> plots_;

    void createScatterPlots();
    void createLabels();
    void createStatsLabels();
    /**
     * Test wether a given column should be included in the rendering or not. Does this by looking
     * up or creating a bool property in parameters_.
     */
    bool isIncluded(std::shared_ptr<Column> col);

    size_t numParams_;

    ScatterPlotGL::Properties scatterPlotproperties_;
    ColumnOptionProperty color_;

    ColumnOptionProperty selectedX_;
    ColumnOptionProperty selectedY_;

    CompositeProperty labels_;
    FloatVec4Property fontColor_;

    FontFaceOptionProperty fontFace_;
    IntProperty fontSize_;
    FontFaceOptionProperty fontFaceStats_;
    IntProperty statsFontSize_;
    BoolProperty showCorrelationValues_;  // Show numerical correlation values

    CompositeProperty parameters_;

    TransferFunctionProperty correlationTF_;

    std::vector<std::shared_ptr<Texture2D>> labelsTextures_;
    std::vector<std::shared_ptr<Texture2D>> statsTextures_;
    std::vector<std::shared_ptr<Texture2D>> bgTextures_;

    TextRenderer textRenderer_;
    TextureQuadRenderer textureQuadRenderer_;

    EventProperty mouseEvent_;

    std::unordered_map<size_t, int> visibleIDToColumnID_;  //! Helper map to convert from ids in
                                                           //! "matrix order" (as shown on screen)
                                                           //! to index of column in the dataframe.

    BitSet filteredIndices_;
    std::unordered_map<uint32_t, uint32_t> indexToRowMap_;
};

}  // namespace plot

}  // namespace inviwo
