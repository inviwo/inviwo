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

#ifndef IVW_IMAGEHIGHPASS_H
#define IVW_IMAGEHIGHPASS_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.ImageHighPass, Image High Pass}
 * ![](org.inviwo.ImageHighPass.png?classIdentifier=org.inviwo.ImageHighPass)
 * Applies a high pass filter on the input image.
 *
 * ### Inports
 *   * __inputImage__ Input image
 *
 * ### Outports
 *   * __outputImage__ Filtered input image
 *
 * ### Properties
 *   * __Kernel Size__ Size of the applied high pass filter
 *   * __Sharpen__ Toggles additional sharpening operation
 */

/**
 * \class ImageHighPass
 *
 * \brief Applies a high pass filter on the input image.
 */
class IVW_MODULE_BASEGL_API ImageHighPass : public ImageGLProcessor {
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    ImageHighPass();
    virtual ~ImageHighPass() = default;

protected:
    virtual void preProcess(TextureUnitContainer &cont) override;

private:
    IntProperty kernelSize_;
    BoolProperty sharpen_;
};

}  // namespace inviwo

#endif  // IVW_IMAGEHIGHPASS_H
