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

#ifndef IVW_IMAGECHANNELSELECT_H
#define IVW_IMAGECHANNELSELECT_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {
/** \docpage{org.inviwo.ImageChannelSelect, Image Channel Select}
 * ![](org.inviwo.ImageChannelSelect.png?classIdentifier=org.inviwo.ImageChannelSelect)
 * Creates a single channel texture with the selected channel from the drop down menu.
 *
 * ### Inports
 *   * __inputImage__ Input image
 *
 * ### Outports
 *   * __outputImage__ Single Channel Output Image
 *
 * ### Properties
 *   * __channelSelector__ Selection between RED, GREEN, BLUE, and ALPHA
 */

/**
 * \class ImageChannelSelect
 *
 * \brief Creates a single channel texture with the selected channel from the drop down menu.
 */
class IVW_MODULE_BASEGL_API ImageChannelSelect : public ImageGLProcessor {
public:
    ImageChannelSelect();
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    OptionPropertyString channelSelector_; //!< Selection between RED, GREEN, BLUE, and ALPHA
};

}  // namespace inviwo

#endif // IVW_IMAGECHANNELSELECT_H
