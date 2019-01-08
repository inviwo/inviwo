/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_IMAGEMAPPING_H
#define IVW_IMAGEMAPPING_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.ImageMapping, Image Mapping}
 * Maps the input image to an output image with the help of a transfer function.
 * ![](org.inviwo.ImageMapping.png?classIdentifier=org.inviwo.ImageMapping)
 *
 * ### Inports
 *   * __ImageInport__ The input image.
 *
 * ### Outports
 *   * __ImageOutport__ The output image.
 *
 * ### Properties
 *   * __Transfer Function__ The transfer function used for mapping input to output values
 *                           including the alpha channel.
 */

/*! \class ImageMapping
 *
 * \brief Maps the input image to an output with the help of a transfer function.
 */
class IVW_MODULE_BASEGL_API ImageMapping : public ImageGLProcessor {
public:
    ImageMapping();
    ~ImageMapping();
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void preProcess(TextureUnitContainer &cont) override;
    virtual void afterInportChanged() override;

private:
    TransferFunctionProperty transferFunction_;
};

}  // namespace inviwo

#endif  // IVW_IMAGEMAPPING_H
