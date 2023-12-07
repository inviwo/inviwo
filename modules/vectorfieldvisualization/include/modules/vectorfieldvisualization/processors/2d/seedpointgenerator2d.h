/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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

#include <inviwo/core/processors/processor.h>                       // for Proce...
#include <inviwo/core/processors/processorinfo.h>                   // for Proce...
#include <inviwo/core/properties/boolproperty.h>                    // for BoolP...
#include <inviwo/core/properties/compositeproperty.h>               // for Compo...
#include <inviwo/core/properties/optionproperty.h>                  // for Optio...
#include <inviwo/core/properties/ordinalproperty.h>                 // for IntSi...
#include <inviwo/core/util/staticstring.h>                          // for opera...
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>  // for SeedP...

#include <functional>   // for __base
#include <random>       // for mt19937
#include <string>       // for opera...
#include <string_view>  // for opera...
#include <vector>       // for opera...

namespace inviwo {

class IVW_MODULE_VECTORFIELDVISUALIZATION_API SeedPointGenerator2D : public Processor {
public:
    enum class Generator { Random, HaltonSequence };
    enum class SamplingDomain { FullDomain, Line, Rectangle, Disk };

    SeedPointGenerator2D();
    virtual ~SeedPointGenerator2D() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    SeedPoints3DOutport seeds_;

    CompositeProperty samplingDomain_;
    OptionProperty<SamplingDomain> domain_;
    FloatVec2Property position_;
    FloatVec2Property extent_;
    FloatVec2Property endpoint_;
    FloatProperty radius_;

    OptionProperty<Generator> generator_;

    IntSizeTProperty numPoints_;
    IntSizeTProperty haltonXBase_;
    IntSizeTProperty haltonYBase_;

    CompositeProperty randomness_;
    BoolProperty useSameSeed_;  ///< Use the same seed for each call to process.
    IntProperty seed_;          ///<  The seed used to initialize the random sequence

private:
    std::random_device rd_;
    std::mt19937 mt_;
};

}  // namespace inviwo
