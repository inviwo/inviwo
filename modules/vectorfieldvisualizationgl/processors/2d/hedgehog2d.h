/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_HEDGEHOG2D_H
#define IVW_HEDGEHOG2D_H

#include <modules/vectorfieldvisualizationgl/vectorfieldvisualizationglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <random>


namespace inviwo {

/**
 * \class HedgeHog2D
 *
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 *
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_VECTORFIELDVISUALIZATIONGL_API HedgeHog2D : public Processor { 

    enum class GlyphType {
        Arrow,
        Quiver 
    };

public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    HedgeHog2D();
    virtual ~HedgeHog2D();

    virtual void process() override;

private:
    ImageInport vectorFeild_;
    MeshOutport mesh_;

    FloatProperty glyphScale_;
    IntVec2Property numberOfGlyphs_;
    BoolProperty jitter_;
    TemplateOptionProperty<GlyphType> glyphType_;

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

    void adjustVisibilites();

    vec4 getColor(const dvec2 &velocity);

    void createArrow(BasicMesh &mesh, IndexBufferRAM &index, float x, float y, float dx, float dy, const dvec2 &velocity);
    void createQuiver(BasicMesh &mesh, IndexBufferRAM &index, float x, float y, float dx, float dy, const dvec2 &velocity);
};

} // namespace

#endif // IVW_HEDGEHOG2D_H

