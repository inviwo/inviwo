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

#ifndef IVW_GRAYSCALE_CL_H
#define IVW_GRAYSCALE_CL_H

#include <modules/basecl/baseclmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>

#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/kernelowner.h>

namespace inviwo {
/** \docpage{org.inviwo.GrayscaleCL, Image Grayscale}
 * ![](org.inviwo.GrayscaleCL.png?classIdentifier=org.inviwo.GrayscaleCL)
 * Turns an image into grayscale using the following operation:
 *      out = 0.2989*red + 0.5870*green + 0.1140*blue;
 *
 * ### Inports
 *   * __ImageInport__ The input color image.
 *
 * ### Outports
 *   * __ImageOutport__ The output grayscale image.
 *
 * ### Properties
 *   * __Use OpenGL sharing__ Share input and output image with OpenGL
 */

/**
 * \brief Turns an image into grayscale.
 *
 */
class IVW_MODULE_BASECL_API GrayscaleCLProcessor : public Processor, public ProcessorKernelOwner {
public:
    GrayscaleCLProcessor();
    ~GrayscaleCLProcessor() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

private:
    ImageInport input_;
    ImageOutport outport_;

    BoolProperty useGLSharing_;

    cl::Kernel* kernel_;
};

}  // namespace inviwo

#endif  // IVW_GRAYSCALE_CL_H
