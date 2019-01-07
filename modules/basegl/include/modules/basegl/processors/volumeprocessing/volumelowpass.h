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

#ifndef IVW_VOLUMELOWPASS_H
#define IVW_VOLUMELOWPASS_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeLowPass, Volume Low Pass}
 * ![](org.inviwo.VolumeLowPass.png?classIdentifier=org.inviwo.VolumeLowPass)
 * Applies a low pass filter on the input volume.
 *
 * ### Inports
 *   * __inputVolume__ Input volume
 *
 * ### Outports
 *   * __outputVolume__ Output volume
 *
 * ### Properties
 *   * __Kernel Size__ Size of the applied low pass filter
 *   * __Use Gaussian Weights__ Toggles between a Gaussian kernel and a box filter
 *   * __Sigma__ Sigma used by the Gaussian kernel
 *
 */
class IVW_MODULE_BASEGL_API VolumeLowPass : public VolumeGLProcessor {
public:
    VolumeLowPass();
    virtual ~VolumeLowPass();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void preProcess(TextureUnitContainer &cont) override;
    virtual void postProcess() override;

    virtual void initializeResources() override;

private:
    IntProperty kernelSize_;

    BoolCompositeProperty useGaussianWeights_;
    FloatProperty sigma_;
    BoolProperty updateDataRange_;
};

}  // namespace inviwo

#endif  // IVW_VOLUMELOWPASS_H
