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

#ifndef IVW_SEEDPOINTGENERATOR_H
#define IVW_SEEDPOINTGENERATOR_H

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/util/timer.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <random>

namespace inviwo {

/** \docpage{org.inviwo.SeedPointGenerator, Seed Point Generator}
 * ![](org.inviwo.SeedPointGenerator.png?classIdentifier=org.inviwo.SeedPointGenerator)
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __<Inport1>__ <description>.
 *
 * ### Outports
 *   * __<Outport1>__ <description>.
 * 
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */


/**
 * \class SeedPointGenerator
 * \brief <brief description> 
 * <Detailed description from a developer prespective>
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API SeedPointGenerator : public Processor { 
public:
    InviwoProcessorInfo();
    SeedPointGenerator();
    virtual ~SeedPointGenerator() = default;
     
    virtual void process() override;


    void onGeneratorChange();

private:
    DataOutport<std::vector<vec3>> seedPoints_;

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

} // namespace

#endif // IVW_SEEDPOINTGENERATOR_H

