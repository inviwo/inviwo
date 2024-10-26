/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <modules/vectorfieldvisualizationgl/vectorfieldvisualizationglmoduledefine.h>

#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/geometry/typedmesh.h>
#include <inviwo/core/ports/layerport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/staticstring.h>

#include <functional>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo {

class IVW_MODULE_VECTORFIELDVISUALIZATIONGL_API HedgeHog2D : public Processor {

    enum class GlyphType { Arrow, Quiver };

public:
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    HedgeHog2D();
    virtual ~HedgeHog2D();

    virtual void process() override;

private:
    void adjustVisibilites();
    vec4 getColor(const dvec2& velocity);
    void createArrow(BasicMesh& mesh, IndexBufferRAM& index, float x, float y, float dx, float dy,
                     const dvec2& velocity);
    void createQuiver(BasicMesh& mesh, IndexBufferRAM& index, float x, float y, float dx, float dy,
                      const dvec2& velocity);

    LayerInport inport_;
    MeshOutport mesh_;

    FloatProperty glyphScale_;
    IntVec2Property numberOfGlyphs_;
    BoolProperty jitter_;
    OptionProperty<GlyphType> glyphType_;

    FloatVec4Property color_;

    CompositeProperty arrowSettings_;
    FloatProperty arrowBaseWidth_;
    FloatProperty arrowHookWidth_;
    FloatProperty arrowHeadRatio_;

    CompositeProperty quiverSettings_;
    FloatProperty quiverHookWidth_;
    FloatProperty quiverHeadRatio_;

    std::random_device rd_;
    std::mt19937 mt_;
};

}  // namespace inviwo
