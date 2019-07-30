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

#include <modules/gamepad/cameracontrolledproperty.h>

namespace inviwo {

CameraControlledProperty::CameraControlledProperty(std::string identifier, std::string displayName)
	:GamepadControlledProperty(identifier, displayName), 
	controlledProperty("controlledProperty","ControlledProperty"),
	zoomPlus("zoomPlus","ZoomIn Button"),
	zoomMinus("zoomMinus","ZoomOut Button"),
	rotateHorizontal("rotateHorizontal","Horizontal Orientation Joystick Axis"),
	rotateVertical("rotateVertical","Vertical Rotation Joystick Axis"),
	transferHorizontal("transferHorizontal","Horizontal Transfer Joystick Axis"),
	transferVertical("transferVertical", "Vertical Transfer Joystick Axis"),
	resetButton("resetButton", "resetCamera"),
	rotationSensitivity("rotationSensitivity", "Rotation Sensitivity",0.1,0.001,1),
	transferSensitivity("movementSensitivity", "Movement Sensitivity",0.1,0.001,1),
	zoomSensitivity("zoomSensitivity", "Zoom Sensitivity",1)
{
	std::list<std::string> Joysticks{ "Left Joystick X","Left Joystick Y","Right Joystick X", "Right Joystick Y" };
	std::list<std::string> Triggers{ "R2","L2" };
	std::list<std::string> Buttons{ "A","B","X","Y","R1","L1","Left","Right","Up","Down","L3","R3" };
	for each (std::string joystick in Joysticks)
	{
		rotateHorizontal.addOption(joystick, joystick);
		rotateVertical.addOption(joystick, joystick);
		transferHorizontal.addOption(joystick, joystick);
		transferVertical.addOption(joystick, joystick);
	}
	for each (std::string trigger in Triggers)
	{
		zoomPlus.addOption(trigger, trigger);
		zoomMinus.addOption(trigger, trigger);
	}
	for each (std::string button in Buttons)
	{
		resetButton.addOption(button,button);
	}
	rotateHorizontal.setSelectedValue("Left Joystick X");
	rotateVertical.setSelectedValue("Left Joystick Y");
	transferHorizontal.setSelectedValue("Right Joystick X");
	transferVertical.setSelectedValue("Right Joystick Y");
	zoomPlus.setSelectedValue("R2");
	zoomMinus.setSelectedValue("L2");
	resetButton.setSelectedValue("R1");

	buttonsList.push_back(&rotateHorizontal);
	buttonsList.push_back(&rotateVertical);
	buttonsList.push_back(&transferHorizontal);
	buttonsList.push_back(&transferVertical);
	buttonsList.push_back(&zoomPlus);
	buttonsList.push_back(&zoomMinus);
	buttonsList.push_back(&resetButton);


	//TODO Very dirty hotfix so reset camera works, needs to be changed
	for (size_t i = 0; i <7; i++)
	{
			actions.push_back(std::bind(&CameraControlledProperty::resetCamera, this,std::placeholders::_1));
	}
	joyActions.push_back(std::bind(&CameraControlledProperty::rotateHorizontally,this,std::placeholders::_1));
	joyActions.push_back(std::bind(&CameraControlledProperty::rotateVertically,this,std::placeholders::_1));
	joyActions.push_back(std::bind(&CameraControlledProperty::transferHorizontally,this,std::placeholders::_1));
	joyActions.push_back(std::bind(&CameraControlledProperty::transferVertically,this,std::placeholders::_1));
	joyActions.push_back(std::bind(&CameraControlledProperty::zoomIn,this,std::placeholders::_1));
	joyActions.push_back(std::bind(&CameraControlledProperty::zoomOut,this,std::placeholders::_1));

	addProperties(controlledProperty
		,zoomPlus, zoomMinus, zoomSensitivity
		, rotateHorizontal, rotateVertical, rotationSensitivity
		, transferHorizontal, transferVertical, transferSensitivity
		, resetButton);


}

const std::vector<OptionPropertyString*>& CameraControlledProperty::getButtons()
{
	return buttonsList;
}

const std::vector<GamepadControlledProperty::Action>& CameraControlledProperty::getActions()
{
	return actions;
}

const std::vector<GamepadControlledProperty::JoyAction>& CameraControlledProperty::getJoyActions()
{
	return joyActions;
}

void CameraControlledProperty::zoom(double value)
{
	auto from = controlledProperty.getLookFrom();
	float oldR = sqrtf(from.x * from.x + from.y * from.y + from.z * from.z);
	float newR = oldR + value*zoomSensitivity;
	float ratio = oldR / newR;
	controlledProperty.setLookFrom(
		vec3(from.x* ratio, from.y* ratio, from.z* ratio)
	); 
}

void CameraControlledProperty::transfer(double horizontalMovement, double verticalMovement)
{
	const vec3 right = glm::normalize(glm::cross(controlledProperty.getLookTo() - controlledProperty.getLookFrom(), controlledProperty.getLookUp()));
    controlledProperty.setLook(controlledProperty.getLookFrom() - horizontalMovement*transferSensitivity * right + verticalMovement*transferSensitivity * controlledProperty.getLookUp(),
    controlledProperty.getLookTo() - horizontalMovement*transferSensitivity * right + verticalMovement*transferSensitivity * controlledProperty.getLookUp(), controlledProperty.getLookUp());
}

void CameraControlledProperty::rotate(double horizontalRotation, double verticalRotation)
{
	    const auto& to = controlledProperty.getLookTo();
    const auto& from = controlledProperty.getLookFrom();
    const auto& up = controlledProperty.getLookUp();

        // Compute coordinates on a sphere to rotate from and to
            const auto rot = glm::half_pi<float>() * (vec3(horizontalRotation*rotationSensitivity,verticalRotation*rotationSensitivity,0));
            const auto Pa = glm::normalize(from - to);
            const auto Pc = glm::rotate(glm::rotate(Pa, rot.y, glm::cross(Pa, up)), rot.x, up);
            auto lastRot = glm::quat(Pc, Pa);

        controlledProperty.setLook(to + glm::rotate(lastRot, from - to), to, glm::rotate(lastRot, up));
}

void CameraControlledProperty::zoomIn(double value)
{
	zoom(value*zoomSensitivity);
}

void CameraControlledProperty::zoomOut(double value)
{
	zoom(-value*zoomSensitivity);
}

void CameraControlledProperty::rotateHorizontally(double value)
{
	rotate(value, 0);
}

void CameraControlledProperty::rotateVertically(double value)
{
	rotate(0, value);
}

void CameraControlledProperty::transferHorizontally(double value)
{
	transfer(value, 0);
}

void CameraControlledProperty::transferVertically(double value)
{
	transfer(0,value);
}

void CameraControlledProperty::resetCamera(bool isHeld)
{
	if (!isHeld) {
		controlledProperty.resetCamera();
	}
}

}  // namespace inviwo
