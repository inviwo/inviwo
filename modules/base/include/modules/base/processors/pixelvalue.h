/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_PIXELVALUE_H
#define IVW_PIXELVALUE_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/eventproperty.h>

namespace inviwo {

/** docpage{org.inviwo.PixelValue, Pixel Value}
 * ![](org.inviwo.PixelValue.png?classIdentifier=org.inviwo.PixelValue)
 * Read the pixel value under the mouse of the image that is passed through the processor
 *
 * ### Inport
 *   * __inport___ Input image
 *
 * ### Outport
 *   * __outport__ Output image, pass through of input image
 *
 * ### Properties
 *   * __Pixel Value__ The pixel value under the mouse of the first color layer
 *   * __Pixel Value (as string)__ As a string.
 *   * __Normalized Pixel Value__ Normalized to [0,1]
 *   * __Picking Value__ The pixel value under the mouse of the picking layer
 *   * __Picking Value (as string)__ As a string.
 *   * __Depth Value__ The depth value under the mouse of the depth layer
 *   * __Depth Value (as string)__ As a string.
 *   * __Coordinates__ The mouse coordinates in the image.
 *
 */
class IVW_MODULE_BASE_API PixelValue : public Processor {
public:
    PixelValue();
    virtual ~PixelValue() = default;

    virtual void process() override;

    void mouseMoveEvent(Event* theevent);

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    ImageInport inport_;
    ImageOutport outport_;

    IntSize2Property coordinates_;
    std::vector<DoubleVec4Property> pixelValues_;
    std::vector<FloatVec4Property> pixelValuesNormalized_;
    DoubleVec4Property pickingValue_;
    DoubleProperty depthValue_;

    std::vector<StringProperty> pixelStrValues_;
    StringProperty pickingStrValue_;
    StringProperty depthStrValue_;

    EventProperty mouseMove_;
};

}  // namespace inviwo

#endif  // IVW_PIXELVALUE_H
