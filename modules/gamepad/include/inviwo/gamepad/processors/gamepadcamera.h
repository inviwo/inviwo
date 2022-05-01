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
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/util/timer.h>

#include <inviwo/gamepad/properties/gamepadvaluesproperty.h>
#include <inviwo/gamepad/properties/gamepadtextproperty.h>
#include <inviwo/gamepad/gamepadmoduledefine.h>

namespace inviwo {

/** \docpage{org.inviwo.GamepadCamera, Gamepad Camera}
 * ![](org.inviwo.GamepadCamera.png?classIdentifier=org.inviwo.GamepadCamera)
 *
 * Connected to a GamepadInput processor, this processor can steer the camera.
 * Connect the camera from this processor to whatever processor you want to steer.
 * Learn how to steer the camera by reading the help text (press Start button on gamepad).
 *
 * The processor supports home positions, i.e., saved camera properties
 * that can be recalled at any time.
 *
 * ### Properties
 *   * __Frames per Second__ How often the joysticks shall be queried for updates.
 *   * __Sensitivity__ How much to move/rotate per update.
 *   * __Camera__ Connect this to whatever camera you want to steer.
 *   * __Gamepad__ Connect this to a readout group of a GamepadInput processor.
 *   * __Home Positions__ List of home positions. You can add, remove, and rename them.
 *   * __Keyboard Shortcuts__ Shortcuts for switching home positions via keyboard.
 *   * __Help and Notifications__ Help and notification for the camera.
 *
 */
class IVW_MODULE_GAMEPAD_API GamepadCamera : public Processor, public PropertyOwnerObserver {
public:
    GamepadCamera();
    virtual ~GamepadCamera() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    static const std::string helpText_;

    // Methods
protected:
    /// Determines the nearest canonical position and goes there.
    /// If we are already at a canonical position, we go to the next one.
    void GoToNearestCanonicalPosition();

    /// Returns the nearest canonical vector for a given vector
    vec3 NearestCanonicalDirection(const vec3& Direction, const float AngleDistance) const;

    /// Rotate by canonical amounts in horizontal and vertical direction
    void RotateCanonical(const int nHorizontal, const int nVertical);

    /** Rotates via pitch, yaw and roll on the trackball by the given amounts.

        @param[in] rotH Amount of horizontal rotation in radians.
        @param[in] rotV Amount of vertical rotation in radians.
        @param[in] Right Horizontal vector. We rotate vertically around this axis (pitch).
        @param[in, out] Dir Vector pointing to center of trackball. We rotate around it (roll).
                        Will be updated to point to new position after rotation.
        @param[in, out] Up Vertical vector. We rotate horizontally around this axis (yaw).
                        Will be updated.
    */
    void RotateOnTrackball(const float rotH, const float rotV, const vec3& Right, vec3& Dir,
                           vec3& Up);

    /** Deals with the button presses around setting, removing and jumping to home positions.

        @param[in] bBWasRelease Whether the B-button on the gamepad was released.
                   If false, we assume that the X-button was released instead.
    */
    void SetOrRevertHome(const bool bBWasReleased);

    /// Add the current camera as a new home position
    void AddHome();

    /// If the current camera is identical to the current home position, then delete that home
    /// position. If they are not identical yet, jump to the cuurent home position and the next call
    /// to this function
    /// will trigger the deletion.
    void RemoveHome();

    /// Jump to next home position.
    void NextHome();

    /// Jump to previous home position.
    void PreviousHome();

    /// Create a state string depending on the pressed buttons
    void SetStateString();

    /// Called by the timer. Most things happen here.
    void Tick();

private:
    void deserialize(Deserializer& d);
    virtual void onDidAddProperty(Property* property, size_t index) override;

    // Properties
public:
    /// Update speed.
    /// This is the desired frames per second.
    /// We will query the gamepad and update the camera as often as this property suggests and the
    /// system allows.
    IntSizeTProperty propFPS;

    /// How strong we move the camera in one step.
    /// Zero means, we do not move the camera at all.
    /// One means, we move the camera by the full amount,
    /// which means something different for the particular camera movement aspects.
    FloatProperty propSensitivity;

    /// Camera steered by the gamepad
    CameraProperty propCamera;

    /// Readout from the gamepad
    GamepadValuesProperty propGamepad;

    /// ID of the current home position
    IntSizeTProperty propCurrentHomeID;

    /// Container for camera properties holding different home positions
    ListProperty propHomePositions;

    /// Shortcuts for switching between home positions
    CompositeProperty propKeyboardShortcuts;

    /// Activates the next home position
    EventProperty propEventNextHome;

    /// Activates the previous home position
    EventProperty propEventPreviousHome;

    /// Holds text properties for help and notifications
    GamepadTextProperty propText;

    // Attributes
protected:
    /// Do not overwrite an added home position with the current camera during deserialization.
    bool bCurrentlyLoading;

private:
    /// Calls the Tick() function in regular intervals to change the camera
    Timer Ticker;

    /// Whether we call the Tick() function for the first time.
    /// If so, we want to save the current camera as a home position, if no home positions exist
    /// yet.
    bool bFirstTick;
};

}  // namespace inviwo
