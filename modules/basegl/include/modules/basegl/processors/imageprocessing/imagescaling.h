/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>       // for IVW_MODULE_BASEGL_API

#include <inviwo/core/ports/imageport.h>             // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>     // for BoolProperty
#include <inviwo/core/properties/optionproperty.h>   // for OptionPropertyDouble
#include <inviwo/core/properties/ordinalproperty.h>  // for DoubleProperty, IntSize2Property
#include <inviwo/core/util/glmvec.h>                 // for size2_t

namespace inviwo {
class Deserializer;
class Event;
class Outport;

/** \docpage{org.inviwo.ImageScaling, Image Scaling}
 * ![](org.inviwo.ImageScaling.png?classIdentifier=org.inviwo.ImageScaling)
 * This processor provides functionality for up-scaling or down-scaling an image with respect to the
 * size of the input image. Alternatively, an absolut size can be set.
 *
 * ### Inports
 *   * __Source Image__ The mixed image
 *
 * ### Outports
 *   * __Output Image__ The scaled image
 *
 * ### Properties
 *   * __Enabled__   Enables or disables scaling of the input image
 *   * __Scaling Factor__
 *   * __Custom Factor__   custom scaling factor is used if scaling factor is "Custom"
 *   * __Absolute Size__   specific size is used if scaling factor is "Absolute"
 */

/**
 * \class ImageScaling
 * \brief Processor for up-scaling or down-scaling an image. Modifies the resize events by scaling
 * them before they are propagated further.
 */
class IVW_MODULE_BASEGL_API ImageScaling : public Processor {
public:
    ImageScaling();
    virtual ~ImageScaling() = default;

    virtual void process() override;

    virtual void propagateEvent(Event*, Outport* source) override;

    virtual void deserialize(Deserializer& d) override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    size2_t calcInputImageSize() const;
    bool resizeInports();

    ImageInport inport_;
    ImageOutport outport_;

    BoolProperty enabled_;
    OptionPropertyDouble scalingFactor_;  //<! if negative, use custom scaling factor
    DoubleProperty customFactor_;
    IntSize2Property absoluteSize_;

    size2_t lastValidOutputSize_ = {0u, 0u};
    bool deserializing_ = false;
};

}  // namespace inviwo
