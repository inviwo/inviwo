/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_IMAGEMIXER_H
#define IVW_IMAGEMIXER_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/baseoptionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {

class Shader;

namespace BlendModes {
    enum Mode {
        Mix, //!< f(a,b) = a * (1 - alpha) + b * alpha
        Over, //!< f(a,b) = b, b over a, regular front-to-back blending
        Multiply, //!< f(a,b) = a * b
        Screen, //!< f(a,b) = 1 - (1 - a) * (1 - b)
        Overlay, //!< f(a,b) = 2 * a *b, if a < 0.5,   f(a,b) = 1 - 2(1 - a)(1 - b), otherwise (combination of Multiply and Screen)
        HardLight, //!< Overlay where a and b are swapped
        Divide, //!< f(a,b) = a/b
        Addition, //!< f(a,b) = a + b, clamped to [0,1]
        Subtraction, //!< f(a,b) = a - b, clamped to [0,1]
        Difference, //!< f(a,b) = |a - b|
        DarkenOnly, //!< f(a,b) = min(a, b), per component
        BrightenOnly, //!< f(a,b) = max(a, b), per component
    };
}

/** \docpage{org.inviwo.ImageMixer, Image Mixer}
 * Mixes two input images according to the chosen blend mode. 
 * ![](imagemixer.png)
 * The result is blended with the input image A according to 
 * <tt>mix(a,b) = a * (1 - weight) + f(a,b) * weight</tt>,
 * where f(a,b) is given by the blend mode.
 *
 * Supported blend modes for determining <tt>f(a,b)</tt>
 * <table>
 *   <tr><td>Over</td><td><tt>f(a,b) = b over a</tt>, regular front-to-back blending</td></tr>
 *   <tr><td>Multiply</td><td><tt>f(a,b) = a * b</tt></td></tr>
 *   <tr><td>Screen</td><td><tt>f(a,b) = 1 - (1 - a) * (1 - b)</tt></td></tr>
 *   <tr><td>Overlay</td><td><tt>f(a,b) = 2 * a *b, if a < 0.5</tt>, and</td></tr>
 *      <tr><td></td><td><tt>f(a,b) = 1 - 2(1 - a)(1 - b)</tt>, otherwise (combination of Multiply and Screen)</td></tr>
 *   <tr><td>HardLight</td><td>Overlay where a and b are swapped</td></tr>
 *   <tr><td>Divide</td><td><tt>f(a,b) = a/b</tt></td></tr>
 *   <tr><td>Addition</td><td><tt>f(a,b) = a + b</tt>, clamped to [0,1]</td></tr>
 *   <tr><td>Subtraction</td><td><tt>f(a,b) = a - b</tt>, clamped to [0,1]</td></tr>
 *   <tr><td>Difference</td><td><tt>f(a,b) = |a - b|</tt></td></tr>
 *   <tr><td>DarkenOnly</td><td><tt>f(a,b) = min(a, b)</tt>, per component</td></tr>
 *   <tr><td>BrightenOnly</td><td><tt>f(a,b) = max(a, b)</tt>, per component</td></tr>
 * </table>
 * 
 * ### Inports
 *   * __ImageInport__ Input image A.
 *   * __ImageInport__ Input image B.
 *
 * ### Outports
 *   * __ImageOutport__ The output image. <tt>mix(a,b) = a * (1 - weight) + f(a,b) * weight</tt>
 * 
 * ### Properties
 *   * __Blend Mode__ Blend mode used for mixing the input images.
 *   * __Weight__     Weighting factor for mixing the blending result with input image A. 
 */

/*! \class ImageMixer
 *
 * \brief Mixes two input images according to the chosen blend mode.
 */
class IVW_MODULE_BASEGL_API ImageMixer : public Processor {
public:
    ImageMixer();
    ~ImageMixer();
    InviwoProcessorInfo();

    virtual void initialize();
    virtual void deinitialize();

    virtual void process();

protected:
    virtual void initializeResources();
    
private:
    ImageInport inport0_; //!< first input image
    ImageInport inport1_; //!< second input image

    ImageOutport outport_; //!< output image

    OptionPropertyInt blendingMode_; //!< blend mode from BlendModes::Mode
    FloatProperty weight_; //!< weighting factor
    Shader* shader_;
};

} // namespace

#endif // IVW_IMAGEMIXER_H
