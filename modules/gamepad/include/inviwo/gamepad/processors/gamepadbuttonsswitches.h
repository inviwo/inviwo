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

#pragma once

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/boolproperty.h>

#include <inviwo/gamepad/properties/gamepadvaluesproperty.h>
#include <inviwo/gamepad/properties/gamepadtextproperty.h>
#include <inviwo/gamepad/gamepadmoduledefine.h>

namespace inviwo {

/** \docpage{org.inviwo.GamepadButtonsSwitches, Gamepad Buttons & Switches}
 * ![](org.inviwo.GamepadButtonsSwitches.png?classIdentifier=org.inviwo.GamepadButtonsSwitches)
 *
 * Allows to press buttons and toggle switches in Inviwo using a gamepad.
 *
 * ### Properties
 *   * __Buttons__ List of buttons. Connect with the buttons in other processors that you want to
 * steer.
 *   * __Switches__ List of switches. Connect with the switches in other processors that you want to
 * steer.
 *   * __Gamepad__ Connect with GamepadInput processor to get input from a gamepad.
 *   * __Help and Notifications__ Help and notification texts for heads-up display.
 */
class IVW_MODULE_GAMEPAD_API GamepadButtonsSwitches : public Processor {
public:
    GamepadButtonsSwitches();
    virtual ~GamepadButtonsSwitches() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    // Methods
protected:
    /// Presses button and switches the switch
    void Trigger(const BoolProperty& Input, ButtonProperty& Button, BoolProperty& Switch);

    /// Creates help text
    void GenerateHelpText(std::string& HelpText);

    // Properties
public:
    /// Buttons that will be pressed when the respective button on the gamepad will be released
    CompositeProperty propButtons;
    ButtonProperty propButtonA;
    ButtonProperty propButtonB;
    ButtonProperty propButtonX;
    ButtonProperty propButtonY;
    ButtonProperty propButtonLeft;
    ButtonProperty propButtonRight;
    ButtonProperty propButtonUp;
    ButtonProperty propButtonDown;
    ButtonProperty propButtonL1;
    ButtonProperty propButtonR1;
    ButtonProperty propButtonL3;
    ButtonProperty propButtonR3;
    ButtonProperty propButtonStart;
    ButtonProperty propButtonCenter;
    ButtonProperty propButtonGuide;

    /// Boolean checkboxes whose state will switch when the respective button on the gamepad will be
    /// released
    CompositeProperty propSwitches;
    BoolProperty propSwitchA;
    BoolProperty propSwitchB;
    BoolProperty propSwitchX;
    BoolProperty propSwitchY;
    BoolProperty propSwitchLeft;
    BoolProperty propSwitchRight;
    BoolProperty propSwitchUp;
    BoolProperty propSwitchDown;
    BoolProperty propSwitchL1;
    BoolProperty propSwitchR1;
    BoolProperty propSwitchL3;
    BoolProperty propSwitchR3;
    BoolProperty propSwitchStart;
    BoolProperty propSwitchCenter;
    BoolProperty propSwitchGuide;

    /// Readout from the gamepad
    GamepadValuesProperty propGamepad;

    /// Holds text properties for help and notifications
    GamepadTextProperty propText;

    // Attributes
private:
};

}  // namespace inviwo
