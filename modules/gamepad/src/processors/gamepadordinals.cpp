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

#include <inviwo/gamepad/processors/gamepadordinals.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GamepadOrdinals::processorInfo_{
    "org.inviwo.GamepadOrdinals",  // Class identifier
    "Gamepad Ordinals",            // Display name
    "Gamepad",                     // Category
    CodeState::Experimental,       // Code state
    Tags::CPU,                     // Tags
};
const ProcessorInfo GamepadOrdinals::getProcessorInfo() const { return processorInfo_; }

const std::string GamepadOrdinals::helpText_{
    "Use the joysticks, the DPAD, and the triggers\n\
to change the following ordinals:\n\
"};

GamepadOrdinals::GamepadOrdinals()
    : Processor()
    , propFPS("fps", "Frames per Second", 15, 1, 60, 1)
    , propSensitivity("sensitivity", "Sensitivity", 0.15f, 0, 1.0f)
    , propMappings("mappings", "Mappings", 20)
    , propGamepad("gamepad", "Gamepad")
    , propText("text", "Help and Notifications")
    , Ticker(Timer::Milliseconds(1000 / propFPS), [&] { Tick(); }) {
    // Update speed in frames per second.
    propFPS.onChange([&] { Ticker.setInterval(Timer::Milliseconds(1000 / propFPS)); });
    addProperty(propFPS);

    // How much we change per frame.
    addProperty(propSensitivity);

    // Mappings
    util::for_each_type<Types>{}(TypeFunctor{}, *this);
    propMappings.setCurrentStateAsDefault();
    addProperty(propMappings);

    // Setup the gamepad input
    propGamepad.setCollapsed(true).setReadOnly(true).setCurrentStateAsDefault();
    addProperty(propGamepad);

    // Help and Notifications
    propText.setCollapsed(true)
        .setReadOnly(true)
        .setSerializationMode(PropertySerializationMode::None)
        .setCurrentStateAsDefault();
    addProperties(propText);

    // Show help when pressing Start button; including the actual mapping
    propGamepad.propButtonStart.onChange([&] {
        if (!propGamepad.propButtonStart) {
            std::string HelpText = helpText_;
            for (Property* p : propMappings) {
                auto pMapping = dynamic_cast<BaseOrdinalGamepadProperty*>(p);
                if (pMapping) pMapping->GetDescription(HelpText);
            }

            if (propText.propHelp.get() != HelpText) {
                propText.setHelpText(HelpText);
            } else {
                propText.setHelpText(" ");
            }
        }
    });

    /* We are not doing the below, since it seemed tedious to use. */
    ////When any button on the DPAD is pressed, we change the value using the default increment.
    // propGamepad.propButtonLeft.onChange([&] { if (!propGamepad.propButtonLeft) DPADPressed(-1,
    // 0); }); propGamepad.propButtonRight.onChange([&] { if (!propGamepad.propButtonRight)
    // DPADPressed(1, 0); }); propGamepad.propButtonUp.onChange([&] { if (!propGamepad.propButtonUp)
    // DPADPressed(0, -1); }); propGamepad.propButtonDown.onChange([&] { if
    // (!propGamepad.propButtonDown) DPADPressed(0, 1); });

    // Start listening to any gamepad changes
    Ticker.start();
}

void GamepadOrdinals::Tick() {
    // Get sensitivity
    float SensitivityBase(1);
    if (propGamepad.propButtonB) SensitivityBase *= 3.0f;
    if (propGamepad.propButtonX) SensitivityBase /= 3.0f;
    const float Sensitivity = SensitivityBase * propSensitivity.get();

    NetworkLock lock(this);
    for (Property* p : propMappings) {
        auto pMapping = dynamic_cast<BaseOrdinalGamepadProperty*>(p);
        if (pMapping) pMapping->ContinuousUpdate(propGamepad, Sensitivity, SensitivityBase);
    }
}

// void GamepadOrdinals::DPADPressed(const float x, const float y)
//{
//    NetworkLock lock(this);
//    for (Property* p : propMappings)
//    {
//        auto pMapping = dynamic_cast<BaseOrdinalGamepadProperty*>(p);
//        if (pMapping) pMapping->DiscreteUpdate(x, y);
//    }
//}

}  // namespace inviwo
