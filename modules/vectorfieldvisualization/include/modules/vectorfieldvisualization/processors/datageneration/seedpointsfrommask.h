/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volume.h>                               // for Volume
#include <inviwo/core/ports/datainport.h>                                           // for DataI...
#include <inviwo/core/ports/outportiterable.h>                                      // for Outpo...
#include <inviwo/core/processors/processor.h>                                       // for Proce...
#include <inviwo/core/processors/processorinfo.h>                                   // for Proce...
#include <inviwo/core/properties/boolproperty.h>                                    // for BoolP...
#include <inviwo/core/properties/compositeproperty.h>                               // for Compo...
#include <inviwo/core/properties/ordinalproperty.h>                                 // for IntPr...
#include <inviwo/core/util/glmvec.h>                                                // for uvec3
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>                  // for SeedP...

#include <random>                                                                   // for mt19937
#include <string>                                                                   // for string
#include <unordered_map>                                                            // for opera...
#include <vector>                                                                   // for vector

#include <fmt/core.h>                                                               // for forma...

namespace inviwo {

class IVW_MODULE_VECTORFIELDVISUALIZATION_API SeedPointsFromMask : public Processor {
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    SeedPointsFromMask();
    virtual ~SeedPointsFromMask() {}

protected:
    virtual void process() override;

    DataInport<Volume, 0> volumes_;
    SeedPoints3DOutport seedPoints_;

    DoubleProperty threshold_;

    BoolProperty enableSuperSample_;
    IntProperty superSample_;

    CompositeProperty randomness_;
    BoolProperty useSameSeed_;
    IntProperty seed_;
    BoolProperty transformToWorld_;

private:
    std::mt19937 mt_;
    std::uniform_real_distribution<float> dis_;
};

}  // namespace inviwo
