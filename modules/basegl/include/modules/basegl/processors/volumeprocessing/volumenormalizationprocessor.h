/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/basegl/algorithm/volumenormalization.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeNormalizationProcessor, Volume Normalization Processor}
 * ![](org.inviwo.VolumeNormalizationProcessor.png?classIdentifier=org.inviwo.VolumeNormalizationProcessor)
 *
 * Normalizes the selected channels of the input volume to range [0,1].
 * Note that this algorithm normalizes channels independently, it does not normalize a multi-channel
 * volume in terms of vector norms!
 *
 * ### Inputs
 *   * __Volume inport__ Input Volume
 *
 * ### Outports
 *   * __Volume outport__ Normalized volume (if so selected)
 *
 * ### Properties
 *   * __Channels__ Check the boxes for those channels you wish to normalize to range [0,1]
 */

/**
 * \class VolumeNormalizationProcessor
 *
 * Enables the usage of the %VolumeNormalization algorithm. For details about the algorithm,
 * please see VolumeNormalization.
 * Note that this algorithm normalizes channels independently, it does not normalize a multi-channel
 * volume in terms of vector norms!
 */
class IVW_MODULE_BASEGL_API VolumeNormalizationProcessor : public Processor {
public:
    VolumeNormalizationProcessor();
    virtual ~VolumeNormalizationProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport volumeInport_;
    VolumeOutport volumeOutport_;

    CompositeProperty channels_;
    BoolProperty normalizeChannel0_;
    BoolProperty normalizeChannel1_;
    BoolProperty normalizeChannel2_;
    BoolProperty normalizeChannel3_;

    VolumeNormalization volumeNormalization_;
};

}  // namespace inviwo
