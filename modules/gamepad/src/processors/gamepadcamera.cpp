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

#include <inviwo/gamepad/processors/gamepadcamera.h>

#include <inviwo/gamepad/processors/gamepadinput.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GamepadCamera::processorInfo_{
    "org.inviwo.GamepadCamera",  // Class identifier
    "Gamepad Camera",            // Display name
    "Gamepad",                   // Category
    CodeState::Experimental,     // Code state
    Tags::CPU,                   // Tags
};
const ProcessorInfo GamepadCamera::getProcessorInfo() const { return processorInfo_; }

const std::string GamepadCamera::helpText_{
    "Left Joystick: rotate on trackball (pitch and yaw)\n\
DPAD: rotate stepwise on trackball  (pitch and yaw)\n\
Y + the above: roll\n\
A + the above: use world coordinate system\n\
\n\
Right Joystick: move\n\
\n\
X: slow rotation or movement\n\
B: fast rotation or movement\n\
\n\
Left (shoulder): restrict rotation/movement to single, most dominant axis\n\
Right (shoulder): reset to nearest canonical position (modify with X or B)\n\
\n\
X + B: jump to next home position\n\
B + X: jump to previous home position\n\
Left + X + B: add home position\n\
Left + B + X: delete home position\
"};

GamepadCamera::GamepadCamera()
    : Processor()
    , propFPS("fps", "Frames per Second", 15, 1, 60, 1)
    , propSensitivity("sensitivity", "Sensitivity", 0.15f, 0, 1.0f)
    , propCamera("camera", "Camera")
    , propGamepad("gamepad", "Gamepad")
    , propCurrentHomeID("currentHomeID", "Current Home ID", 0, {0, ConstraintBehavior::Immutable},
                        {100, ConstraintBehavior::Immutable}, 1)
    , propHomePositions("homePositions", "Home Positions", 100)
    , propKeyboardShortcuts("keyboardShortcuts", "Keyboard Shortcuts")
    , propEventNextHome(
          "eventNextHome", "Next Home",
          [this](Event* e) {
              NextHome();
              e->markAsUsed();
          },
          IvwKey::L, KeyState::Press)
    , propEventPreviousHome(
          "eventPreviousHome", "Previous Home",
          [this](Event* e) {
              PreviousHome();
              e->markAsUsed();
          },
          IvwKey::J, KeyState::Press)
    , propText("text", "Help and Notifications")
    , bCurrentlyLoading(false)
    , Ticker(Timer::Milliseconds(1000 / propFPS), [&] { Tick(); })
    , bFirstTick(true) {
    // Update speed in frames per second.
    propFPS.onChange([&] { Ticker.setInterval(Timer::Milliseconds(1000 / propFPS)); });
    addProperty(propFPS);

    // How much we change per frame.
    addProperty(propSensitivity);

    // Actual camera to be manipulated
    propCamera.setCollapsed(true).setCurrentStateAsDefault();
    addProperty(propCamera);

    // Gamepad to listen to
    propGamepad.setCollapsed(true).setReadOnly(true).setCurrentStateAsDefault();
    addProperty(propGamepad);

    // Home positions: prefab and observation
    CameraProperty Template("home0", "Home 0");
    Template.setCollapsed(true).setCurrentStateAsDefault();
    propHomePositions.addPrefab(std::make_unique<CameraProperty>(Template));
    propHomePositions.PropertyOwnerObservable::addObserver(this);

    // Home positions: properties
    propCurrentHomeID.setReadOnly(true)
        .setVisible(false)
        .setSemantics(PropertySemantics::Text)
        .setCurrentStateAsDefault();
    propHomePositions.setCollapsed(true).setCurrentStateAsDefault();
    addProperties(propCurrentHomeID, propHomePositions);

    // Keyboard shortcuts for switching home positions
    propKeyboardShortcuts.addProperties(propEventNextHome, propEventPreviousHome);
    propKeyboardShortcuts.setCollapsed(true).setCurrentStateAsDefault();
    addProperty(propKeyboardShortcuts);

    // Help and Notifications
    propText.setCollapsed(true)
        .setReadOnly(true)
        .setSerializationMode(PropertySerializationMode::None)
        .setCurrentStateAsDefault();
    addProperties(propText);

    // Show help when pressing Start button
    propGamepad.propButtonStart.onChange([&] {
        if (!propGamepad.propButtonStart) {
            if (propText.propHelp.get() != helpText_) {
                propText.setHelpText(helpText_);
            } else {
                propText.setHelpText(" ");
            }
        }
    });

    // Combinations of the B-button and X-button allow to save/remove home positions
    propGamepad.propButtonB.onChange([&] {
        SetStateString();
        if (!propGamepad.propButtonB) SetOrRevertHome(true);
    });
    propGamepad.propButtonX.onChange([&] {
        SetStateString();
        if (!propGamepad.propButtonX) SetOrRevertHome(false);
    });

    // Other state modifiers
    propGamepad.propButtonA.onChange([&] { SetStateString(); });
    propGamepad.propButtonY.onChange([&] { SetStateString(); });
    propGamepad.propButtonL1.onChange([&] { SetStateString(); });

    // When the right button (shoulder) is pressed, we adjust the view to a nearby canonical
    // position.
    propGamepad.propButtonR1.onChange([&] {
        if (!propGamepad.propButtonR1) GoToNearestCanonicalPosition();
    });

    // When any button on the DPAD is pressed, we rotate by a canonical amount.
    propGamepad.propButtonLeft.onChange([&] {
        if (!propGamepad.propButtonLeft) RotateCanonical(-1, 0);
    });
    propGamepad.propButtonRight.onChange([&] {
        if (!propGamepad.propButtonRight) RotateCanonical(1, 0);
    });
    propGamepad.propButtonUp.onChange([&] {
        if (!propGamepad.propButtonUp) RotateCanonical(0, -1);
    });
    propGamepad.propButtonDown.onChange([&] {
        if (!propGamepad.propButtonDown) RotateCanonical(0, 1);
    });

    // Start listening to any gamepad changes
    Ticker.start();
}

void GamepadCamera::deserialize(Deserializer& d) {
    bCurrentlyLoading = true;
    Processor::deserialize(d);
    bCurrentlyLoading = false;
}

void GamepadCamera::GoToNearestCanonicalPosition() {
    // Get current position
    const vec3& LookFrom = propCamera.getLookFrom();
    const vec3& LookTo = propCamera.getLookTo();
    const vec3& LookUp = propCamera.getLookUp();

    // Augment current camera setup with some auxillary vectors and scalars
    const vec3 CamDir = LookFrom - LookTo;
    const float ViewDistance = glm::length(CamDir);

    // Which slots to consider? 22.5 degrees, 45 degrees, 90 degrees?
    float AngleDistance(glm::quarter_pi<float>());
    if (propGamepad.propButtonB) AngleDistance = glm::half_pi<float>();
    if (propGamepad.propButtonX) AngleDistance = glm::quarter_pi<float>() / 2.0f;

    // Snap into place and compute the corresponding rotation. Apply it to LookUp.
    const vec3 CanonicalCamDir = NearestCanonicalDirection(CamDir, AngleDistance);
    const vec3 CanonicalLookFrom = LookTo + ViewDistance * CanonicalCamDir;
    //// -  rotate the LookUp vector and snap it afterwards.
    const auto RotQuaternion = glm::quat(glm::normalize(CamDir), CanonicalCamDir);
    const vec3 IntermediateLookUp = glm::rotate(RotQuaternion, LookUp);

    // Snap the LookUp to a canonical place as well
    vec3 CanonicalLookUp = NearestCanonicalDirection(IntermediateLookUp, AngleDistance);
    // Question/TODO: should we orthogonolize this setup again? LookUp and CamDir should be
    // orthogonal.

    // Final Setup
    propCamera.setLook(CanonicalLookFrom, LookTo, CanonicalLookUp);

    // Notify
    propText.propNotification = std::string("reset to nearest canoncial position");
}

vec3 GamepadCamera::NearestCanonicalDirection(const vec3& Direction,
                                              const float AngleDistance) const {
    // Get current position in spherical coordinates (angles only)
    const float Inclination = std::atan2(
        std::sqrt(Direction[0] * Direction[0] + Direction[1] * Direction[1]), Direction[2]);
    const float Azimuth = std::atan2(Direction[1], Direction[0]);

    // Based on the current angles, what is the nearest slot?
    const float SlotInclination = round(Inclination / AngleDistance) * AngleDistance;
    const float SlotAzimuth =
        round((Azimuth + glm::pi<float>()) / AngleDistance) * AngleDistance - glm::pi<float>();

    // Return the canonical direction
    return vec3(sin(SlotInclination) * cos(SlotAzimuth), sin(SlotInclination) * sin(SlotAzimuth),
                cos(SlotInclination));
}

void GamepadCamera::RotateCanonical(const int nHorizontal, const int nVertical) {
    // Get current camera setup
    const vec3& LookFrom = propCamera.getLookFrom();
    const vec3& LookTo = propCamera.getLookTo();
    const vec3& LookUp = propCamera.getLookUp();

    // Augment current camera setup with some auxillary vectors and scalars
    const vec3 CamDir = glm::normalize(LookFrom - LookTo);
    const float ViewDistance = glm::length(LookFrom - LookTo);
    const vec3 LookRight = glm::cross(CamDir, LookUp);
    const vec3 LookRightNormed = glm::normalize(LookRight);
    const vec3 LookUpNormed = glm::normalize(LookUp);  // pretty sure it always is normed, but
                                                       // anyway

    // Rotate by how much? 22.5 degrees, 45 degrees, 90 degrees?
    float AngleDistance(glm::quarter_pi<float>());
    if (propGamepad.propButtonB) AngleDistance = glm::half_pi<float>();
    if (propGamepad.propButtonX) AngleDistance = glm::quarter_pi<float>() / 2.0f;
    const float rotH = nHorizontal * AngleDistance;
    const float rotV = nVertical * AngleDistance;

    // Actual Rotation
    vec3 NewCamDir(CamDir);
    vec3 NewLookUp(LookUpNormed);
    RotateOnTrackball(rotH, rotV, LookRightNormed, NewCamDir, NewLookUp);
    const vec3 NewLookFrom = LookTo + ViewDistance * NewCamDir;

    // Final Setup
    propCamera.setLook(NewLookFrom, LookTo, NewLookUp);
}

void GamepadCamera::RotateOnTrackball(const float rotH, const float rotV, const vec3& Right,
                                      vec3& Dir, vec3& Up) {
    // Anything to rotate at all?
    if (rotH == 0 && rotV == 0) return;

    // Rotate via the camera coordinate system, or via the world coordinate system?
    if (!propGamepad.propButtonA) {
        // Pitch and Yaw when the Y-button is not pressed, and Roll when the Y-button is pressed
        if (!propGamepad.propButtonY) {
            /////////////////////////////////
            // Pitch and Yaw
            // - we rotate the view vector as it points from the center of the trackball outwards
            const vec3 StartRotDir = glm::normalize(Dir);
            Dir = StartRotDir;
            // - horizontal rotation around the Up vector
            Dir = glm::rotate(Dir, rotH, Up);
            // - vertical rotation around the Right vector
            Dir = glm::rotate(Dir, -rotV, Right);
            // -  finally, rotate the Up vector as well, since we are moving on a sphere
            const auto RotQuaternion = glm::quat(StartRotDir, Dir);
            Up = glm::rotate(RotQuaternion, Up);
        } else {
            /////////////////////////////////
            // Roll
            // - we roll by the maximal of the x/y values of the joystick.
            const float roll = fabs(rotH) > fabs(rotV) ? rotH : rotV;
            Up = glm::rotate(Up, roll, Dir);
        }
    } else {
        // Rotate around x-axis and y-axis when the Y-button is not pressed.
        // Rotate around the z-axis when the Y-button is pressed.
        // For the latter, we use the maximal of the x/y values of the joystick.
        const vec3 EulerAngles(
            propGamepad.propButtonY ? 0 : rotH, propGamepad.propButtonY ? 0 : rotV,
            propGamepad.propButtonY ? (fabs(rotH) > fabs(rotV) ? rotH : rotV) : 0);

        const auto RotQuaternion = glm::quat(EulerAngles);
        Dir = glm::rotate(RotQuaternion, Dir);
        Up = glm::rotate(RotQuaternion, Up);
    }
}

void GamepadCamera::SetOrRevertHome(const bool bBWasReleased) {
    // B-button: X-Modifier recall next one
    // B-button: Left-X-Modifier add one
    // X-button: B-Modifier recall previous one
    // X-button: Left-B-Modifier recall current one; when identical, then remove current one

    // State of modifiers and variables
    const bool bXPressed = propGamepad.propButtonX;
    const bool bBPressed = propGamepad.propButtonB;
    const bool bLeftPressed = propGamepad.propButtonL1;
    const bool bXWasReleased = !bBWasReleased;
    const bool bHaveAnyHomes = (propHomePositions.size() > 0);

    // Anything pressed?
    if (!bXPressed && !bBPressed) return;

    // Add current position as a new home position
    if (bLeftPressed && bXPressed && bBWasReleased) {
        AddHome();
        return;
    }

    // We need at least one home for anything further.
    if (!bHaveAnyHomes) return;

    // Delete current home position, if we are on it
    if (bLeftPressed && bXWasReleased && bBPressed) {
        RemoveHome();
        return;
    }

    // Recall next home position
    if (bXPressed && bBWasReleased) {
        NextHome();
        return;
    }

    // Recall previous home position
    if (bBPressed && bXWasReleased) {
        PreviousHome();
        return;
    }
}

void GamepadCamera::onDidAddProperty(Property* property, size_t) {
    // Are we restoring a network? Then we will not change the camera property.
    if (bCurrentlyLoading) return;

    CameraProperty* pNewHome = dynamic_cast<CameraProperty*>(property);
    if (!pNewHome) return;

    // Set the new camera home to the current camera
    pNewHome->set(&propCamera);
    pNewHome->setCollapsed(true).setCurrentStateAsDefault();
    propCurrentHomeID = propHomePositions.size() - 1;

    // Notify
    propText.propNotification =
        std::string("new home position \'") + pNewHome->getDisplayName() + std::string("\' added");
}

void GamepadCamera::AddHome() { propHomePositions.constructProperty(0); }

void GamepadCamera::RemoveHome() {
    if (propHomePositions.size() == 0) {
        propText.propNotification = std::string("no home position to delete");
        return;
    }

    // Get current home position
    propCurrentHomeID = propCurrentHomeID % propHomePositions.size();
    CameraProperty* pHome = dynamic_cast<CameraProperty*>(propHomePositions[propCurrentHomeID]);
    if (pHome && propCamera.get() == pHome->get()) {
        // The current camera is at this home position. Now delete.
        propText.propNotification =
            std::string("deleted home position \'") + pHome->getDisplayName() + "\'";
        propHomePositions.removeProperty(pHome);
        pHome = nullptr;

        // We leave, if nothing is left. Otherwise, some of the code below will have modulo/division
        // by zero.
        if (propHomePositions.size() == 0) return;

        // Rename identifiers/names to be a continguously numbered list
        for (int i(0); i < propHomePositions.size(); i++) {
            // Only rename if it looks like a default name
            if (propHomePositions[i]->getDisplayName().find("Home ") == 0) {
                std::string Identifier(std::string("home") + std::to_string(i));
                std::string DisplayName(std::string("Home ") + std::to_string(i));
                propHomePositions[i]->setIdentifier(Identifier);
                propHomePositions[i]->setDisplayName(DisplayName);
            }
        }

        // Jump to next home: prepare ID here, jump a few lines below.
        propCurrentHomeID = propCurrentHomeID % propHomePositions.size();
    } else {
        // The current camera is not at this position yet.
        // Update notification and jump (code below) there first.
        propText.propNotification = std::string("delete home position \'") +
                                    propHomePositions[propCurrentHomeID]->getDisplayName() +
                                    std::string("\'? (press again)");
    }

    // The current camera is not at this position yet. Jump there first.
    if (propHomePositions.size() > 0) {
        propCamera.set(propHomePositions[propCurrentHomeID]);
    }
}

void GamepadCamera::NextHome() {
    if (propHomePositions.size() > 0) {
        propCurrentHomeID = (propCurrentHomeID + 1) % propHomePositions.size();
        propCamera.set(propHomePositions[propCurrentHomeID]);

        propText.propNotification = std::string("home position \'") +
                                    propHomePositions[propCurrentHomeID]->getDisplayName() + "\'";
    }
}

void GamepadCamera::PreviousHome() {
    if (propHomePositions.size() > 0) {
        propCurrentHomeID =
            (propCurrentHomeID == 0) ? propHomePositions.size() - 1 : propCurrentHomeID - 1;
        propCamera.set(propHomePositions[propCurrentHomeID]);

        propText.propNotification = std::string("home position \'") +
                                    propHomePositions[propCurrentHomeID]->getDisplayName() + "\'";
    }
}

void GamepadCamera::SetStateString() {
    // What is going on?
    std::vector<std::string> StateVector;
    if (propGamepad.propButtonA) {
        StateVector.push_back(std::string("world rotation ") +
                              ((propGamepad.propButtonY) ? "around z-axis" : "around x/y-axes"));
    } else {
        if (propGamepad.propButtonY) StateVector.push_back("roll");
    }

    if (!propGamepad.propButtonB && propGamepad.propButtonX) {
        StateVector.push_back("slow");
        if (!propGamepad.propButtonL1) StateVector.push_back("next home: push B");
    }
    if (propGamepad.propButtonB && !propGamepad.propButtonX) {
        StateVector.push_back("fast");
        if (!propGamepad.propButtonL1) StateVector.push_back("previous home: push X");
    }

    if (propGamepad.propButtonL1) {
        if (!propGamepad.propButtonY) StateVector.push_back("single-axis");
        if (propGamepad.propButtonX) StateVector.push_back("to add home: push B");
        if (propGamepad.propButtonB) StateVector.push_back("to delete home: push X");
    }

    // Accumulate, making sure to have commas in the right places
    std::string State(" ");
    if (!StateVector.empty()) {
        State = StateVector[0];
        for (size_t i(1); i < StateVector.size(); i++) {
            State += std::string(", ") + StateVector[i];
        }
    }

    // Actually set the state
    propText.propState.set(State);
}

void GamepadCamera::Tick() {
    // Get current camera setup
    const vec3& LookFrom = propCamera.getLookFrom();
    const vec3& LookTo = propCamera.getLookTo();
    const vec3& LookUp = propCamera.getLookUp();

    // Set initial home, if we do not have any
    if (bFirstTick) {
        // TODO: Will already be called while we are just being dropped into the network.
        bFirstTick = false;
        if (propHomePositions.size() == 0) AddHome();
        return;
    }

    // Augment current camera setup with some auxillary vectors and scalars
    const vec3 CamDir = glm::normalize(LookFrom - LookTo);
    const float ViewDistance = glm::length(LookFrom - LookTo);
    const vec3 LookRight = glm::cross(CamDir, LookUp);
    const vec3 LookRightNormed = glm::normalize(LookRight);
    const vec3 LookUpNormed = glm::normalize(LookUp);  // pretty sure it always is normed, but
                                                       // anyway

    // Get sensitivity
    float SensitivityFactor(1);
    if (propGamepad.propButtonB) SensitivityFactor *= 3.0f;
    if (propGamepad.propButtonX) SensitivityFactor /= 3.0f;
    const float s = SensitivityFactor * propSensitivity.get();

    // Prepare output
    vec3 NewLookFrom(LookFrom);  // pitch and yaw
    vec3 NewLookTo(LookTo);      // translate
    vec3 NewLookUp(LookUp);      // roll
    vec3 NewCamDir(CamDir);

    ///////////////////////////////////////////////
    // ZOOM
    //
    // Zoom using the left/right triggers on the shoulder of the gamepad
    const float Zoom = 1 + s * (propGamepad.propButtonL2 - propGamepad.propButtonR2);

    ///////////////////////////////////////////////
    // ROTATION
    //
    // Rotate around the center of the trackball, i.e., LookTo, using the left joystick.
    // Adjust the LookUp accordingly.
    // Several modificators can be used to achieve exclusivity and other effects.
    // We apply pitch and yaw when the joystick is used normally,
    // and we switch into roll when the Y-button is pressed additionally.

    // Amount of rotation
    float rawRotH = propGamepad.propAxisLeftX;
    float rawRotV = propGamepad.propAxisLeftY;
    // - exclusive?
    if (propGamepad.propButtonL1) {
        if (fabs(rawRotH) > fabs(rawRotV)) {
            rawRotV = 0;
        } else {
            rawRotH = 0;
        }
    }
    // - final amount of rotation
    const float rotH = s * glm::quarter_pi<float>() * rawRotH;
    const float rotV = s * glm::quarter_pi<float>() * rawRotV;

    // Actual rotation
    RotateOnTrackball(rotH, rotV, LookRightNormed, NewCamDir, NewLookUp);
    NewLookFrom = LookTo + Zoom * ViewDistance * NewCamDir;

    ///////////////////////////////////////////////
    // TRANSLATION
    //
    // Move the entire trackball in the horizontal/vertical direction given by the image plane.
    // We use the right joystick.
    float movH = propGamepad.propAxisRightX;
    float movV = propGamepad.propAxisRightY;
    // - exclusive?
    if (propGamepad.propButtonL1) {
        if (fabs(movH) > fabs(movV)) {
            movV = 0;
        } else {
            movH = 0;
        }
    }
    const vec3 MoveOffset = (ViewDistance / 16) *  // division by 16 is just a personal preference
                            (s * movH * LookRightNormed + s * movV * LookUpNormed);

    // Final Setup
    propCamera.setLook(NewLookFrom + MoveOffset, LookTo + MoveOffset, NewLookUp);
}

}  // namespace inviwo
