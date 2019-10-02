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

#pragma once

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/geometry/typedmesh.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/rendering/texturequadrenderer.h>
#include <modules/fontrendering/textrenderer.h>

#include <modules/userinterfacegl/glui/widgets/doubleminmaxpropertywidget.h>

#include <modules/plotting/properties/categoricalaxisproperty.h>
#include <inviwo/dataframe/properties/dataframeproperty.h>
#include <inviwo/dataframe/properties/dataframecolormapproperty.h>
#include <modules/plotting/properties/marginproperty.h>

#include <modules/plottinggl/utils/axisrenderer.h>

namespace inviwo {
class PickingEvent;

/** \docpage{org.inviwo.ParallelCoordinates, Parallel Coordinates}
 * ![](org.inviwo.ParallelCoordinates.png?classIdentifier=org.inviwo.ParallelCoordinates)
 * This processor plots a given DataFrame using a Parallel Coordinate Plot.
 *
 * ### Inports
 *   * __DataFrame__  data input for plotting
 *   * __BrushingAndLinking__   inport for brushing & linking interactions
 *
 * ### Outports
 *   * __outport__   rendered image of the parallel coordinate plot
 *
 */
namespace plot {

class PCPAxisSettings;

class IVW_MODULE_PLOTTINGGL_API ParallelCoordinates : public Processor {
public:
    enum class BlendMode { None = 0, Additive = 1, Sutractive = 2, Regular = 3 };
    enum class LabelPosition { None, Above, Below };
    enum class AxisSelection { Single, Multiple, None };

public:
    ParallelCoordinates();
    virtual ~ParallelCoordinates();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

    void adjustMargins();

    void updateBrushing(PCPAxisSettings& axis);

    DataInport<DataFrame> dataFrame_;
    BrushingAndLinkingInport brushingAndLinking_;
    ImageOutport outport_;

    CompositeProperty axisProperties_;

    DataFrameColormapProperty colormap_;

    TemplateOptionProperty<AxisSelection> axisSelection_;

    CompositeProperty lineSettings_;
    TemplateOptionProperty<BlendMode> blendMode_;
    FloatProperty falllofPower_;
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
    TemplateOptionProperty<LabelPosition> captionPosition_;
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
    void axisPicked(PickingEvent* p, size_t pickedID, PickType pt);

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

        // startFilter, startRegular, startSelected, end
        std::array<size_t, 4> offsets;

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
    int hoveredLine_ = -1;
    int hoveredAxis_ = -1;

    bool brushingDirty_;
    bool updating_ = false;
};

}  // namespace plot

}  // namespace inviwo
