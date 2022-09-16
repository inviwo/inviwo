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

#include <inviwo/core/ports/imageport.h>                                            // for Image...
#include <inviwo/core/processors/processor.h>                                       // for Proce...
#include <inviwo/core/processors/processorinfo.h>                                   // for Proce...
#include <inviwo/core/properties/boolproperty.h>                                    // for BoolP...
#include <inviwo/core/properties/compositeproperty.h>                               // for Compo...
#include <inviwo/core/properties/ordinalproperty.h>                                 // for IntPr...
#include <inviwo/core/util/glmvec.h>                                                // for dvec2
#include <modules/base/properties/gaussianproperty.h>                               // for Gauss...

#include <random>                                                                   // for unifo...
#include <utility>                                                                  // for pair
#include <vector>                                                                   // for vector

namespace inviwo {
class Deserializer;
class Serializer;

class IVW_MODULE_VECTORFIELDVISUALIZATION_API RBFVectorFieldGenerator2D : public Processor {
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    RBFVectorFieldGenerator2D();
    virtual ~RBFVectorFieldGenerator2D();

    virtual void process() override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    ImageOutport vectorField_;

    IntVec2Property size_;
    IntProperty seeds_;

    CompositeProperty randomness_;
    BoolProperty useSameSeed_;
    IntProperty seed_;
    FloatProperty shape_;
    Gaussian1DProperty gaussian_;

    std::random_device rd_;
    std::mt19937 mt_;

    dvec2 randomVector();

    void createSamples();

    std::uniform_real_distribution<double> theta_;
    std::uniform_real_distribution<double> x_;

    std::vector<std::pair<dvec2, dvec2>> samples_;
};

}  // namespace inviwo
