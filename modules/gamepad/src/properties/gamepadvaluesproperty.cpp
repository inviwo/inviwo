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

#include <inviwo/gamepad/properties/gamepadvaluesproperty.h>

namespace inviwo {

const std::string GamepadValuesProperty::classIdentifier = "org.inviwo.GamepadValuesProperty";
std::string GamepadValuesProperty::getClassIdentifier() const { return classIdentifier; }

GamepadValuesProperty::GamepadValuesProperty(std::string identifier, std::string displayName,
                                             InvalidationLevel invalidationLevel,
                                             PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , propAxisLeftX("axisLeftX", "Left X", 0, -1, 1, 0.01f)
    , propAxisLeftY("axisLeftY", "Left Y", 0, -1, 1, 0.01f)
    , propAxisRightX("axisRightX", "Right X", 0, -1, 1, 0.01f)
    , propAxisRightY("axisRightY", "Right Y", 0, -1, 1, 0.01f)
    , propButtonL2("buttonL2", "Left Trigger", 0, 0, 1, 0.01f)
    , propButtonR2("buttonR2", "Right Trigger", 0, 0, 1, 0.01f)
    , propButtonA("buttonA", "A")
    , propButtonB("buttonB", "B")
    , propButtonX("buttonX", "X")
    , propButtonY("buttonY", "Y")
    , propButtonLeft("buttonLeft", "DPAD Left")
    , propButtonRight("buttonRight", "DPAD Right")
    , propButtonUp("buttonUp", "DPAD Up")
    , propButtonDown("buttonDown", "DPAD Down")
    , propButtonL1("buttonL1", "Left Button")
    , propButtonR1("buttonR1", "Right Button")
    , propButtonL3("buttonL3", "Left Stick")
    , propButtonR3("buttonR3", "Right Stick")
    //, propButtonSelect("buttonSelect", "Select")
    , propButtonStart("buttonStart", "Start")
    , propButtonCenter("buttonCenter", "Center")
    , propButtonGuide("buttonGuide", "Guide") {
    setupProperties();
}

GamepadValuesProperty::GamepadValuesProperty(const GamepadValuesProperty& rhs)
    : CompositeProperty(rhs)
    , propAxisLeftX(rhs.propAxisLeftX)
    , propAxisLeftY(rhs.propAxisLeftY)
    , propAxisRightX(rhs.propAxisRightX)
    , propAxisRightY(rhs.propAxisRightY)
    , propButtonL2(rhs.propButtonL2)
    , propButtonR2(rhs.propButtonR2)
    , propButtonA(rhs.propButtonA)
    , propButtonB(rhs.propButtonB)
    , propButtonX(rhs.propButtonX)
    , propButtonY(rhs.propButtonY)
    , propButtonLeft(rhs.propButtonLeft)
    , propButtonRight(rhs.propButtonRight)
    , propButtonUp(rhs.propButtonUp)
    , propButtonDown(rhs.propButtonDown)
    , propButtonL1(rhs.propButtonL1)
    , propButtonR1(rhs.propButtonR1)
    , propButtonL3(rhs.propButtonL3)
    , propButtonR3(rhs.propButtonR3)
    //, propButtonSelect(rhs.propButtonSelect)
    , propButtonStart(rhs.propButtonStart)
    , propButtonCenter(rhs.propButtonCenter)
    , propButtonGuide(rhs.propButtonGuide) {
    setupProperties();
}

GamepadValuesProperty* GamepadValuesProperty::clone() const {
    return new GamepadValuesProperty(*this);
}

GamepadValuesProperty::~GamepadValuesProperty() {}

GamepadValuesProperty& GamepadValuesProperty::setReadOnlyContent(bool value) {
    for (auto& elem : properties_) {
        elem->setReadOnly(value);
    }
    return *this;
}

void GamepadValuesProperty::setupProperties() {
    // Add them all in what I consider a reasonable order
    addProperties(propAxisLeftX, propAxisLeftY, propAxisRightX, propAxisRightY, propButtonL2,
                  propButtonR2, propButtonA, propButtonB, propButtonX, propButtonY, propButtonLeft,
                  propButtonRight, propButtonUp, propButtonDown, propButtonL1, propButtonR1,
                  propButtonL3, propButtonR3,
                  /*propButtonSelect,*/ propButtonStart, propButtonCenter, propButtonGuide);
}

}  // namespace inviwo
