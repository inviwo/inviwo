/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_PIXELTOBUFFERPROCESSOR_H
#define IVW_PIXELTOBUFFERPROCESSOR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/interactionhandler.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/bufferport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/boolproperty.h>

namespace inviwo {

//class OnClickInteractionHandler : public InteractionHandler {
//public:
//    OnClickInteractionHandler();
//    virtual ~OnClickInteractionHandler();
//
//
//};

class IVW_MODULE_BASE_API PixelToBufferProcessor : public Processor, public InteractionHandler { 
public:
    PixelToBufferProcessor();
    virtual ~PixelToBufferProcessor(){}
	
	InviwoProcessorInfo();
    void setPixelToCollectFrom(const ivec2& xy);
    void clearOutput();
    virtual void invokeEvent(Event* event);
protected:
    virtual void process();
    void inportChanged();
    void handleInteractionEventsChanged();
private:
    ImageInport inport_;
    BufferOutport pixelValues_;


    IntVec2Property fromPixel_;
    IntProperty channel_;
    ButtonProperty clearValues_;
    BoolProperty handleInteractionEvents_; ///< Enable or disable pixel picking

    Buffer_FLOAT64 values_;
};

} // namespace

#endif // IVW_PIXELTOBUFFERPROCESSOR_H

