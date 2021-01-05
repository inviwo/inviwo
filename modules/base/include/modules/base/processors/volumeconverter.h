/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <modules/base/properties/datarangeproperty.h>
#include <inviwo/core/ports/volumeport.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeConverter, Volume Converter}
 * ![](org.inviwo.VolumeConverter.png?classIdentifier=org.inviwo.VolumeConverter)
 * Converts the data type of a volume to a given output data format. The number of channels remains
 * unchanged.
 *
 * ### Inports
 *   * __inport__    volume input
 *
 * ### Outports
 *   * __outport__   converted input volume
 *
 * ### Properties
 *   * __Format__    data format of the output volume. The number of channels remains
 *                   the same. If identical to input data, no conversion will be performed.
 *   * __Use Data Range__   If enabled, the processor will utilize data mapping between different
 *                   integer formats. That is, each data value is normalized using the data range of
 *                   the input volume before being adjusted to the target format (using the range of
 *                   the data type). For example in a conversion from `uint8 [0 255]` to
 *                   `uint16 [0 65536]`, a value of `255` will be mapped to `65536`. If the target
 *                   format is floating point, then data values are only normalized.
 *                   Float formats are __not__ normalized!
 *
 */
class IVW_MODULE_BASE_API VolumeConverter : public Processor {
public:
    VolumeConverter();
    virtual ~VolumeConverter() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport inport_;
    VolumeOutport outport_;

    StringProperty inputFormat_;
    TemplateOptionProperty<DataFormatId> format_;
    BoolProperty enableDataMapping_;

    DataRangeProperty dataRange_;
    DoubleMinMaxProperty outputDataRange_;
};

}  // namespace inviwo
