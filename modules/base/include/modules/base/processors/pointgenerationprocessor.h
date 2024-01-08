/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/dataoutport.h>                 // for DataOutport
#include <inviwo/core/processors/processor.h>              // for Processor
#include <inviwo/core/processors/processorinfo.h>          // for ProcessorInfo
#include <inviwo/core/properties/boolcompositeproperty.h>  // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>           // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>        // for FloatVec3Property, IntSizeTPro...
#include <inviwo/core/util/glmvec.h>                       // for vec3
#include <inviwo/core/util/staticstring.h>                 // for operator+

#include <string>  // for operator+, string
#include <vector>  // for vector

#include <fmt/core.h>      // for format, basic_string_view
#include <fmt/format.h>    // for formatbuf<>::int_type, formatb...
#include <glm/gtx/io.hpp>  // for operator<<

namespace inviwo {

class IVW_MODULE_BASE_API Point3DGenerationProcessor : public Processor {
public:
    Point3DGenerationProcessor();
    virtual ~Point3DGenerationProcessor() override = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataOutport<std::vector<vec3>> outport_;

    struct Grid {
        FloatVec3Property a1;
        FloatVec3Property a2;
        FloatVec3Property a3;
        BoolProperty autoCenter;
        FloatVec3Property offset;
        IntSize3Property nPoints;
        FloatVec3Property jitter;
        IntSizeTProperty seed;
    };

    struct RandomBox {
        FloatVec3Property a1;
        FloatVec3Property a2;
        FloatVec3Property a3;
        BoolProperty autoCenter;
        FloatVec3Property offset;
        IntSizeTProperty nPoints;
        IntSizeTProperty seed;
    };

    struct RandomSphere {
        FloatVec3Property center;
        FloatVec2Property radius;
        IntSizeTProperty nPoints;
        IntSizeTProperty seed;
    };

    Grid grid_;
    RandomBox box_;
    RandomSphere sphere_;

    BoolCompositeProperty gridProps_;
    BoolCompositeProperty boxProps_;
    BoolCompositeProperty sphereProps_;
};

}  // namespace inviwo
