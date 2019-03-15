/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_IMAGEHUESATURATIONLUMINANCE_H
#define IVW_IMAGEHUESATURATIONLUMINANCE_H

#include <modules/postprocessing/postprocessingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>
#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.ImageHueSaturationLuminance, Image Hue Saturation Luminance}
 * ![](org.inviwo.ImageHueSaturationLuminance.png?classIdentifier=org.inviwo.ImageHueSaturationLuminance)
 * Controls hue, saturation and luminance of an image.
 * Input image is in RGB color space. The colors are then converted into HSL and the following
 * manipulations are performed:
 *
 *    hsl.r = mod(hsl.r + hue, 1.0);
 *    hsl.g = clamp(hsl.g + sat, 0.0, 1.0);
 *    hsl.b = clamp(hsl.b + lum, 0.0, 1.0);
 *
 * Finally, the image is transformed back into RGB.
 *
 * ### Inports
 *   * __ImageInport__ Input image.
 *
 * ### Outports
 *   * __ImageOutport__ Output image.
 *
 * ### Properties
 *   * __Hue__ Controls hue.
 *   * __Saturation__ Controls saturation.
 *   * __Luminance__ Controls luminance.
 */

/**
 * \class ImageHueSaturationLuminance
 * \brief Controls hue, saturation and luminance of an image.
 */
class IVW_MODULE_POSTPROCESSING_API ImageHueSaturationLuminance : public ImageGLProcessor {
public:
    ImageHueSaturationLuminance();
    virtual ~ImageHueSaturationLuminance() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void preProcess(TextureUnitContainer &cont) override;

private:
    FloatProperty hue_;
    FloatProperty saturation_;
    FloatProperty luminance_;
};

}  // namespace inviwo

#endif  // IVW_IMAGEHUESATURATIONLUMINANCE_H
