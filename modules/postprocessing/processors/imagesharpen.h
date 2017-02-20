/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_IMAGESHARPEN_H
#define IVW_IMAGESHARPEN_H

#include <modules/postprocessing/postprocessingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>

namespace inviwo {

/** \docpage{org.inviwo.ImageSharpen, Image Sharpen}
 * ![](org.inviwo.ImageSharpen.png?classIdentifier=org.inviwo.ImageSharpen)
 * Applies a laplacian filter to the input image.
 * Two kernels are available:
 *
 *     Filter1: -1  -1  -1
 *              -1   8  -1
 *              -1  -1  -1
 *
 *     Filter2:  0  -1   0
 *              -1   4  -1
 *               0  -1   0
 *
 * ### Inports
 *   * __ImageInport__ Input image.
 *
 * ### Outports
 *   * __ImageOutport__ Output image.
 * 
 * ### Properties
 *   * __Sharpen__ Turn filter on/off.
 */

/**
 * \class ImageSharpen
 * \brief Applies a laplacian filter to the input image.
 */
class IVW_MODULE_POSTPROCESSING_API ImageSharpen : public ImageGLProcessor { 
public:
    ImageSharpen();
    virtual ~ImageSharpen() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void preProcess(TextureUnitContainer &cont) override;

private:
    IntProperty passes_;
    BoolProperty sharpen_;
    OptionPropertyInt filter_;
};

} // namespace

#endif // IVW_IMAGESHARPEN_H

