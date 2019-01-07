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

#ifndef IVW_IMAGEBINARY_H
#define IVW_IMAGEBINARY_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>
#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.ImageBinary, Image Binary}
 * ![](org.inviwo.ImageBinary.png?classIdentifier=org.inviwo.ImageBinary)
 * Computes a binary image of the input image using a threshold. The output
 * will contain "0" for all values below the threshold and "1" otherwise.
 *
 * ### Inports
 *   * __inputImage__ Input image
 *
 * ### Outports
 *   * __outputImage__ Binary output image
 *
 * ### Properties
 *   * __Threshold__ Threshold used for the binarization of the input image
 */

/**
 * \class ImageBinary
 *
 * \brief computes a binary image of the input image using a threshold.
 */
class IVW_MODULE_BASEGL_API ImageBinary : public ImageGLProcessor {
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    ImageBinary();
    virtual ~ImageBinary() {}

protected:
    virtual void preProcess(TextureUnitContainer &cont) override;

    FloatProperty threshold_;

private:
};

}  // namespace inviwo

#endif  // IVW_IMAGEBINARY_H
