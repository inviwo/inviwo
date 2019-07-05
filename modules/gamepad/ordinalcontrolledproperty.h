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
#include <modules/gamepad/gamepadcontrolledproperty.h>

namespace inviwo {

/**
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS_FROM_A_DEVELOPER_PERSPECTIVE
 */
template <typename T>
class IVW_MODULE_GAMEPAD_API OrdinalControlledProperty : public GamepadControlledProperty
{
public:
    OrdinalControlledProperty(std::string identifier,std::string displayName);
    virtual ~OrdinalControlledProperty() = default;
	std::vector<OptionPropertyString*>& getButtons();
	std::vector<Action>& getActions();
	std::vector<JoyAction>& getJoyActions();

	OrdinalProperty<T> controlledProperty;
	OptionPropertyString buttonPlus;
	OptionPropertyString buttonMinus;
	OrdinalProperty<T> sensitivity;
	
	

private:
	std::vector<OptionPropertyString*> buttonsList;
	std::vector<Action> actions;
	std::vector<JoyAction> joyActions;
	void incrementProperty(bool isHeld);
	void decrementProperty(bool isHeld);
	void modifyProperty(double value);
};

template <typename T>
OrdinalControlledProperty<T>::OrdinalControlledProperty(std::string identifier,std::string displayName) 
	: GamepadControlledProperty(identifier,displayName)
	,controlledProperty("controlledProperty","Controlled Property")
	,sensitivity("sensitivity","Sensitivity")
	,buttonPlus("buttonPlus", "Increase Button")
	,buttonMinus("buttonMinus", "Decrease Button")
	{
	std::list<std::string> buttons{"A","B","X","Y","R1","L1","R2","L2","Left","Right","Up","Down","L3","R3"
		,"Left Joystick X", "Right Joystick X","Left Joystick Y", "Right Joystick Y"};
	for each (std::string button in buttons)
	{
		buttonPlus.addOption(button,button);
		buttonMinus.addOption(button,button);
	}
	buttonMinus.setSelectedValue("B");
	addProperty(controlledProperty);
	addProperty(buttonPlus);
	addProperty(buttonMinus);
	addProperty(sensitivity);
	actions.push_back(std::bind(&OrdinalControlledProperty<T>::incrementProperty, this,std::placeholders::_1));
	actions.push_back(std::bind(&OrdinalControlledProperty<T>::decrementProperty,this,std::placeholders::_1));
	joyActions.push_back(std::bind(&OrdinalControlledProperty<T>::modifyProperty, this, std::placeholders::_1));
	buttonsList.push_back(&buttonPlus);
	buttonsList.push_back(&buttonMinus);

}

template<typename T>
inline void OrdinalControlledProperty<T>::incrementProperty(bool isHeld)
{
	if (isHeld) {
		controlledProperty = controlledProperty + sensitivity;
	}
}

template<typename T>
inline void OrdinalControlledProperty<T>::decrementProperty(bool isHeld)
{
	if (isHeld) {
		controlledProperty = controlledProperty - sensitivity;
	}
}

template<typename T>
inline void OrdinalControlledProperty<T>::modifyProperty(double value)
{
	controlledProperty = controlledProperty + value*sensitivity;
}

template<typename T>
inline std::vector<OptionPropertyString*>& OrdinalControlledProperty<T>::getButtons() 
{
	return buttonsList;
}

template<typename T>
inline std::vector<GamepadControlledProperty::Action>& OrdinalControlledProperty<T>::getActions() 
{
	return actions;
}

template<typename T>
inline std::vector<GamepadControlledProperty::JoyAction>& OrdinalControlledProperty<T>::getJoyActions()
{
	return joyActions;
}


}  // namespace inviwo
