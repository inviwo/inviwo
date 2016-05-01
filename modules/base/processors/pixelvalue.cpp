/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/base/processors/pixelvalue.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/interaction/events/mouseevent.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PixelValue::processorInfo_{
    "org.inviwo.PixelValue",      // Class identifier
    "Pixel Value",                // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo PixelValue::getProcessorInfo() const {
    return processorInfo_;
}

PixelValue::PixelValue()
    : Processor()
    , inport_("inport_",true)
    , outport_("outport",false)
    , coordinates_("Coordinates", "Coordinates", size2_t(0), size2_t(std::numeric_limits<size_t>::lowest()), size2_t(std::numeric_limits<size_t>::max()), size2_t(1), InvalidationLevel::Valid, PropertySemantics::Text)
    , pixelValue_("pixelValue", "Pixel Value", dvec4(0), dvec4(std::numeric_limits<double>::lowest()), dvec4(std::numeric_limits<double>::max()), dvec4(std::numeric_limits<double>::epsilon()), InvalidationLevel::Valid, PropertySemantics::Text)
    , pickingValue_("pickingValue", "Picking Value", dvec4(0), dvec4(std::numeric_limits<double>::lowest()), dvec4(std::numeric_limits<double>::max()), dvec4(std::numeric_limits<double>::epsilon()), InvalidationLevel::Valid, PropertySemantics::Text)
    , depthValue_("depthValue_", "Depth Value", 0.0, std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), std::numeric_limits<double>::epsilon(), InvalidationLevel::Valid, PropertySemantics::Text)
    , pixelStrValue_("pixelStrValue", "Pixel Value (as string)", "", InvalidationLevel::Valid)
    , pickingStrValue_("pickingStrValue", "Picking Value (as string)", "", InvalidationLevel::Valid)
    , depthStrValue_("depthStrValue", "Depth Value (as string)", "", InvalidationLevel::Valid)

    , mouseMove_("mouseMove", "Mouse Move",
        new MouseEvent(MouseEvent::MOUSE_BUTTON_ANY_AND_NONE, InteractionEvent::MODIFIER_NONE,
            MouseEvent::MOUSE_STATE_MOVE),
        new Action(this, &PixelValue::mouseMoveEvent))

{
    
    addPort(inport_);
    addPort(outport_);

    addProperty(pixelValue_);
    addProperty(pixelStrValue_);
    addProperty(pickingValue_);
    addProperty(pickingStrValue_);
    addProperty(depthValue_);
    addProperty(depthStrValue_);
    addProperty(coordinates_);
    addProperty(mouseMove_);

}
    
void PixelValue::process() {
    outport_.setData(inport_.getData());
}

void PixelValue::mouseMoveEvent(Event* theevent) {
    if (auto mouseEvent = dynamic_cast<MouseEvent*>(theevent)) {
        auto img = inport_.getData();
        auto dims = img->getDimensions();
        size2_t pos = static_cast<size2_t>(mouseEvent->posNormalized() * vec2(dims));
        coordinates_.set(pos);
        pixelValue_.set(img->getColorLayer()->getRepresentation<LayerRAM>()->getAsDVec4(pos));
        pickingValue_.set(img->getPickingLayer()->getRepresentation<LayerRAM>()->getAsDVec4(pos));
        depthValue_.set(img->getDepthLayer()->getRepresentation<LayerRAM>()->getAsDouble(pos));


        pixelStrValue_.set(toString(img->getColorLayer()->getRepresentation<LayerRAM>()->getAsDVec4(pos)));
        pickingStrValue_.set(toString(img->getPickingLayer()->getRepresentation<LayerRAM>()->getAsDVec4(pos)));
        depthStrValue_.set(toString(img->getDepthLayer()->getRepresentation<LayerRAM>()->getAsDouble(pos)));
    }
    
    


}

} // namespace

