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

#include <gamepad/boolcontrolledproperty.h>

namespace inviwo {

BoolControlledProperty::BoolControlledProperty(std::string identifier, std::string displayName) 
	:GamepadControlledProperty(identifier, displayName)
	,controlledProperty("boolProperty", "Property")
	,buttonController("buttonController","Switch Button")
{
	std::list<std::string> buttons{"A","B","X","Y","R1","L1","Left","Right","Up","Down","L3","R3"};
	for each (std::string button in buttons)
	{
		buttonController.addOption(button,button);
	}
	addProperty(controlledProperty);
	addProperty(buttonController);
	buttonsList.push_back(&buttonController);
	actions.push_back(std::bind(&BoolControlledProperty::changeState, this,std::placeholders::_1));
	setCurrentStateAsDefault();
}

const std::vector<OptionPropertyString*>& BoolControlledProperty::getButtons()
{
	return buttonsList;
}

const std::vector<GamepadControlledProperty::Action>& BoolControlledProperty::getActions()
{
	return actions;
}

const std::vector<GamepadControlledProperty::JoyAction>& BoolControlledProperty::getJoyActions()
{
	return {};
}

void BoolControlledProperty::changeState(bool isHeld)
{
	if (!isHeld) {
		controlledProperty = !controlledProperty.get();
	}
}

}  // namespace inviwo
