/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/gamepad/gamepadmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/gamepad/gamepadcontrolledproperty.h>
#include <inviwo/core/properties/cameraproperty.h>

namespace inviwo {

/**
 * \brief GamepadControlledProperty for CameraProperties
 */
class IVW_MODULE_GAMEPAD_API CameraControlledProperty : public GamepadControlledProperty {
public:
    CameraControlledProperty(std::string identifier, std::string displayName);
    virtual ~CameraControlledProperty() = default;
	CameraProperty controlledProperty;

	OptionPropertyString zoomPlus;
	OptionPropertyString zoomMinus;
	OptionPropertyString rotateHorizontal;
	OptionPropertyString rotateVertical;
	OptionPropertyString transferHorizontal;
	OptionPropertyString transferVertical;
	OptionPropertyString resetButton;
	FloatProperty zoomSensitivity;
	FloatProperty rotationSensitivity;
	FloatProperty transferSensitivity;

	const  std::vector<OptionPropertyString*>& getButtons();
	const  std::vector<Action>& getActions();
	const  std::vector<JoyAction>& getJoyActions();

private:
	std::vector<OptionPropertyString*> buttonsList;
	std::vector<Action> actions;
	std::vector<JoyAction> joyActions;
	void zoom(double value);
	void transfer(double horizontalMovement, double verticalMovement);
	void rotate(double horizontalRotation, double verticalRotation);
	void zoomIn(double value);
	void zoomOut(double value);
	void rotateHorizontally(double value);
	void rotateVertically(double value);
	void transferHorizontally(double value);
	void transferVertically(double value);
	void resetCamera(bool isHeld);

};

}  // namespace inviwo
