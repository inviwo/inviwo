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

#include <modules/base/basemoduledefine.h>                           // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/image/image.h>                  // for Image
#include <inviwo/core/ports/imageport.h>                             // for ImageOutport
#include <inviwo/core/ports/outportiterable.h>                       // for OutportIterable
#include <inviwo/core/processors/processorinfo.h>                    // for ProcessorInfo
#include <inviwo/core/util/glmvec.h>                                 // for uvec3
#include <modules/base/processors/vectorelementselectorprocessor.h>  // for VectorElementSelecto...

#include <string>                                                    // for string
#include <vector>                                                    // for vector

#include <fmt/core.h>                                                // for format, format_to
#include <glm/vec3.hpp>                                              // for operator+

namespace inviwo {

/** \docpage{org.inviwo.ImageTimeStepSelector, Image Sequence Element Selector}
 * ![](org.inviwo.ImageTimeStepSelector.png?classIdentifier=org.inviwo.ImageTimeStepSelector)
 *
 * Select a specific volume out of a sequence of images
 *
 * ### Inport
 *   * __inport__ Sequence of images
 * ### Outport
 *   * __outport__ Selected image
 *
 * ### Properties
 *   * __Step__ The image sequence index to extract
 */

class IVW_MODULE_BASE_API ImageSequenceElementSelectorProcessor
    : public VectorElementSelectorProcessor<Image, ImageOutport> {
public:
    ImageSequenceElementSelectorProcessor();
    virtual ~ImageSequenceElementSelectorProcessor() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
};
}  // namespace inviwo
