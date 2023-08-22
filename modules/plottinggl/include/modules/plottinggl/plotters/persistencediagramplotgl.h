/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>  // for IVW_MODULE_PLOTTI...

#include <inviwo/core/datastructures/bitset.h>                          // for BitSet
#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer, BufferBas...
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferTarget, Buf...
#include <inviwo/core/datastructures/image/image.h>                     // for Image
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/interaction/pickingmapper.h>                      // for PickingMapper
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>                   // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                     // for FloatProperty
#include <inviwo/core/properties/propertysemantics.h>                   // for PropertySemantics
#include <inviwo/core/properties/transferfunctionproperty.h>            // for TransferFunctionP...
#include <inviwo/core/util/dispatcher.h>                                // for Dispatcher
#include <inviwo/core/util/glmvec.h>                                    // for size2_t, vec2, ivec2
#include <modules/opengl/shader/shader.h>                               // for Shader
#include <modules/opengl/texture/textureutils.h>                        // for ImageInport
#include <modules/plotting/properties/axisproperty.h>                   // for AxisProperty
#include <modules/plotting/properties/axisstyleproperty.h>              // for AxisStyleProperty
#include <modules/plotting/properties/marginproperty.h>                 // for MarginProperty
#include <modules/plottinggl/utils/axisrenderer.h>                      // for AxisRenderer

#include <array>          // for array
#include <cstddef>        // for size_t
#include <cstdint>        // for uint32_t
#include <functional>     // for function
#include <memory>         // for shared_ptr, uniqu...
#include <set>            // for set
#include <string>         // for string
#include <tuple>          // for tie
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

namespace inviwo {

class Column;
class ImageOutport;
class Processor;
template <typename T>
class TemplateColumn;

using IndexBuffer = Buffer<std::uint32_t, BufferTarget::Index>;
class PickingEvent;

namespace plot {

/**
 * \class PersistenceDiagramPlotGL
 * \brief base class for plotting a persistence diagram using OpenGL
 */
class IVW_MODULE_PLOTTINGGL_API PersistenceDiagramPlotGL {
public:
    using ToolTipFunc = void(PickingEvent*, size_t);
    using ToolTipCallbackHandle = std::shared_ptr<std::function<ToolTipFunc>>;
    using SelectionFunc = void(const BitSet&);
    using SelectionCallbackHandle = std::shared_ptr<std::function<SelectionFunc>>;

    class Properties : public CompositeProperty {
    public:
        virtual std::string getClassIdentifier() const override;
        static const std::string classIdentifier;

        Properties(std::string identifier, std::string displayName,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                   PropertySemantics semantics = PropertySemantics::Default);

        Properties(const Properties& rhs);
        virtual Properties* clone() const override;
        virtual ~Properties() = default;

        BoolProperty showPoints_;
        FloatProperty radius_;
        FloatProperty lineWidth_;
        FloatProperty lineWidthDiagonal_;
        FloatVec4Property pointColor_;
        FloatVec4Property lineColor_;
        FloatVec4Property hoverColor_;
        FloatVec4Property selectionColor_;
        TransferFunctionProperty tf_;
        MarginProperty margins_;
        FloatProperty axisMargin_;

        FloatProperty borderWidth_;
        FloatVec4Property borderColor_;

        BoolProperty hovering_;

        AxisStyleProperty axisStyle_;
        AxisProperty xAxis_;
        AxisProperty yAxis_;

    private:
        auto props() {
            return std::tie(showPoints_, radius_, lineWidth_, lineWidthDiagonal_, pointColor_,
                            lineColor_, hoverColor_, selectionColor_, tf_, margins_, axisMargin_,
                            borderWidth_, borderColor_, hovering_, axisStyle_, xAxis_, yAxis_);
        }
        auto props() const {
            return std::tie(showPoints_, radius_, lineWidth_, lineWidthDiagonal_, pointColor_,
                            lineColor_, hoverColor_, selectionColor_, tf_, margins_, axisMargin_,
                            borderWidth_, borderColor_, hovering_, axisStyle_, xAxis_, yAxis_);
        }
    };

    explicit PersistenceDiagramPlotGL(Processor* processor = nullptr);
    virtual ~PersistenceDiagramPlotGL() = default;

    void plot(Image& dest, IndexBuffer* indices = nullptr, bool useAxisRanges = false);
    void plot(Image& dest, const Image& src, IndexBuffer* indices = nullptr,
              bool useAxisRanges = false);
    void plot(ImageOutport& dest, IndexBuffer* indices = nullptr, bool useAxisRanges = false);
    void plot(ImageOutport& dest, ImageInport& src, IndexBuffer* indices = nullptr,
              bool useAxisRanges = false);
    void plot(const ivec2& start, const ivec2& size, IndexBuffer* indices = nullptr,
              bool useAxisRanges = false);

    void setXAxisLabel(const std::string& label);

    void setYAxisLabel(const std::string& label);

    void setXAxis(std::shared_ptr<const Column> col);

    enum class Type { Death, Persistence };
    void setYAxis(std::shared_ptr<const Column> col, Type type = Type::Death);

    void setXAxisData(std::shared_ptr<const BufferBase> buffer);
    void setYAxisData(std::shared_ptr<const BufferBase> buffer);
    void setColorData(std::shared_ptr<const BufferBase> buffer);

    void setIndexColumn(std::shared_ptr<const TemplateColumn<uint32_t>> indexcol);

    void setSelectedIndices(const BitSet& indices);

    ToolTipCallbackHandle addToolTipCallback(std::function<ToolTipFunc> callback);
    SelectionCallbackHandle addSelectionChangedCallback(std::function<SelectionFunc> callback);

    Properties properties_;
    Shader pointShader_;
    Shader lineShader_;

protected:
    void plot(const size2_t& dims, IndexBuffer* indices, bool useAxisRanges);

    void renderLines(const size2_t& dims, const std::vector<uint32_t>& diagonalIndices,
                     const std::vector<uint32_t>& indices);
    void renderPoints(const size2_t& dims, const std::vector<uint32_t>& indices);
    void renderAxis(const size2_t& dims);

    void objectPicked(PickingEvent* p);
    uint32_t getGlobalPickId(uint32_t localIndex) const;

    Type type = Type::Death;
    std::shared_ptr<const BufferBase> xAxis_;
    std::shared_ptr<const BufferBase> yAxis_;
    std::shared_ptr<const BufferBase> color_;
    std::shared_ptr<const TemplateColumn<uint32_t>> indexColumn_;

    vec2 minmaxX_;
    vec2 minmaxY_;
    vec2 minmaxC_;
    vec2 minmaxR_;

    std::array<AxisRenderer, 2> axisRenderers_;

    PickingMapper picking_;
    BitSet selectedIndices_;
    std::set<uint32_t> hoveredIndices_;

    Processor* processor_;

    Dispatcher<ToolTipFunc> tooltipCallback_;
    Dispatcher<SelectionFunc> selectionChangedCallback_;
};

}  // namespace plot

}  // namespace inviwo
