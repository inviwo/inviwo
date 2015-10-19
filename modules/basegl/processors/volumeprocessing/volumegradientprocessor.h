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

#ifndef IVW_GRADIENTVOLUMEPROCESSOR_H
#define IVW_GRADIENTVOLUMEPROCESSOR_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>


namespace inviwo {

/**
 * \class VolumeGradientProcessor
 *
 * \brief Computes the gradient of a 3D scalar field and stores it in a vec3 volume.
 */
/** \docpage{org.inviwo.VolumeGradient, Volume Gradient}
 * ![](org.inviwo.VolumeGradient.png?classIdentifier=org.inviwo.VolumeGradient)
 *
 * ...
 * 
 * 
 * 
 * ### Properties
 *   * __Render Channel__ ...
 *   * __Stored voxel values in 4th channel__ ...
 *
 */
class IVW_MODULE_BASEGL_API VolumeGradientProcessor : public VolumeGLProcessor  { 
public:
    VolumeGradientProcessor();
    virtual ~VolumeGradientProcessor();
    InviwoProcessorInfo();

protected:
    virtual void preProcess(TextureUnitContainer &cont) override;
    virtual void postProcess() override;

    virtual void initializeResources() override;

private:
    void onVolumeChange();

    OptionPropertyInt channel_;
    BoolProperty dataInChannel4_;
};

} // namespace

#endif // IVW_GRADIENTVOLUMEPROCESSOR_H

