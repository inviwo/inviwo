/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/ports/bufferport.h>                               // for BufferOutport
#include <inviwo/core/ports/imageport.h>                                // for ImageInport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>                      // for ButtonProperty
#include <inviwo/core/properties/ordinalproperty.h>                     // for IntProperty, IntV...
#include <inviwo/core/util/glmvec.h>                                    // for ivec2

#include <memory>         // for unique_ptr, share...
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set

namespace inviwo {
class Event;

/** \docpage{org.inviwo.PixelToBufferProcessor, Pixel to buffer}
 * ![](org.inviwo.PixelToBufferProcessor.png?classIdentifier=org.inviwo.PixelToBufferProcessor)
 *
 * ...
 *
 * ### Inports
 *   * __input__ ...
 *
 * ### Outports
 *   * __pixelValues__ ...
 *
 * ### Properties
 *   * __From pixel__ ...
 *   * __Clear collected values__ ...
 *   * __Channel__ ...
 *   * __Enable picking__ ...
 *
 */
class IVW_MODULE_BASE_API PixelToBufferProcessor : public Processor {
public:
    using PosBuffer = Buffer<double>;
    PixelToBufferProcessor();
    virtual ~PixelToBufferProcessor() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    void setPixelToCollectFrom(const ivec2& xy);
    void clearOutput();
    virtual void invokeEvent(Event* event) override;

protected:
    virtual void process() override;
    void inportChanged();
    void handleInteractionEventsChanged();

private:
    ImageInport inport_;
    BufferOutport pixelValues_;

    IntVec2Property fromPixel_;
    IntProperty channel_;
    ButtonProperty clearValues_;
    BoolProperty handleInteractionEvents_;  ///< Enable or disable pixel picking

    std::shared_ptr<PosBuffer> values_;
};

}  // namespace inviwo
