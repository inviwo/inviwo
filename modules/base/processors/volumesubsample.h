/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMESUBSAMPLE_H
#define IVW_VOLUMESUBSAMPLE_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/base/algorithm/volume/volumeramsubsample.h>
#include <inviwo/core/processors/activityindicator.h>

#include <future>

namespace inviwo {

/** \docpage{org.inviwo.VolumeSubsample, Volume Subsample}
 * ![](org.inviwo.VolumeSubsample.png?classIdentifier=org.inviwo.VolumeSubsample)
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
 *   * __Enable Operation__ ...
 *   * __Factors__ ...
 *
 */
class IVW_MODULE_BASE_API VolumeSubsample : public Processor, public ActivityIndicatorOwner {
public:
    InviwoProcessorInfo();

    VolumeSubsample();
    virtual ~VolumeSubsample() = default;

protected:
    virtual void process() override;

    virtual void invalidate(InvalidationLevel invalidationLevel,
                            Property* modifiedProperty = nullptr) override;

private:
    VolumeInport inport_;
    VolumeOutport outport_;

    BoolProperty enabled_;
    BoolProperty waitForCompletion_;
    IntVec3Property subSampleFactors_;

    std::future<std::shared_ptr<Volume>> result_;
    bool dirty_;
};
}

#endif  // IVW_VOLUMESUBSAMPLE_H
