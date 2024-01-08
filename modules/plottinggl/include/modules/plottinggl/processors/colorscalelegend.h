/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>  // for IVW_MODULE_PLOTTINGGL_API

#include <inviwo/core/ports/imageport.h>                   // for ImageInport, ImageOutport
#include <inviwo/core/ports/volumeport.h>                  // for VolumeInport
#include <inviwo/core/processors/processor.h>              // for Processor
#include <inviwo/core/processors/processorinfo.h>          // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>           // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>      // for CompositeProperty
#include <inviwo/core/properties/boolcompositeproperty.h>  // for BoolCompositeProperty
#include <inviwo/core/properties/isotfproperty.h>          // for IsoTFProperty
#include <inviwo/core/properties/optionproperty.h>         // for OptionProperty, OptionPropert...
#include <inviwo/core/properties/ordinalproperty.h>        // for IntProperty, FloatProperty
#include <inviwo/core/properties/stringproperty.h>         // for StringProperty
#include <inviwo/core/properties/buttongroupproperty.h>
#include <inviwo/core/datastructures/geometry/mesh.h>       // for Mesh
#include <inviwo/core/util/glmvec.h>                        // for ivec2
#include <inviwo/core/util/staticstring.h>                  // for operator+
#include <modules/opengl/rendering/texturequadrenderer.h>   // for TextureQuadRenderer
#include <modules/opengl/shader/shader.h>                   // for Shader
#include <modules/plotting/properties/axisproperty.h>       // for AxisProperty
#include <modules/plotting/properties/axisstyleproperty.h>  // for AxisStyleProperty
#include <modules/plottinggl/utils/axisrenderer.h>          // for AxisRenderer

#include <functional>   // for __base
#include <string>       // for operator==, operator+
#include <string_view>  // for operator==
#include <tuple>        // for tuple
#include <vector>       // for operator!=, vector, operator==

namespace inviwo {

namespace plot {

class IVW_MODULE_PLOTTINGGL_API ColorScaleLegend : public Processor {
public:
    ColorScaleLegend();
    virtual ~ColorScaleLegend() = default;

    virtual void initializeResources() override;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    enum class BackgroundStyle {
        NoBackground,
        SolidColor,
        CheckerBoard,
        CheckerboardAndOpaque,
        Opaque
    };
    enum class LabelType { String, Data, Custom };
    enum class Placement {
        OutsideLeft,
        OutsideTop,
        OutsideRight,
        OutsideBottom,
        InsideLeft,
        InsideTop,
        InsideRight,
        InsideBottom,
    };
    std::tuple<ivec2, ivec2, ivec2, ivec2> getPositions(ivec2 dim) const;

    std::vector<ButtonGroupProperty::Button> buttons();
    void setPlacement(Placement placement);

    ImageInport inport_;
    VolumeInport volumeInport_;
    ImageOutport outport_;

    BoolProperty enabled_;
    IsoTFProperty isotfComposite_;

    // position properties
    CompositeProperty positioning_;
    ButtonGroupProperty legendPresets_;
    FloatVec2Property position_;
    IntProperty margin_;
    IntVec2Property legendSize_;

    // style customization properties
    AxisStyleProperty axisStyle_;
    OptionProperty<LabelType> labelType_;
    StringProperty title_;
    OptionProperty<BackgroundStyle> backgroundStyle_;
    FloatProperty checkerBoardSize_;
    FloatVec4Property bgColor_;
    IntProperty borderWidth_;
    BoolCompositeProperty isovalues_;
    FloatProperty triSize_;

    // shader variables
    TextureQuadRenderer textureRenderer_;
    Shader shader_;
    Shader isoValueShader_;

    // axis properties
    AxisProperty axis_;
    AxisRenderer axisRenderer_;

    Mesh isovalueMesh_;
};

}  // namespace plot

}  // namespace inviwo
