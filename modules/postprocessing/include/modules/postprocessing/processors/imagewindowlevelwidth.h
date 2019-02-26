/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#ifndef IVW_IMAGEWINDOWLEVELWIDTH_H
#define IVW_IMAGEWINDOWLEVELWIDTH_H

#include <modules/postprocessing/postprocessingmoduledefine.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/eventproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.ImageBrightnessContrast, Image Brightness Contrast}
 * ![](org.inviwo.ImageBrightnessContrast.png?classIdentifier=org.inviwo.ImageBrightnessContrast)
 * Controls brightness and contrast of an image.
 * The following manipulations are applied:
 *
 * ### Inports
 *   * __ImageInport__ Input image.
 *   * __VolumeInport__ Input volume.
 *
 * ### Outports
 *   * __ImageOutport__ Output image.
 *
 * ### Properties
 *   * __Level__ Controls level.
 *   * __Window__ Controls window.
 */

/**
 * \class ImageBrightnessContrast
 * \brief Controls brightness and contrast of an image.
 */
class IVW_MODULE_POSTPROCESSING_API ImageWindowLevelWidth : public ImageGLProcessor {
public:
    ImageWindowLevelWidth();
    virtual ~ImageWindowLevelWidth() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void preProcess(TextureUnitContainer &cont) override;

private:
    VolumeInport volume_;

    FloatProperty windowCenter_;
    FloatProperty windowWidth_;
    FloatProperty sensitivity_;

    EventProperty mouseEventWindowCenter_;
    EventProperty mouseEventWindowWidth_;

    void windowCenterCallback(Event* event);
    void windowWidthCallback(Event* event);

    vec2 lastMousePos_;
};

}  // namespace inviwo

#endif  // IVW_IMAGEWINDOWLEVELWIDTH_H
