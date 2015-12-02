/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_IMAGERESAMPLE_H
#define IVW_IMAGERESAMPLE_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

/*! \class ImageResample
 *
 * \brief Upscale/Downscale image according to parameters
 *
 */
/** \docpage{org.inviwo.ImageResample, Image Resample}
 * ![](org.inviwo.ImageResample.png?classIdentifier=org.inviwo.ImageResample)
 *
 * ...
 * 
 * 
 * 
 * ### Properties
 *   * __Dimension Source__ ...
 *   * __Outport dimensions__ ...
 *   * __Interpolation Type__ ...
 *
 */
class IVW_MODULE_BASEGL_API ImageResample : public ImageGLProcessor  { 
public:
    ImageResample();
    virtual ~ImageResample();
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void initializeResources() override;

protected:
    void interpolationTypeChanged();
    void dimensionChanged();
    void dimensionSourceChanged();

private:
    OptionPropertyInt interpolationType_;
    OptionPropertyInt dimensionSource_;
    IntVec2Property dimensions_;
};

} // namespace

#endif // IVW_IMAGERESAMPLE_H

