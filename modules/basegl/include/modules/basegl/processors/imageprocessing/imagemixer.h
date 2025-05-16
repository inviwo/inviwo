/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/ports/imageport.h>             // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>     // for BoolProperty
#include <inviwo/core/properties/optionproperty.h>   // for OptionPropertyInt
#include <inviwo/core/properties/ordinalproperty.h>  // for FloatProperty
#include <modules/opengl/shader/shader.h>            // for Shader

namespace inviwo {

namespace BlendModes {
enum Mode {
    Mix,           //!< f(a,b) = a * (1 - alpha) + b * alpha
    Over,          //!< f(a,b) = b, b over a, regular front-to-back blending
    Multiply,      //!< f(a,b) = a * b
    Screen,        //!< f(a,b) = 1 - (1 - a) * (1 - b)
    Overlay,       //!< f(a,b) = 2 * a *b, if a < 0.5,   f(a,b) = 1 - 2(1 - a)(1 - b), otherwise
                   //!< (combination of Multiply and Screen)
    HardLight,     //!< Overlay where a and b are swapped
    Divide,        //!< f(a,b) = a/b
    Addition,      //!< f(a,b) = a + b, clamped to [0,1]
    Subtraction,   //!< f(a,b) = a - b, clamped to [0,1]
    Difference,    //!< f(a,b) = |a - b|
    DarkenOnly,    //!< f(a,b) = min(a, b), per component
    BrightenOnly,  //!< f(a,b) = max(a, b), per component
};
}

/**
 * @brief Mixes two input images according to the chosen blend mode.
 */
class IVW_MODULE_BASEGL_API ImageMixer : public Processor {
public:
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    ImageMixer();
    virtual ~ImageMixer();

    virtual void process() override;

protected:
    virtual void initializeResources() override;

private:
    ImageInport inport0_;  //!< first input image
    ImageInport inport1_;  //!< second input image

    ImageOutport outport_;  //!< output image

    OptionPropertyInt blendingMode_;  //!< blend mode from BlendModes::Mode
    FloatProperty weight_;            //!< weighting factor
    BoolProperty clamp_;
    Shader shader_;
};

}  // namespace inviwo
