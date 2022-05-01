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

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/stringproperty.h>

#include <inviwo/gamepad/gamepadmoduledefine.h>

namespace inviwo {

/** \brief Help and notification texts for gamepad modules.

    Gamepads may be used when presenting or working in full-screen mode.
    Then it is useful to guide the user with some heads-up display.
    This property contains three strings, each intended for a different purpose.
    A notification string gives a brief feedback and disappears from the screen automatically.
    A state string notifies us about a particular state for the controller.
    A help string provides a help text for the entire module.

    This property should be added to any module that works with gamepad input
    and wants to display some help or notification to the user.

    A composite processor GamepadUI is available which contains
    three text renderers pre-configured to show help and notifications
    in the correct places on the screen.

    @author Tino Weinkauf
*/
class IVW_MODULE_GAMEPAD_API GamepadTextProperty : public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    GamepadTextProperty(std::string identifier, std::string displayName,
                        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                        PropertySemantics semantics = PropertySemantics::Default);

    GamepadTextProperty(const GamepadTextProperty& rhs);

    virtual GamepadTextProperty* clone() const override;

    virtual ~GamepadTextProperty();

    // Methods
public:
    /// Sets notification. Will be cleared after 2 seconds.
    void setNotificationText(const std::string& text);

    /// Sets text reflecting the current state.
    void setStateText(const std::string& text);

    /// Sets help text.
    void setHelpText(const std::string& text);

    // Properties
public:
    /// Holds short notification text about an event that just happened.
    /// Example: "Home Position 1 saved"
    StringProperty propNotification;

    /// Holds short help text about the current state.
    /// Examples: "single-axis mode" or "rotation around world axes"
    StringProperty propState;

    /// Holds long help text about the entire usage of the processor.
    StringProperty propHelp;

    // Attributes
protected:
    /// Timer for clearing the notification
    Timer NotificationClearTimer;
};

}  // namespace inviwo
