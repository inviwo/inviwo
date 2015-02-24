/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "volumeslice.h"
#include <inviwo/core/datastructures/volume/volumeramslice.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>

namespace inviwo {

ProcessorClassIdentifier(VolumeSlice, "org.inviwo.VolumeSlice");
ProcessorDisplayName(VolumeSlice, "Volume Slice");
ProcessorTags(VolumeSlice, Tags::CPU);
ProcessorCategory(VolumeSlice, "Volume Operation");
ProcessorCodeState(VolumeSlice, CODE_STATE_STABLE);

VolumeSlice::VolumeSlice()
    : Processor()
    , inport_("volume.inport")
    , outport_("image.outport")
    , sliceAlongAxis_("sliceAxis", "Slice along axis")
    , sliceNumber_("sliceNumber", "Slice Number", 4, 1, 8) 
    , handleInteractionEvents_("handleEvents", "Handle interaction events", true,
    VALID)
    , mouseShiftSlice_("mouseShiftSlice", "Mouse Slice Shift",
    new MouseEvent(MouseEvent::MOUSE_BUTTON_NONE, InteractionEvent::MODIFIER_NONE,
    MouseEvent::MOUSE_STATE_WHEEL),
    new Action(this, &VolumeSlice::eventShiftSlice))
    , stepSliceUp_("stepSliceUp", "Key Slice Up", 
    new KeyboardEvent('W', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
    new Action(this, &VolumeSlice::eventStepSliceUp))
    , stepSliceDown_("stepSliceDown", "Key Slice Down", 
    new KeyboardEvent('S', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
    new Action(this, &VolumeSlice::eventStepSliceDown))
    , gestureShiftSlice_("gestureShiftSlice", "Gesture Slice Shift",
    new GestureEvent(GestureEvent::PAN, GestureEvent::GESTURE_STATE_ANY, 3),
    new Action(this, &VolumeSlice::eventGestureShiftSlice)) {
    addPort(inport_);
    addPort(outport_);
    sliceAlongAxis_.addOption("x", "X axis", CoordinateEnums::X);
    sliceAlongAxis_.addOption("y", "Y axis", CoordinateEnums::Y);
    sliceAlongAxis_.addOption("z", "Z axis", CoordinateEnums::Z);
    sliceAlongAxis_.setSelectedIndex(0);
    sliceAlongAxis_.setCurrentStateAsDefault();
    addProperty(sliceAlongAxis_);
    addProperty(sliceNumber_);

    addProperty(handleInteractionEvents_);

    mouseShiftSlice_.setVisible(false);
    mouseShiftSlice_.setCurrentStateAsDefault();
    addProperty(mouseShiftSlice_);

    addProperty(stepSliceUp_);
    addProperty(stepSliceDown_);

    gestureShiftSlice_.setVisible(false);
    gestureShiftSlice_.setCurrentStateAsDefault();
    addProperty(gestureShiftSlice_);
}

VolumeSlice::~VolumeSlice() {}

void VolumeSlice::initialize() { Processor::initialize(); }

void VolumeSlice::deinitialize() { Processor::deinitialize(); }

void VolumeSlice::invokeInteractionEvent(Event* event) {
    if (!handleInteractionEvents_) return;
    Processor::invokeInteractionEvent(event);
}

void VolumeSlice::shiftSlice(int shift) {
    int newSlice = sliceNumber_.get() + shift;
    if (newSlice >= sliceNumber_.getMinValue() && newSlice <= sliceNumber_.getMaxValue())
        sliceNumber_.set(newSlice);
}

void VolumeSlice::process() {
    const VolumeRAM* vol = inport_.getData()->getRepresentation<VolumeRAM>();

    const ivec3 dims(vol->getDimensions());

    switch (sliceAlongAxis_.get()) {
        case CoordinateEnums::X:
            if (dims.x != sliceNumber_.getMaxValue()) {
                sliceNumber_.setMaxValue(dims.x);
                sliceNumber_.set(dims.x / 2);
            }
            break;
        case CoordinateEnums::Y:
            if (dims.y != sliceNumber_.getMaxValue()) {
                sliceNumber_.setMaxValue(dims.y);
                sliceNumber_.set(dims.y / 2);
            }
            break;
        case CoordinateEnums::Z:
            if (dims.z != sliceNumber_.getMaxValue()) {
                sliceNumber_.setMaxValue(dims.z);
                sliceNumber_.set(dims.z / 2);
            }
            break;
    }

    LayerRAM* sliceImage = VolumeRAMSlice::apply(
        vol, static_cast<CoordinateEnums::CartesianCoordinateAxis>(sliceAlongAxis_.get()),
        static_cast<unsigned int>(sliceNumber_.get() - 1));

    Image* outImage =
        new Image(sliceImage->getDimensions(), sliceImage->getDataFormat());
    outImage->getColorLayer()->addRepresentation(sliceImage);

    outport_.setData(outImage);
}

void VolumeSlice::eventShiftSlice(Event* event){
    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);
    int steps = mouseEvent->wheelSteps();
    shiftSlice(steps);
}

void VolumeSlice::eventStepSliceUp(Event*){
    shiftSlice(1);
}

void VolumeSlice::eventStepSliceDown(Event*){
    shiftSlice(-1);
}

void VolumeSlice::eventGestureShiftSlice(Event* event){
    GestureEvent* gestureEvent = static_cast<GestureEvent*>(event);
    if (gestureEvent->deltaPos().y < 0)
        shiftSlice(1);
    else if (gestureEvent->deltaPos().y > 0)
        shiftSlice(-1);
}


}  // inviwo namespace
