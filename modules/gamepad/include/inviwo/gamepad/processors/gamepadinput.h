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
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/gamepad/properties/gamepadvaluesproperty.h>
#include <inviwo/gamepad/properties/gamepadtextproperty.h>

#include <inviwo/gamepad/gamepadmoduledefine.h>

#include <QWindow>
#include <QGamepad>

namespace inviwo {

/** \docpage{org.inviwo.GamepadInput, Gamepad Input}
 * ![](org.inviwo.GamepadInput.png?classIdentifier=org.inviwo.GamepadInput)
 *
 * This processor allows to steer Inviwo properties via a gamepad / controller.
 * Several gamepads can be connected to the computer and used at the same time,
 * but each gamepad requires a separate GamepadInput processor.
 * You can switch between different gamepads at the top of the processor.
 *
 * This processor has no inputs or outputs in the classic sense.
 * Everything is done through property connections.
 *
 * This processor itself will not steer other Inviwo processors,
 * but rather one needs to connect specific gamepad-related processors,
 * which will then manipulate the camera or some buttons.
 *
 * Connections between gamepad-related processors are done via readout groups.
 * A readout group represents the raw information from the gamepad.
 * A camera-steering gamepad processor can then use this to manipulate the camera.
 *
 * This GamepadInput processor can have several readout groups.
 * Only one is active at the time. Hence, we can switch between
 * readout groups. This allows us to steer the camera, then switch
 * to the manipuation of other buttons or a transfer function.
 * One can think of this as "different modes" for the gamepad.
 * Note that several gamepad-related processors can be connected
 * to the same readout group. In this case, one needs to manually
 * take care of non-overlapping key assignments.
 *
 * A heads-up display is supported through a GamepadTextProperty.
 * One should connected the properties State, Notification and Help
 * to a GamepadUI composite processor.
 *
 * ### Properties
 *   * __Gamepad__ Switch between gamepads.
 *   * __Readout Groups__ Add, remove, rename readout groups.
 *   * __Active__ Which readout group is active.
 *   * __Keyboard Shortcuts__ Switch between active readout groups. This can also be done via the
 * Select button on the gamepad.
 *   * __Help and Notifications__ Provides help texts.
 *
 */
class IVW_MODULE_GAMEPAD_API GamepadInput : public Processor,
                                            public PropertyObserver,
                                            public PropertyOwnerObserver,
                                            public QWindow {
public:
    GamepadInput();
    virtual ~GamepadInput();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    // Methods:
public:
    /// Updates the UI and other things when gamepads are connected or disconnected.
    void updateGamepads();

    /// Updates the UI for selecting the readout group
    void updateGroupSelect();

    /// Connects to the gamepad selected via the propGamepadSelect property.
    void connectToSelectedGamepad();

    /// Connects to the active readout group
    void connectToSelectedGroup();

    /// Activates the next readout group
    void nextReadoutGroup();

    /// Activates the previous readout group
    void previousReadoutGroup();

private:
    /// Detect gamepads after reloading workspace and other updates
    virtual void deserialize(Deserializer& d) override;

    virtual void onSetDisplayName(Property* property, const std::string& displayName) override;
    virtual void onDidAddProperty(Property* property, size_t index) override;
    virtual void onDidRemoveProperty(Property* property, size_t index) override;

    // Properties
public:
    /// Selects the gamepad to be used by this processor.
    /// Several gamepads can be supported by using several instances of this processor.
    OptionPropertyInt propGamepadSelect;

    /// Group of several instances of GamepadValuesProperty.
    /// Other processors can connect to one of them to receive gamepad input,
    /// and we can switch between which one is active.
    ListProperty propReadoutGroups;

    /// Selects the active readout group.
    OptionPropertyString propGroupSelect;

    /// Shortcuts for switching between groups
    CompositeProperty propKeyboardShortcuts;

    /// Activates the next readout group
    EventProperty propEventNextGroup;

    /// Activates the previous readout group
    EventProperty propEventPreviousGroup;

    /// Text for notifications, state and help
    GamepadTextProperty propText;

    // Attributes
private:
    /// Actual gamepad connection
    QGamepad Pad;

    /// Connections between QGamepad and the readout.
    /// Used for disconnecting when switching readout groups.
    QList<QMetaObject::Connection> readoutConnections;

    /// Pointer to current readout
    GamepadValuesProperty* pCR;
};

}  // namespace inviwo
