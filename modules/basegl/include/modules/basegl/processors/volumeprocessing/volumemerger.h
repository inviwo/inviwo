/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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

#ifndef IVW_VOLUMEMERGER_H
#define IVW_VOLUMEMERGER_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeMerger, Volume Merger}
 * ![](org.inviwo.VolumeMerger.png?classIdentifier=org.inviwo.VolumeMerger)
 * Merges up to four single-channel volumes into a single volume. If, for example,
 * input volumes 1 and 4 are given, the output volume will have 2 channels where the
 * first one contains volume 1 and the second one volume 4.
 *
 * ### Inports
 *   * __inputVolume__ Input volume 1
 *   * __volume2__ Input volume 2
 *   * __volume3__ Input volume 3
 *   * __volume4__ Input volume 4
 *
 * ### Outports
 *   * __outputVolume__ Merged volume
 */

/**
 * \class VolumeMerger
 * \brief merges up to four single-channel volumes into a single volume
 */
class IVW_MODULE_BASEGL_API VolumeMerger : public VolumeGLProcessor {
public:
    VolumeMerger();
    virtual ~VolumeMerger() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    VolumeInport vol2_;
    VolumeInport vol3_;
    VolumeInport vol4_;
    virtual void preProcess(TextureUnitContainer &cont) override;
};

}  // namespace inviwo

#endif  // IVW_VOLUMEMERGER_H
