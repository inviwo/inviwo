/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/util/dispatcher.h>

#include <modules/base/algorithm/dataminmax.h>
#include <modules/basegl/properties/stipplingproperty.h>
#include <modules/basegl/properties/linesettingsproperty.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shader.h>

#include <inviwo/dataframe/datastructures/dataframe.h>

#include <modules/plotting/interaction/boxselectioninteractionhandler.h>
#include <modules/plotting/properties/marginproperty.h>
#include <modules/plotting/properties/axisproperty.h>
#include <modules/plotting/properties/axisstyleproperty.h>

#include <modules/plottinggl/rendering/boxselectionrenderer.h>
#include <modules/plottinggl/utils/axisrenderer.h>

#include <optional>
#include <unordered_set>

namespace inviwo {

class Processor;
class PickingEvent;
class BufferObjectArray;

namespace plot {

class IVW_MODULE_PLOTTINGGL_API ScatterPlotGL : public InteractionHandler {
public:
    using ToolTipFunc = void(PickingEvent*, size_t);
    using ToolTipCallbackHandle = std::shared_ptr<std::function<ToolTipFunc>>;
    using HighlightFunc = void(const BitSet&);
    using HighlightCallbackHandle = std::shared_ptr<std::function<HighlightFunc>>;
    using SelectionFunc = void(const BitSet&);
    using SelectionCallbackHandle = std::shared_ptr<std::function<SelectionFunc>>;

    enum class SortingOrder { Ascending, Descending };

    class Properties : public CompositeProperty {
    public:
        virtual std::string getClassIdentifier() const override;
        static const std::string classIdentifier;

        Properties(std::string_view identifier, std::string_view displayName,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                   PropertySemantics semantics = PropertySemantics::Default);

        Properties(const Properties& rhs);
        virtual Properties* clone() const override;
        virtual ~Properties() = default;

        BoolProperty useCircle_;
        FloatProperty radiusRange_;
        FloatProperty minRadius_;
        TransferFunctionProperty tf_;
        FloatVec4Property color_;

        BoolCompositeProperty showHighlighted_;
        FloatVec3Property highlightColor_;
        FloatProperty highlightAlpha_;
        FloatProperty highlightIntensity_;

        BoolCompositeProperty showSelected_;
        FloatVec3Property selectionColor_;
        FloatProperty selectionAlpha_;
        FloatProperty selectionIntensity_;

        BoolCompositeProperty showFiltered_;
        FloatVec3Property filterColor_;
        FloatProperty filterAlpha_;
        FloatProperty filterIntensity_;

        BoolProperty tooltip_;

        BoxSelectionProperty boxSelectionSettings_;  ///! (Mouse) Drag selection/filtering
        MarginProperty margins_;
        FloatProperty axisMargin_;

        FloatProperty borderWidth_;
        FloatVec4Property borderColor_;

        AxisStyleProperty axisStyle_;
        AxisProperty xAxis_;
        AxisProperty yAxis_;

    private:
        auto props() {
            return std::tie(radiusRange_, useCircle_, minRadius_, tf_, color_, showHighlighted_,
                            showSelected_, showFiltered_, tooltip_, boxSelectionSettings_, margins_,
                            axisMargin_, borderWidth_, borderColor_, axisStyle_, xAxis_, yAxis_);
        }
        auto props() const {
            return std::tie(radiusRange_, useCircle_, minRadius_, tf_, color_, showHighlighted_,
                            showSelected_, showFiltered_, tooltip_, boxSelectionSettings_, margins_,
                            axisMargin_, borderWidth_, borderColor_, axisStyle_, xAxis_, yAxis_);
        }
    };

    explicit ScatterPlotGL(Processor* processor = nullptr);
    virtual ~ScatterPlotGL() = default;

    void plot(Image& dest, bool useAxisRanges = false);
    void plot(Image& dest, const Image& src, bool useAxisRanges = false);
    void plot(ImageOutport& dest, bool useAxisRanges = false);
    void plot(ImageOutport& dest, ImageInport& src, bool useAxisRanges = false);
    void plot(const ivec2& start, const ivec2& size, bool useAxisRanges = false);

    void setXAxisLabel(const std::string& label);

    void setYAxisLabel(const std::string& label);

    void setXAxis(const Column* col);

    void setYAxis(const Column* col);

    void setXAxisData(const Column* col);
    void setYAxisData(const Column* col);
    void setColorData(const Column* col);
    void setRadiusData(const Column* col);
    void setSortingData(const Column* col);
    void setIndexColumn(std::shared_ptr<const TemplateColumn<uint32_t>> indexcol);

    void setSortingOrder(SortingOrder order);

    /**
     * Set the brushed indices for filtered, selected, and highlighted data points.
     * Note that the indices are supposed to be zero-based that is idx in [0, numDataPoints).
     * @see setFilteredIndices setSelectedIndices setHighlightedIndices
     */
    void setIndices(const BitSet& filtered, const BitSet& selected, const BitSet& highlighted);
    /**
     * Set the brushed indices for filtered data points only.
     * Note that the indices are supposed to be zero-based that is idx in [0, numDataPoints).
     * @see setIndices setSelectedIndices setHighlightedIndices
     */
    void setFilteredIndices(const BitSet& indices);
    /**
     * Set the brushed indices for selected data points only.
     * Note that the indices are supposed to be zero-based that is idx in [0, numDataPoints).
     * @see setIndices setFilteredIndices setHighlightedIndices
     */
    void setSelectedIndices(const BitSet& indices);
    /**
     * Set the brushed indices for highlighted data points only.
     * Note that the indices are supposed to be zero-based that is idx in [0, numDataPoints).
     * @see setIndices setFilteredIndices setSelectedIndices
     */
    void setHighlightedIndices(const BitSet& indices);

    ToolTipCallbackHandle addToolTipCallback(std::function<ToolTipFunc> callback);
    HighlightCallbackHandle addHighlightChangedCallback(std::function<HighlightFunc> callback);
    SelectionCallbackHandle addSelectionChangedCallback(std::function<SelectionFunc> callback);
    SelectionCallbackHandle addFilteringChangedCallback(std::function<SelectionFunc> callback);

    // InteractionHandler
    virtual void invokeEvent(Event* event) override;
    virtual std::string getClassIdentifier() const override { return "org.inviwo.scatterplotgl"; };

    Properties properties_;
    Shader shader_;

protected:
    void plot(const size2_t& dims, bool useAxisRanges);
    void attachVertexAttributes();
    void setShaderUniforms(TextureUnitContainer& cont, const size2_t& dims, bool useAxisRanges);
    void renderAxis(const size2_t& dims);

    void objectPicked(PickingEvent* p);
    uint32_t getGlobalPickId(uint32_t localIndex) const;

    void partitionData();

    struct Points {
        BufferObjectArray boa;
        IndexBuffer indices;

        // startFilter, startRegular, startSelected, startHighlighted, end
        std::array<std::uint32_t, 5> offsets;

        std::shared_ptr<const BufferBase> xCoord;
        std::shared_ptr<const BufferBase> yCoord;
        std::shared_ptr<const BufferBase> color;
        std::shared_ptr<const BufferBase> radius;
        std::shared_ptr<const BufferBase> sorting;
        Buffer<std::uint32_t> pickIds;
    };
    Points points_;

    std::shared_ptr<const TemplateColumn<uint32_t>> indexColumn_;

    SortingOrder sortOrder_ = SortingOrder::Ascending;

    vec2 minmaxX_;
    vec2 minmaxY_;
    vec2 minmaxC_;
    vec2 minmaxR_;

    std::array<AxisRenderer, 2> axisRenderers_;

    PickingMapper picking_;

    BitSet filtered_;
    BitSet selected_;
    BitSet highlighted_;

    bool partitionDirty_;
    Processor* processor_;

    Dispatcher<ToolTipFunc> tooltipCallback_;
    Dispatcher<HighlightFunc> highlightChangedCallback_;
    Dispatcher<SelectionFunc> selectionChangedCallback_;
    Dispatcher<SelectionFunc> filteringChangedCallback_;

    BoxSelectionInteractionHandler::SelectionCallbackHandle boxSelectionChangedCallback_;
    BoxSelectionInteractionHandler::SelectionCallbackHandle boxFilteringChangedCallback_;

    BoxSelectionInteractionHandler boxSelectionHandler_;
    BoxSelectionRenderer selectionRectRenderer_;
};

}  // namespace plot

}  // namespace inviwo
