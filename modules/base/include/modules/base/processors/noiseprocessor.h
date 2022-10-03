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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/imageport.h>               // for ImageOutport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>       // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>  // for CompositeProperty
#include <inviwo/core/properties/minmaxproperty.h>     // for FloatMinMaxProperty, IntMinMaxProp...
#include <inviwo/core/properties/optionproperty.h>     // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>    // for IntProperty, IntSizeTProperty, Flo...
#include <inviwo/core/util/staticstring.h>             // for operator+

#include <functional>   // for __base
#include <random>       // for mt19937, random_device
#include <string>       // for operator==, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, operator==

namespace inviwo {

/**
 * \brief A processor to generate a noise image
 */
class IVW_MODULE_BASE_API NoiseProcessor : public Processor {
    enum class NoiseType { Random, Perlin, PoissonDisk, HaltonSequence };

public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    NoiseProcessor();
    virtual ~NoiseProcessor();

    virtual void process() override;

protected:
    ImageOutport noise_;

    IntSize2Property size_;           ///< Size of the output image.
    OptionProperty<NoiseType> type_;  ///< Witch type of noise to generate.
    FloatMinMaxProperty range_;       ///< The min/max values of the output values (default: [0 1]).
    IntMinMaxProperty levels_;   ///< Numbers of levels used in the generation of the Perlin noise
    FloatProperty persistence_;  ///< Controls the sharpens in Perlin noise

    IntProperty poissonDotsAlongX_;  ///< Average number of points along the x-axis.
    IntProperty poissonMaxPoints_;   ///< Maximum number of output points (total).

    IntSizeTProperty haltonNumPoints_;
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
