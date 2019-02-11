/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_NUMPYMANDELBROT_H
#define IVW_NUMPYMANDELBROT_H

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/python3/pythonscript.h>
#include <inviwo/core/properties/minmaxproperty.h>

namespace inviwo {

//(\f$ Z_{n+1} = Z_{n}^p + C \f$)

/** \docpage{org.inviwo.NumpyMandelbrot, NumPy Mandelbrot}
 * ![](org.inviwo.NumpyMandelbrot.png?classIdentifier=org.inviwo.NumpyMandelbrot)
 *
 * En example processor illustrating how Python can be used to create images.
 * Processor creates an "empty" image and passes it along to a python script. This python calculates
 * for each pixel if that pixel is part of the Mandelbrot set.
 * (https://en.wikipedia.org/wiki/Mandelbrot_set) and updates the pixel in the image accordingly.
 * See the Example workspace in File/Python3/mandelbrot.inv
 *
 *
 * ### Outports
 *   * __outport__ The final image, suitable for color mapping using the ImageMapping Processor
 *
 * ### Properties
 *   * __Size__ The size of the output image
 *   * __Real bounds__ Range of the real values used in the initial complex number mapped to the
 * x-axis
 *   * __Imaginary bounds__ Range of the imaginary values used in the initial complex number mapped
 * to the y-axis
 *   * __power__ To what power to raise Z in each iteration.
 *   * __Iterations__ Maximum iterations to use for testing membership.
 *
 */
class IVW_MODULE_PYTHON3_API NumpyMandelbrot : public Processor {
public:
    NumpyMandelbrot();
    virtual ~NumpyMandelbrot() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    ImageOutport outport_;

    IntSize2Property size_;
    FloatMinMaxProperty realBounds_;
    FloatMinMaxProperty imaginaryBound_;
    FloatProperty power_;
    IntSizeTProperty iterations_;

    PythonScriptDisk script_;
};

}  // namespace inviwo

#endif  // IVW_NUMPYMANDELBROT_H
