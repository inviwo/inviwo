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

#include <inviwo/core/ports/datainport.h>                                     // for DataI...
#include <inviwo/core/ports/meshport.h>                                       // for MeshO...
#include <inviwo/core/ports/outportiterable.h>                                // for Outpo...
#include <inviwo/core/ports/volumeport.h>                                     // for Volum...
#include <inviwo/core/processors/processor.h>                                 // for Proce...
#include <inviwo/core/processors/processorinfo.h>                             // for Proce...
#include <inviwo/core/properties/boolproperty.h>                              // for BoolP...
#include <inviwo/core/properties/optionproperty.h>                            // for Optio...
#include <inviwo/core/properties/ordinalproperty.h>                           // for Float...
#include <inviwo/core/properties/stringproperty.h>                            // for Strin...
#include <inviwo/core/properties/transferfunctionproperty.h>                  // for Trans...
#include <inviwo/core/util/glmvec.h>                                          // for vec4
#include <inviwo/core/util/spatial4dsampler.h>                                // for Spati...
#include <inviwo/core/util/staticstring.h>                                    // for opera...
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>  // for Integ...
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>            // for SeedP...
#include <modules/vectorfieldvisualization/properties/pathlineproperties.h>   // for PathL...

#include <functional>     // for __base
#include <string>         // for opera...
#include <string_view>    // for opera...
#include <unordered_map>  // for opera...
#include <vector>         // for vector

#include <fmt/core.h>      // for format
#include <glm/gtx/io.hpp>  // for opera...

namespace inviwo {
class Deserializer;

class IVW_MODULE_VECTORFIELDVISUALIZATION_API PathLinesDeprecated : public Processor {
public:
    enum class ColoringMethod { Velocity, Timestamp, ColorPort };
    PathLinesDeprecated();
    virtual ~PathLinesDeprecated() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

    virtual void deserialize(Deserializer& d) override;

private:
    DataInport<Spatial4DSampler<3, double>> sampler_;
    SeedPointsInport<3> seedPoints_;
    DataInport<std::vector<vec4>> colors_;
    VolumeSequenceInport volume_;
    IntegralLineSetOutport lines_;

    MeshOutport linesStripsMesh_;

    PathLineProperties pathLineProperties_;

    TransferFunctionProperty tf_;
    OptionProperty<ColoringMethod> coloringMethod_;
    FloatProperty velocityScale_;
    StringProperty maxVelocity_;

    BoolProperty allowLooping_;
};

}  // namespace inviwo
