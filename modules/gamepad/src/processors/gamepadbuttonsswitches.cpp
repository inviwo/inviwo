/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/gamepad/processors/gamepadbuttonsswitches.h>

#include <inviwo/gamepad/processors/gamepadinput.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GamepadButtonsSwitches::processorInfo_{
    "org.inviwo.GamepadButtonsSwitches",  // Class identifier
    "Gamepad Buttons & Switches",         // Display name
    "Gamepad",                            // Category
    CodeState::Experimental,              // Code state
    Tags::CPU,                            // Tags
};
const ProcessorInfo GamepadButtonsSwitches::getProcessorInfo() const { return processorInfo_; }

GamepadButtonsSwitches::GamepadButtonsSwitches()
    : Processor()
    , propButtons("buttons", "Buttons")
    , propButtonA("buttonA", "Button A")
    , propButtonB("buttonB", "Button B")
    , propButtonX("buttonX", "Button X")
    , propButtonY("buttonY", "Button Y")
    , propButtonLeft("buttonLeft", "Button Left")
    , propButtonRight("buttonRight", "Button Right")
    , propButtonUp("buttonUp", "Button Up")
    , propButtonDown("buttonDown", "Button Down")
    , propButtonL1("buttonL1", "Button L1")
    , propButtonR1("buttonR1", "Button R1")
    , propButtonL3("buttonL3", "Button L3")
    , propButtonR3("buttonR3", "Button R3")
    , propButtonStart("buttonStart", "Button Start")
    , propButtonCenter("buttonCenter", "Button Center")
    , propButtonGuide("buttonGuide", "Button Guide")
    , propSwitches("switches", "Switches")
    , propSwitchA("switchA", "Switch A")
    , propSwitchB("switchB", "Switch B")
    , propSwitchX("switchX", "Switch X")
    , propSwitchY("switchY", "Switch Y")
    , propSwitchLeft("switchLeft", "Switch Left")
    , propSwitchRight("switchRight", "Switch Right")
    , propSwitchUp("switchUp", "Switch Up")
    , propSwitchDown("switchDown", "Switch Down")
    , propSwitchL1("switchL1", "Switch L1")
    , propSwitchR1("switchR1", "Switch R1")
    , propSwitchL3("switchL3", "Switch L3")
    , propSwitchR3("switchR3", "Switch R3")
    , propSwitchStart("switchStart", "Switch Start")
    , propSwitchCenter("switchCenter", "Switch Center")
    , propSwitchGuide("switchGuide", "Switch Guide")
    , propGamepad("gamepad", "Gamepad")
    , propText("text", "Help and Notifications") {
    // Setup buttons
    propButtons.addProperties(propButtonA, propButtonB, propButtonX, propButtonY, propButtonLeft,
                              propButtonRight, propButtonUp, propButtonDown, propButtonL1,
                              propButtonR1, propButtonL3, propButtonR3, propButtonStart,
                              propButtonCenter, propButtonGuide);
    propButtons.setCollapsed(true).setCurrentStateAsDefault();
    addProperty(propButtons);

    // Setup switches
    propSwitches.addProperties(propSwitchA, propSwitchB, propSwitchX, propSwitchY, propSwitchLeft,
                               propSwitchRight, propSwitchUp, propSwitchDown, propSwitchL1,
                               propSwitchR1, propSwitchL3, propSwitchR3, propSwitchStart,
                               propSwitchCenter, propSwitchGuide);
    propSwitches.setCollapsed(true).setCurrentStateAsDefault();
    addProperty(propSwitches);

    // Setup the gamepad input
    propGamepad.setCollapsed(true).setReadOnly(true).setCurrentStateAsDefault();
    addProperty(propGamepad);

    // Translate gamepad input into button presses and state changes of the switches
    propGamepad.propButtonA.onChange(
        [&]() { Trigger(propGamepad.propButtonA, propButtonA, propSwitchA); });
    propGamepad.propButtonB.onChange(
        [&]() { Trigger(propGamepad.propButtonB, propButtonB, propSwitchB); });
    propGamepad.propButtonX.onChange(
        [&]() { Trigger(propGamepad.propButtonX, propButtonX, propSwitchX); });
    propGamepad.propButtonY.onChange(
        [&]() { Trigger(propGamepad.propButtonY, propButtonY, propSwitchY); });
    propGamepad.propButtonLeft.onChange(
        [&]() { Trigger(propGamepad.propButtonLeft, propButtonLeft, propSwitchLeft); });
    propGamepad.propButtonRight.onChange(
        [&]() { Trigger(propGamepad.propButtonRight, propButtonRight, propSwitchRight); });
    propGamepad.propButtonUp.onChange(
        [&]() { Trigger(propGamepad.propButtonUp, propButtonUp, propSwitchUp); });
    propGamepad.propButtonDown.onChange(
        [&]() { Trigger(propGamepad.propButtonDown, propButtonDown, propSwitchDown); });
    propGamepad.propButtonL1.onChange(
        [&]() { Trigger(propGamepad.propButtonL1, propButtonL1, propSwitchL1); });
    propGamepad.propButtonR1.onChange(
        [&]() { Trigger(propGamepad.propButtonR1, propButtonR1, propSwitchR1); });
    propGamepad.propButtonL3.onChange(
        [&]() { Trigger(propGamepad.propButtonL3, propButtonL3, propSwitchL3); });
    propGamepad.propButtonR3.onChange(
        [&]() { Trigger(propGamepad.propButtonR3, propButtonR3, propSwitchR3); });
    propGamepad.propButtonStart.onChange(
        [&]() { Trigger(propGamepad.propButtonStart, propButtonStart, propSwitchStart); });
    propGamepad.propButtonCenter.onChange(
        [&]() { Trigger(propGamepad.propButtonCenter, propButtonCenter, propSwitchCenter); });
    propGamepad.propButtonGuide.onChange(
        [&]() { Trigger(propGamepad.propButtonGuide, propButtonGuide, propSwitchGuide); });

    // Help and Notifications
    propText.setCollapsed(true)
        .setReadOnly(true)
        .setSerializationMode(PropertySerializationMode::None)
        .setCurrentStateAsDefault();
    addProperties(propText);

    // Show help when pressing Start button
    propGamepad.propButtonStart.onChange([&] {
        if (!propGamepad.propButtonStart) {
            std::string HelpText;
            GenerateHelpText(HelpText);

            if (propText.propHelp.get().size() < 3) {
                propText.setHelpText(HelpText);
            } else {
                propText.setHelpText(" ");
            }
        }
    });
}

void GamepadButtonsSwitches::Trigger(const BoolProperty& Input, ButtonProperty& Button,
                                     BoolProperty& Switch) {
    // We issue a button press or switch state change on releasing the gamepad button, i.e., when it
    // turns 'false'. This is standard on essentially all operating systems.
    if (Input == false) {
        Button.pressButton();
        Switch = !Switch;

        // Update help text
        if (propText.propHelp.get().size() > 3) {
            std::string HelpText;
            GenerateHelpText(HelpText);
            propText.setHelpText(HelpText);
        }
    }
}

void GamepadButtonsSwitches::GenerateHelpText(std::string& HelpText) {
    HelpText = std::string("Press any button on the gamepad") +
               " to trigger the buttons and switches of this processor.\n" +
               "Here is the current state of the switches:\n\n";

    for (auto pProperty : propSwitches) {
        BoolProperty* pSwitch = dynamic_cast<BoolProperty*>(pProperty);
        if (pSwitch) {
            HelpText += pSwitch->getDisplayName() + (pSwitch->get() ? ": on\n" : ": off\n");
        }
    }
}

}  // namespace inviwo
