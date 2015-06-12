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

#ifndef IVW_VOLUMEGRADIENTMAGNITUDEPROCESSOR_H
#define IVW_VOLUMEGRADIENTMAGNITUDEPROCESSOR_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/optionproperty.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>


namespace inviwo {

/**
 * \class VolumeGradientMagnitude
 *
 * \brief Computes the gradient magnitude of a 3D scalar field and outputs it as float volume.
 *
 * This processor internally computes the gradients of the given 3D scalar field and only
 * writes the magnitudes of the gradients to the outport. It yields the same results as
 * when combining a GradientVolumeProcessor and VectorMagnitudeProcessor. However, the 
 * intermediate gradient volume is never generated and, thus, this processor is more memory 
 * efficient.
 */
/** \docpage{org.inviwo.VolumeGradientMagnitude, Volume Gradient Magnitude}
 * ![](org.inviwo.VolumeGradientMagnitude.png?classIdentifier=org.inviwo.VolumeGradientMagnitude)
 *
 * ...
 * 
 * 
 * 
 * ### Properties
 *   * __Render Channel__ ...
 *
 */
class IVW_MODULE_BASEGL_API VolumeGradientMagnitude : public VolumeGLProcessor  { 
public:
    VolumeGradientMagnitude();
    virtual ~VolumeGradientMagnitude();
    InviwoProcessorInfo();

protected:
    virtual void preProcess();
    virtual void postProcess();

private:
    virtual void afterInportChanged();

    OptionPropertyInt channel_;
};

} // namespace

#endif // IVW_VOLUMEGRADIENTMAGNITUDEPROCESSOR_H
