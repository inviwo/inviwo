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

#include <gamepad/modeproperty.h>

namespace inviwo {

	ModeProperty::ModeProperty(std::string identifier, std::string displayName)
		:CompositeProperty(identifier, displayName),
		type_("property", "Property"),
		create_("add", "Add"){

	util::for_each_type<Types>{}(TypeFunctor{}, *this);
    type_.setSelectedIndex(0);
    type_.setCurrentStateAsDefault();

    addProperty(type_);
    addProperty(create_);
	
	
	 create_.onChange([&]() {
        auto p = factory_[type_.getSelectedIndex()]();
        // make the id unique
        size_t count = 1;
        const auto& base = p->getIdentifier();
        auto id = base;
        auto displayname = base;
        while (getPropertyByIdentifier(id) != nullptr) {
            id = base + toString(count);
            displayname = base + " " + toString(count);
            ++count;
        }
        p->setIdentifier(id);
        p->setDisplayName(displayname);

        p->setSerializationMode(PropertySerializationMode::All);
        properties.push_back(static_cast<GamepadControlledProperty*>(p.get()));
        addProperty(p.release(), true);
    });
	}

	void ModeProperty::buttonPressed(std::string button,bool isHeld)
	{
		auto itProp = properties.begin();
		while (itProp != properties.end()) {
			auto buttons = (*itProp)->getButtons();
			auto itButton = buttons.begin();
			auto itAction = (*itProp)->getActions().begin();
			while (itButton != buttons.end() && itAction != (*itProp)->getActions().end()) {
				if (button == (*itButton)->getSelectedValue()) {
					(*itAction)(isHeld);
				} 
				itButton++;
				itAction++; 
			} 
			itProp++;
		}
	}

	void ModeProperty::joyStickTouched(std::string joyStick, double value)
	{
		auto itProp = properties.begin();
		while (itProp != properties.end()) {
			auto buttons = (*itProp)->getButtons();
			auto itButton = buttons.begin();
			auto itJoyAction = (*itProp)->getJoyActions().begin();
			while (itButton != buttons.end() && itJoyAction != (*itProp)->getJoyActions().end()) {
				if (joyStick == (*itButton)->getSelectedValue()) {
					(*itJoyAction)(value);
				} 
				itButton++;
				itJoyAction++; 
			} 
			itProp++;
		}
	}

}  // namespace inviwo
