/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>  // for IVW_M...

#include <inviwo/core/processors/processor.h>                                       // for Proce...
#include <inviwo/core/processors/processorinfo.h>                                   // for Proce...
#include <inviwo/core/properties/boolproperty.h>                                    // for BoolP...
#include <inviwo/core/properties/compositeproperty.h>                               // for Compo...
#include <inviwo/core/properties/minmaxproperty.h>                                  // for Float...
#include <inviwo/core/properties/optionproperty.h>                                  // for Optio...
#include <inviwo/core/properties/ordinalproperty.h>                                 // for Float...
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>                  // for SeedP...

#include <random>                                                                   // for mt19937

namespace inviwo {

class IVW_MODULE_VECTORFIELDVISUALIZATION_API SeedPointGenerator : public Processor {
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    SeedPointGenerator();
    virtual ~SeedPointGenerator() = default;

    virtual void process() override;

    void onGeneratorChange();

private:
    SeedPoints3DOutport seedPoints_;

    CompositeProperty lineGroup_;
    CompositeProperty planeGroup_;
    CompositeProperty sphereGroup_;

    IntProperty numberOfPoints_;

    IntVec2Property planeResolution_;
    FloatVec3Property planeOrigin_;
    FloatVec3Property planeE1_;
    FloatVec3Property planeE2_;

    FloatVec3Property sphereCenter_;
    FloatMinMaxProperty sphereRadius_;

    FloatVec3Property lineStart_;
    FloatVec3Property lineEnd_;

    OptionPropertyInt generator_;

    CompositeProperty randomness_;
    BoolProperty useSameSeed_;
    IntProperty seed_;

    void randomPoints();
    void planePoints();
    void linePoints();
    void spherePoints();

    std::random_device rd_;
    std::mt19937 mt_;
};

}  // namespace inviwo
