/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>          // for IVW_MODULE_BASEGL_API

#include <inviwo/core/ports/imageport.h>                // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>           // for Processor
#include <inviwo/core/processors/processorinfo.h>       // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>        // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>     // for FloatProperty, IntProperty
#include <modules/basegl/algorithm/imageconvolution.h>  // for ImageConvolution

namespace inviwo {

/** \docpage{org.inviwo.ImageLowPass, Image Low Pass}
 * ![](org.inviwo.ImageLowPass.png?classIdentifier=org.inviwo.ImageLowPass)
 *
 * Applies a low pass filter on the input image.
 *
 *
 * ### Inports
 *   * __inputImage__ Input image.
 *
 * ### Outports
 *   * __outputImage__ Lowpass filtered image.
 *
 * ### Properties
 *   * __Kernel Size__ Size of the kernel to use.
 *   * __Use Gaussian weights__ Whether to use Gaussian weights or constant weights.
 *   * __Sigma__ Controls the shape of the Gaussian bell curve.
 */

/**
 * \class ImageLowPass
 *
 * \brief Applies a low pass filter on the input image using either constant weight or Gaussian
 * weights
 */
class IVW_MODULE_BASEGL_API ImageLowPass : public Processor {
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    ImageLowPass();
    virtual ~ImageLowPass() {}

protected:
    virtual void process() override;

private:
    ImageInport inport_;
    ImageOutport outport_;

    IntProperty kernelSize_;
    BoolProperty gaussian_;
    FloatProperty sigma_;

    ImageConvolution convolution_;
};

}  // namespace inviwo
