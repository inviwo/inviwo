/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_SCATTERPLOTGL_H
#define IVW_SCATTERPLOTGL_H

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shader.h>
#include <modules/base/algorithm/dataminmax.h>

#include <modules/plotting/datastructures/dataframe.h>
#include <modules/plotting/properties/marginproperty.h>
#include <modules/plotting/properties/axisproperty.h>

#include <modules/plottinggl/utils/axisrenderer.h>

namespace inviwo {

namespace plot {

class IVW_MODULE_PLOTTINGGL_API ScatterPlotGL {
public:
    class Properties : public CompositeProperty {
    public:
        InviwoPropertyInfo();

        Properties(std::string identifier, std::string displayName,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                   PropertySemantics semantics = PropertySemantics::Default);

        Properties(const Properties &rhs);
        Properties &operator=(const Properties &that);
        virtual Properties *clone() const override;
        virtual ~Properties() = default;

        BoolProperty useCircle_;
        FloatProperty radiusRange_;
        FloatProperty minRadius_;
        FloatVec4Property color_;
        TransferFunctionProperty tf_;
        MarginProperty margins_;
        FloatProperty axisMargin_;

        FloatProperty borderWidth_;
        FloatVec4Property borderColor_;

        AxisProperty xAxis_;
        AxisProperty yAxis_;
    };

    ScatterPlotGL(Processor *processor);
    ScatterPlotGL();
    virtual ~ScatterPlotGL() = default;

    void plot(Image &dest, IndexBuffer *indices = nullptr);
    void plot(Image &dest, const Image &src, IndexBuffer *indices = nullptr);
    void plot(ImageOutport &dest, IndexBuffer *indices = nullptr);
    void plot(ImageOutport &dest, ImageInport &src, IndexBuffer *indices = nullptr);

    void plot(const ivec2 &start, const ivec2 &size, IndexBuffer *indices = nullptr);

    void setXAxisLabel(const std::string &label);

    void setYAxisLabel(const std::string &label);

    void setXAxis(std::shared_ptr<const plot::Column> col);

    void setYAxis(std::shared_ptr<const plot::Column> col);

    void setXAxisData(std::shared_ptr<const BufferBase> buffer);
    void setYAxisData(std::shared_ptr<const BufferBase> buffer);
    void setColorData(std::shared_ptr<const BufferBase> buffer);
    void setRadiusData(std::shared_ptr<const BufferBase> buffer);

    Properties properties_;
    Shader shader_;

protected:
    void plot(const size2_t &dims, IndexBuffer *indices);
    void renderAxis(const size2_t &dims);

    std::shared_ptr<const BufferBase> xAxis_;
    std::shared_ptr<const BufferBase> yAxis_;
    std::shared_ptr<const BufferBase> color_;
    std::shared_ptr<const BufferBase> radius_;

    vec2 minmaxX_;
    vec2 minmaxY_;
    vec2 minmaxC_;
    vec2 minmaxR_;

    std::array<AxisRenderer, 2> axisRenderers_;
};

}  // namespace plot

}  // namespace inviwo

#endif  // IVW_SCATTERPLOT_H
