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

#ifndef IVW_IMAGEFILTER_H
#define IVW_IMAGEFILTER_H

#include <modules/postprocessing/postprocessingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>
#include <modules/basegl/algorithm/imageconvolution.h>

namespace inviwo {

/** \docpage{org.inviwo.ImageFilter, Image Filter}
 * ![](org.inviwo.ImageFilter.png?classIdentifier=org.inviwo.ImageFilter)
 *
 * ### Inports
 *   * __ImageInport__ Input image.
 *
 * ### Outports
 *   * __ImageOutport__ Output image.
 *
 * ### Properties
 *   * __Enable__ Turn filter on/off.
 */

/**
 * \class ImageFilter
 * \brief Applies a kernel to the input image.
 */
class IVW_MODULE_POSTPROCESSING_API ImageFilter : public Processor {
public:
    ImageFilter();
    virtual ~ImageFilter() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    // Ports
    ImageInport inport_;
    ImageOutport outport_;

    // Properties
    BoolProperty enable_;
    OptionPropertyInt filter_;
    IntProperty passes_;

    // Members
    ImageConvolution convolution_;
    const std::array<std::vector<float>, 10> kernels_ = {
        std::vector<float>{-1, -1, -1, -1, 9, -1, -1, -1, -1},  // Strong sharpen
        std::vector<float>{0, -1, 0, -1, 5, -1, 0, -1, 0},      // Light sharpen
        std::vector<float>{.0625, .125, .0625, .125, .25, .125, .0625, .125, .0625},  // Blur
        std::vector<float>{-2, -1, 0, -1, 1, 1, 0, 1, 2},                             // Emboss
        std::vector<float>{-1, -1, -1, -1, 8, -1, -1, -1, -1},                        // Outline
        std::vector<float>{-1, -2, -1, 0, 0, 0, 1, 2, 1},                             // Sobel h
        std::vector<float>{-1, 0, 1, -2, 0, 2, -1, 0, 1},                             // Sobel v
        std::vector<float>{0, -1, 0, -1, 4, -1, 0, -1, 0},      // Laplacian excl diagonals
        std::vector<float>{-1, -1, -1, -1, 8, -1, -1, -1, -1},  // Laplacian incl diagonals
        std::vector<float>{0,  0,  -1, 0,  0,  0,  -1, -2, -1, 0,  -1, -2, 16,
                           -2, -1, 0,  -1, -2, -1, 0,  0,  0,  -1, 0,  0}  // Laplacian of Gaussian
    };
};

}  // namespace inviwo

#endif  // IVW_IMAGEFILTER_H
