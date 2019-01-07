/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#ifndef IVW_VOLUMEDIFF_H
#define IVW_VOLUMEDIFF_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeDiff, Volume Difference}
 * ![](org.inviwo.VolumeDiff.png?classIdentifier=org.inviwo.VolumeDiff)
 * Computes the difference between two volumes by subtracting the second volume from the first one.
 *
 * ### Inports
 *   * __inputVolume__ Input volume 1
 *   * __volume2__ Input volume 2
 *
 * ### Outports
 *   * __outputVolume__ Difference volume corresponding to <tt>(volume 1 - volume 2 + 1.0)
 * / 2.0</tt>
 *
 */
class IVW_MODULE_BASEGL_API VolumeDiff : public VolumeGLProcessor {
public:
    VolumeDiff();
    virtual ~VolumeDiff() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    VolumeInport vol2_;
    virtual void preProcess(TextureUnitContainer &cont) override;
};

}  // namespace inviwo

#endif  // IVW_VOLUMEDIFF_H
