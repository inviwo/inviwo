/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_IMAGECHANNELCOMBINE_H
#define IVW_IMAGECHANNELCOMBINE_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <modules/opengl/shader/shader.h>

namespace inviwo {
/** \docpage{org.inviwo.ImageChannelCombine, Image Channel Combine}
 * ![](org.inviwo.ImageChannelCombine.png?classIdentifier=org.inviwo.ImageChannelCombine)
 * Creates a 4 channel image with the connected inputs, from each of them the channel specified in
 * its property is used. If the optional alpha channel is not provided, the alpha property is used
 * to set the alpha value of the whole image
 *
 * ### Inports
 *   * __inport0__ Red input image
 *   * __inport1__ Green input image
 *   * __inport2__ Blue input image
 *   * __inport3__ Optional alpha input image
 *
 * ### Outports
 *   * __outport__ Output Image
 *
 * ### Properties
 *   * __Red Channel__ Channel of the 1st input image to use for thre red channel of output image.
 *   * __Green Channel__ Channel of the 2nd input image to use for thre green channel of output
 * image.
 *   * __Blue Channel__ Channel of the 3rd input image to use for thre blue channel of output image.
 *   * __Alpha Channel__ Channel of the 4th input image to use for thre alpha channel of output
 * image.
 *   * __Alpha__ Alpha value in the final texture if no alpha channel is connected to the optional
 * input port
 *
 */
class IVW_MODULE_BASEGL_API ImageChannelCombine : public Processor {
public:
    ImageChannelCombine();
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

private:
    ImageInport inport0_;  //!< red input image
    ImageInport inport1_;  //!< green input image
    ImageInport inport2_;  //!< blue input image
    ImageInport inport3_;  //!< optional alpha input image

    ImageOutport outport_;  //!< output image

    OptionPropertyInt rChannelSrc_;  //!< Channel of 1th input image for the red output channel
    OptionPropertyInt gChannelSrc_;  //!< Channel of 2nd input image for the green output channel
    OptionPropertyInt bChannelSrc_;  //!< Channel of 3rd input image for the blue output channel
    OptionPropertyInt aChannelSrc_;  //!< Channel of 4th input image for the alpha output channel

    FloatProperty alpha_;  //!< alpha value if no alpha channel is connected

    Shader shader_;
};

}  // namespace inviwo

#endif  // IVW_IMAGECHANNELCOMBINE_H
