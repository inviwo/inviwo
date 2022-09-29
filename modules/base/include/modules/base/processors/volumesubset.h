/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>          // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/volumeport.h>           // for VolumeInport, VolumeOutport
#include <inviwo/core/processors/processor.h>       // for Processor
#include <inviwo/core/processors/processorinfo.h>   // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>    // for BoolProperty
#include <inviwo/core/properties/minmaxproperty.h>  // for IntSizeTMinMaxProperty
#include <inviwo/core/util/glmvec.h>                // for size3_t

namespace inviwo {

/** \docpage{org.inviwo.VolumeSubset, Volume Subset}
 * ![](org.inviwo.VolumeSubset.png?classIdentifier=org.inviwo.VolumeSubset)
 *
 * ...
 *
 * ### Inports
 *   * __volume.inport__ ...
 *
 * ### Outports
 *   * __volume.outport__ ...
 *
 * ### Properties
 *   * __Y Slices__ ...
 *   * __Enable Operation__ ...
 *   * __X Slices__ ...
 *   * __Z Slices__ ...
 *   * __Adjust Basis and Offset__ ...
 *
 */
class IVW_MODULE_BASE_API VolumeSubset : public Processor {
public:
    VolumeSubset();
    ~VolumeSubset();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    VolumeInport inport_;
    VolumeOutport outport_;

    BoolProperty enabled_;
    BoolProperty adjustBasisAndOffset_;

    IntSizeTMinMaxProperty rangeX_;
    IntSizeTMinMaxProperty rangeY_;
    IntSizeTMinMaxProperty rangeZ_;

    size3_t dims_;
};

}  // namespace inviwo
