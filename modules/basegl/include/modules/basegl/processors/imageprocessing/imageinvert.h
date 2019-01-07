/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#ifndef IVW_IMAGEINVERT_H
#define IVW_IMAGEINVERT_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>

namespace inviwo {

/** \docpage{org.inviwo.ImageInvert, Image Invert}
 * Create the invert image of an input image. The alpha channel is not touched.
 * ![](org.inviwo.ImageInvert.png?classIdentifier=org.inviwo.ImageInvert)
 * This processor computes the inverted image as follows
 *
 *     out.rgb = 1.0 - in.rgb
 *     out.a = in.a
 *
 * The input range is assumed to be normalized, i.e. [0, 1].
 *
 * ### Inports
 *   * __ImageInport__ The input image.
 *
 * ### Outports
 *   * __ImageOutport__ The output image.
 */

/*! \class ImageInvert
 *
 * \brief Create the invert image of an input image. Alpha channel is not touched.
 *
 * This processor computes the inverted image by subtracting the rgb channels from 1.0
 * utilizing the ImageGLProcessor.
 * The input range is assumed to be normalized, i.e. [0, 1].
 */
class IVW_MODULE_BASEGL_API ImageInvert : public ImageGLProcessor {
public:
    ImageInvert();
    virtual ~ImageInvert();
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
};

}  // namespace inviwo

#endif  // IVW_IMAGEINVERT_H
