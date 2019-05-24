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

#ifndef IVW_IMAGECHANNELREPLICATE_H
#define IVW_IMAGECHANNELREPLICATE_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>

namespace inviwo {
/** \docpage{org.inviwo.ImageChannelReplicate, Image Channel Replicate}
 * ![](org.inviwo.ImageChannelReplicate.png?classIdentifier=org.inviwo.ImageChannelReplicate)
 * Replicates the selected image channel and creates a 4 channel texture with the selected alpha value.
 *
 * ### Inports
 *   * __inputImage__ Input image
 *
 * ### Outports
 *   * __outputImage__ Output Image
 *
 * ### Properties
 *   * __alpha__ Alpha value in the final texture
 */

/**
 * \class ImageChannelReplicate
 *
 * \brief Replicates the selected image channel and creates a 4 channel texture with the selected alpha value.
 */
class IVW_MODULE_BASEGL_API ImageChannelReplicate : public ImageGLProcessor {
public:
    ImageChannelReplicate();
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void preProcess(TextureUnitContainer &cont) override;

private:
    FloatProperty alpha_; //!< alpha value [0,1] of the texture
};

}  // namespace inviwo

#endif  // IVW_IMAGECHANNELREPLICATE_H
