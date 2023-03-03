/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <inviwo/core/ports/datainport.h>                                      // for DataI...
#include <inviwo/core/ports/meshport.h>                                        // for MeshO...
#include <inviwo/core/ports/outportiterable.h>                                 // for Outpo...
#include <inviwo/core/ports/volumeport.h>                                      // for Volum...
#include <inviwo/core/processors/processor.h>                                  // for Proce...
#include <inviwo/core/processors/processorinfo.h>                              // for Proce...
#include <inviwo/core/properties/boolproperty.h>                               // for BoolP...
#include <inviwo/core/properties/ordinalproperty.h>                            // for Float...
#include <inviwo/core/properties/stringproperty.h>                             // for Strin...
#include <inviwo/core/properties/transferfunctionproperty.h>                   // for Trans...
#include <inviwo/core/util/glmvec.h>                                           // for uvec3
#include <inviwo/core/util/spatialsampler.h>                                   // for Spati...
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>   // for Integ...
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>             // for SeedP...
#include <modules/vectorfieldvisualization/properties/streamlineproperties.h>  // for Strea...

#include <string>         // for string
#include <unordered_map>  // for opera...
#include <vector>         // for vector

#include <fmt/core.h>      // for format
#include <glm/mat4x4.hpp>  // for opera...
#include <glm/vec3.hpp>    // for opera...
#include <glm/vec4.hpp>    // for opera...

namespace inviwo {

class IVW_MODULE_VECTORFIELDVISUALIZATION_API StreamLinesDeprecated : public Processor {
public:
    // friend void vectorvis::convertProcessor(StreamLinesDeprecated*);
    StreamLinesDeprecated();
    virtual ~StreamLinesDeprecated();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

protected:
    DataInport<SpatialSampler<3, 3, double>> sampler_;
    SeedPoints3DInport seedPoints_;
    VolumeInport volume_;
    MeshOutport linesStripsMesh_;
    IntegralLineSetOutport lines_;

    StreamLineProperties streamLineProperties_;

    TransferFunctionProperty tf_;
    FloatProperty velocityScale_;
    StringProperty maxVelocity_;

    BoolProperty useMutliThreading_;
};

}  // namespace inviwo
