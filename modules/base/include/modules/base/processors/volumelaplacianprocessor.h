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

#include <modules/base/basemoduledefine.h>                      // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/volumeport.h>                       // for VolumeInport, VolumeOutport
#include <inviwo/core/processors/poolprocessor.h>               // for PoolProcessor
#include <inviwo/core/processors/processorinfo.h>               // for ProcessorInfo
#include <inviwo/core/properties/optionproperty.h>              // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>             // for DoubleProperty
#include <inviwo/core/util/staticstring.h>                      // for operator+
#include <modules/base/algorithm/volume/volumelaplacian.h>      // for VolumeLaplacianPostProces...
#include <modules/base/properties/volumeinformationproperty.h>  // for VolumeInformationProperty

#include <functional>                                           // for __base
#include <string>                                               // for operator==, string
#include <string_view>                                          // for operator==
#include <vector>                                               // for operator!=, vector, opera...

namespace inviwo {

/** \docpage{org.inviwo.VolumeLaplacianProcessor, Volume Laplacian Processor}
 * ![](org.inviwo.VolumeLaplacianProcessor.png?classIdentifier=org.inviwo.VolumeLaplacianProcessor)
 * Computes the Laplacian of the input volume.
 *
 * ### Inports
 *   * __inputVolume__ Input volume
 *
 * ### Outports
 *   * __outputVolume__ Output volume
 *
 */
class IVW_MODULE_BASE_API VolumeLaplacianProcessor : public PoolProcessor {
public:
    VolumeLaplacianProcessor();
    virtual ~VolumeLaplacianProcessor() = default;

    static const ProcessorInfo processorInfo_;
    virtual const ProcessorInfo getProcessorInfo() const override;

    virtual void process() override;

private:
    VolumeInport inport_;
    VolumeOutport outport_;

    OptionProperty<util::VolumeLaplacianPostProcessing> postProcessing_;
    DoubleProperty scale_;

    VolumeInformationProperty inVolume_;
    VolumeInformationProperty outVolume_;
};

}  // namespace inviwo
