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

#include <inviwo/gamepad/properties/gamepadtextproperty.h>

namespace inviwo {
const std::string GamepadTextProperty::classIdentifier = "org.inviwo.GamepadTextProperty";
std::string GamepadTextProperty::getClassIdentifier() const { return classIdentifier; }

GamepadTextProperty::GamepadTextProperty(std::string identifier, std::string displayName,
                                         InvalidationLevel invalidationLevel,
                                         PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , propNotification("notification", "Notification")
    , propState("state", "State")
    , propHelp("help", "Help")
    , NotificationClearTimer(Timer::Milliseconds(2000), [&] {
        propNotification = "";
        NotificationClearTimer.stop();
    }) {
    propNotification = " ";
    propNotification.setCurrentStateAsDefault();
    propState = " ";
    propState.setCurrentStateAsDefault();
    propHelp = " ";
    propHelp.setCurrentStateAsDefault();
    addProperties(propNotification, propState, propHelp);

    // Clear the notification after some time
    propNotification.onChange([&] {
        if (propNotification.get() != "") {
            NotificationClearTimer.start();
        }
    });
}

GamepadTextProperty::GamepadTextProperty(const GamepadTextProperty& rhs)
    : CompositeProperty(rhs)
    , propNotification(rhs.propNotification)
    , propState(rhs.propState)
    , propHelp(rhs.propHelp)
    , NotificationClearTimer(Timer::Milliseconds(2000), [&] {
        propNotification = " ";
        NotificationClearTimer.stop();
    }) {
    propNotification = " ";
    propNotification.setCurrentStateAsDefault();
    propState = " ";
    propState.setCurrentStateAsDefault();
    propHelp = " ";
    propHelp.setCurrentStateAsDefault();
    addProperties(propNotification, propState, propHelp);

    // Clear the notification after some time
    propNotification.onChange([&] {
        if (propNotification.get() != "" && propNotification.get() != " ") {
            NotificationClearTimer.start();
        }
    });
}

GamepadTextProperty* GamepadTextProperty::clone() const { return new GamepadTextProperty(*this); }

GamepadTextProperty::~GamepadTextProperty() {}

void GamepadTextProperty::setNotificationText(const std::string& text) { propNotification = text; }

void GamepadTextProperty::setStateText(const std::string& text) { propState = text; }

void GamepadTextProperty::setHelpText(const std::string& text) { propHelp = text; }

}  // namespace inviwo
