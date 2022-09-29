/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <modules/postprocessing/postprocessingmoduledefine.h>           // for IVW_MODULE_POSTP...

#include <inviwo/core/processors/processorinfo.h>                        // for ProcessorInfo
#include <inviwo/core/properties/ordinalproperty.h>                      // for FloatProperty
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>  // for ImageGLProcessor

namespace inviwo {
class TextureUnitContainer;

/** \docpage{org.inviwo.ImageOpacity, Image Opacity}
 * ![](org.inviwo.ImageOpacity.png?classIdentifier=org.inviwo.ImageOpacity)
 * Controls an image's opacity
 *
 *     out.rgb = in.rgb
 *     out.a = alpha
 *
 * ### Inports
 *   * __ImageInport__ Input image.
 *
 * ### Outports
 *   * __ImageOutport__ Output image.
 *
 * ### Properties
 *   * __Alpha__ Controls the opacity.
 */

/**
 * \class ImageOpacity
 * \brief Controls an image's opacity
 */
class IVW_MODULE_POSTPROCESSING_API ImageOpacity : public ImageGLProcessor {
public:
    ImageOpacity();
    virtual ~ImageOpacity() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void preProcess(TextureUnitContainer& cont) override;

private:
    FloatProperty alpha_;
};

}  // namespace inviwo
