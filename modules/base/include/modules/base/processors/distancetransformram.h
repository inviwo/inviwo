/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
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

#include <inviwo/core/ports/volumeport.h>            // for VolumeInport, VolumeOutport
#include <inviwo/core/processors/poolprocessor.h>    // for PoolProcessor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>     // for BoolProperty
#include <inviwo/core/properties/minmaxproperty.h>   // for DoubleMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>   // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>  // for DoubleProperty, IntProperty, IntSize...
#include <inviwo/core/util/staticstring.h>           // for operator+

#include <functional>   // for __base
#include <iosfwd>       // for ostream
#include <string>       // for operator==, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, operator==

namespace inviwo {

class IVW_MODULE_BASE_API DistanceTransformRAM : public PoolProcessor {
public:
    enum class DataRangeMode { Diagonal, MinMax, Custom };

    DistanceTransformRAM();
    virtual ~DistanceTransformRAM();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

private:
    VolumeInport volumePort_;
    VolumeOutport outport_;

    DoubleProperty threshold_;
    BoolProperty flip_;
    BoolProperty normalize_;
    DoubleProperty resultDistScale_;  // scaling factor for distances
    BoolProperty resultSquaredDist_;  // determines whether output uses squared euclidean distances
    BoolProperty uniformUpsampling_;
    IntProperty upsampleFactorUniform_;    // uniform upscaling of the output field
    IntSize3Property upsampleFactorVec3_;  // non-uniform upscaling of the output field

    DoubleMinMaxProperty dataRangeOutput_;
    OptionProperty<DataRangeMode> dataRangeMode_;
    DoubleMinMaxProperty customDataRange_;
};

IVW_MODULE_BASE_API std::ostream& operator<<(std::ostream& ss,
                                             DistanceTransformRAM::DataRangeMode m);

}  // namespace inviwo
