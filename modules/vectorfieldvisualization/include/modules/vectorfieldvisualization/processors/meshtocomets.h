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

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>

#include <modules/vectorfieldvisualization/datastructures/integralline.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/ports/meshport.h>

#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>
#include <inviwo/core/properties/boolproperty.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>

namespace inviwo {

class IVW_MODULE_VECTORFIELDVISUALIZATION_API MeshToComets : public Processor {
public:
    enum class TimeBy { PositionZ, VertexIndex };
    enum class ColorBy { None, OriginalColor, Color, Normal, Texture };
    // enum class BrushBy { Nothing, LineIndex, VectorPosition };

    MeshToComets();
    virtual ~MeshToComets() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    class ColorByBuffer : public CompositeProperty {
    public:
        ColorByBuffer();
        TemplateOptionProperty<ColorBy> colorBy_;
        DoubleMinMaxProperty scaleBy_;
        BoolProperty loopTF_;
        DoubleProperty minValue_;
        DoubleProperty maxValue_;
        TransferFunctionProperty tf_;
    };

private:
    MeshInport lines_;
    BrushingAndLinkingInport brushingList_;
    // DataInport<std::vector<vec4>> colors_;
    MeshOutport mesh_;
    MeshOutport pointMesh_;

    ColorByBuffer colorByBuffer_;
    BoolProperty filter_;
    TemplateOptionProperty<TimeBy> timeBy_;

    // BoolCompositeProperty timeBasedFiltering_;
    // DoubleMinMaxProperty minMaxT_;
    // ButtonProperty setFromData_;

    // TemplateOptionProperty<Output> output_;

    // FloatProperty ribbonWidth_;

    FloatVec4Property selectedColor_;

    // Comet related properties.
    FloatProperty time_;
    FloatProperty cometLength_;
    BoolProperty scaleWidth_;
    DoubleMinMaxProperty sphereScale_;

    bool isFiltered(size_t idx) const;
    bool isSelected(size_t idx) const;
};

}  // namespace inviwo
