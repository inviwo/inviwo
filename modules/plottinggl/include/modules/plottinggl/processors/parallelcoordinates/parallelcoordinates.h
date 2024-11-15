/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>  // for IVW_MODULE_...

#include <inviwo/core/datastructures/bitset.h>                                // for BitSet
#include <inviwo/core/datastructures/buffer/buffer.h>                         // for IndexBuffer
#include <inviwo/core/datastructures/geometry/typedmesh.h>                    // for TypedMesh
#include <inviwo/core/interaction/pickingmapper.h>                            // for PickingMapper
#include <inviwo/core/ports/datainport.h>                                     // for DataInport
#include <inviwo/core/ports/imageport.h>                                      // for ImageOutport
#include <inviwo/core/ports/outportiterable.h>                                // for OutportIter...
#include <inviwo/core/processors/processor.h>                                 // for Processor
#include <inviwo/core/processors/processorinfo.h>                             // for ProcessorInfo
#include <inviwo/core/properties/boolcompositeproperty.h>                     // for BoolComposi...
#include <inviwo/core/properties/boolproperty.h>                              // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>                            // for ButtonProperty
#include <inviwo/core/properties/compositeproperty.h>                         // for CompositePr...
#include <inviwo/core/properties/optionproperty.h>                            // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                           // for FloatProperty
#include <inviwo/core/properties/stringproperty.h>                            // for StringProperty
#include <inviwo/core/properties/marginproperty.h>                            // for MarginProperty
#include <inviwo/core/util/glmvec.h>                                          // for vec2, size2_t
#include <inviwo/core/util/staticstring.h>                                    // for operator+
#include <inviwo/dataframe/properties/dataframecolormapproperty.h>            // for DataFrameCo...
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>         // for BrushingAnd...
#include <modules/fontrendering/properties/fontproperty.h>                    // for FontProperty
#include <modules/opengl/inviwoopengl.h>                                      // for GLsizei
#include <modules/opengl/shader/shader.h>                                     // for Shader
#include <modules/plottinggl/utils/axisrenderer.h>                            // for AxisRenderer
#include <modules/userinterfacegl/glui/renderer.h>                            // for Renderer
#include <modules/userinterfacegl/glui/widgets/doubleminmaxpropertywidget.h>  // for DoubleMinMa...

#include <array>          // for array
#include <cstddef>        // for size_t
#include <cstdint>        // for uint32_t
#include <functional>     // for __base
#include <memory>         // for unique_ptr
#include <string>         // for operator==
#include <string_view>    // for operator==
#include <unordered_map>  // for operator!=
#include <utility>        // for pair
#include <vector>         // for operator!=

#include <fmt/core.h>  // for format_to

namespace inviwo {
class DataFrame;
class Deserializer;
class PickingEvent;
class Serializer;

namespace plot {

class PCPAxisSettings;

class IVW_MODULE_PLOTTINGGL_API ParallelCoordinates : public Processor {
public:
    enum class BlendMode { None = 0, Additive = 1, Subtractive = 2, Regular = 3 };
    enum class LabelPosition { None, Above, Below };
    enum class AxisSelection { Single, Multiple, None };

public:
    ParallelCoordinates();
    virtual ~ParallelCoordinates();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

    void adjustMargins();

    void updateAxisRange(PCPAxisSettings& axis);
    void updateBrushing(PCPAxisSettings& axis);

    DataInport<DataFrame> dataFrame_;
    BrushingAndLinkingInport brushingAndLinking_;
    ImageInport imageInport_;
    ImageOutport outport_;

    CompositeProperty axisProperties_;

    DataFrameColormapProperty colormap_;

    OptionProperty<AxisSelection> axisSelection_;

    CompositeProperty lineSettings_;
    OptionProperty<BlendMode> blendMode_;
    FloatProperty falloffPower_;
    FloatProperty lineWidth_;
    CompositeProperty selectedLine_;
    FloatProperty selectedLineWidth_;
    BoolCompositeProperty selectedLineColorOverride_;
    FloatVec4Property selectedLineColor_;
    BoolProperty showFiltered_;
    FloatVec3Property filterColor_;
    FloatProperty filterAlpha_;
    FloatProperty filterIntensity_;

    FontProperty captionSettings_;
    OptionProperty<LabelPosition> captionPosition_;
    FloatProperty captionOffset_;
    FloatVec4Property captionColor_;

    FontProperty labelSettings_;
    BoolProperty showLabels_;
    FloatProperty labelOffset_;
    StringProperty labelFormat_;
    FloatVec4Property labelColor_;

    CompositeProperty axesSettings_;
    FloatProperty axisSize_;
    FloatVec4Property axisColor_;
    FloatVec4Property axisHoverColor_;
    FloatVec4Property axisSelectedColor_;
    BoolProperty handlesVisible_;
    FloatProperty handleSize_;
    FloatVec4Property handleColor_;
    FloatVec4Property handleFilteredColor_;

    MarginProperty margins_;
    BoolProperty includeLabelsInMargin_;
    ButtonProperty resetHandlePositions_;

    int getHoveredAxis() const { return hoveredAxis_; }

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    void linePicked(PickingEvent* p);
    enum class PickType { Axis, Lower, Upper, Groove };
    void axisPicked(PickingEvent* p, uint32_t columnId, PickType pt);

private:
    struct ColumnAxis {
        PCPAxisSettings* pcp;
        std::unique_ptr<AxisRenderer> axisRender;
        std::unique_ptr<glui::DoubleMinMaxPropertyWidget> sliderWidget;
    };

    void createOrUpdateProperties();

    void buildLineMesh();
    void buildLineIndices();
    void buildAxisPositions();
    void partitionLines();
    void drawAxis(size2_t size);
    void drawHandles(size2_t size);
    void drawLines(size2_t size);

    void updateBrushing();

    std::pair<size2_t, size2_t> axisPos(size_t columnId) const;

    /**
     * Returns display area excluding margins as lower left and upper right point.
     */
    std::pair<vec2, vec2> getDisplayRect(vec2 size) const;

    glui::Renderer sliderWidgetRenderer_;
    std::vector<ColumnAxis> axes_;

    bool enabledAxesModified_ = false;

    // The enabled axis and the order they are shown, i.e if and in which order the dataFrame
    // columns are shown, The value correspond to an index in the axes_ array. Not a dataFrame
    // column index since a dataFrame vec column will have multiple axes.
    std::vector<size_t> enabledAxes_;

    PickingMapper linePicking_;
    PickingMapper axisPicking_;
    bool isDragging_ = false;

    Shader lineShader_;

    struct Lines {
        TypedMesh<buffertraits::PositionsBuffer1D, buffertraits::PickingBuffer,
                  buffertraits::ScalarMetaBuffer>
            mesh;
        IndexBuffer indices;
        std::vector<GLsizei> sizes;
        std::vector<size_t> starts;

        // startFilter, startRegular, startSelected, startHighlighted, end
        std::array<size_t, 5> offsets;

        std::vector<float> axisPositions;
        // using int here for performance reasons since bool is not supported as GLSL uniform
        // A bool vector would internally be converted to an int array prior setting the uniform.
        // \see UniformSetter<std::array<bool, N>>
        std::vector<int> axisFlipped;

        inline static size_t offsetToIndex(size_t offset, size_t cols) {
            return offset / (cols * sizeof(uint32_t));
        }
        inline static size_t indexToOffset(size_t index, size_t cols) {
            return index * cols * sizeof(uint32_t);
        }
    };
    Lines lines_;

    std::pair<vec2, vec2> marginsInternal_;  // Margins with/without considering labels
    BitSet highlightedLines_;
    int hoveredAxis_ = -1;

    bool brushingDirty_;
    bool updating_ = false;
};

}  // namespace plot

}  // namespace inviwo
