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

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>

#include <inviwo/gamepad/gamepadmoduledefine.h>

namespace inviwo {

/** \brief Reflects the readout from a gamepad.

    This contains all properties that can be read from QGamepad.
    Processors can use this if they want to receive gamepad
    input from a GamepadInput processor through a property link.

    @see GamepadInput

    @author Tino Weinkauf
*/
class IVW_MODULE_GAMEPAD_API GamepadValuesProperty : public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    GamepadValuesProperty(std::string identifier, std::string displayName,
                          InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                          PropertySemantics semantics = PropertySemantics::Default);

    GamepadValuesProperty(const GamepadValuesProperty& rhs);

    virtual GamepadValuesProperty* clone() const override;

    virtual ~GamepadValuesProperty();

    // Methods
public:
    /// Our content shall be read only, but it should be possible to rename this property itself.
    GamepadValuesProperty& setReadOnlyContent(bool value);

private:
    void setupProperties();

    // Properties
public:
    FloatProperty propAxisLeftX;
    FloatProperty propAxisLeftY;
    FloatProperty propAxisRightX;
    FloatProperty propAxisRightY;
    FloatProperty propButtonL2;
    FloatProperty propButtonR2;

    BoolProperty propButtonA;
    BoolProperty propButtonB;
    BoolProperty propButtonX;
    BoolProperty propButtonY;
    BoolProperty propButtonLeft;
    BoolProperty propButtonRight;
    BoolProperty propButtonUp;
    BoolProperty propButtonDown;
    BoolProperty propButtonL1;
    BoolProperty propButtonR1;
    BoolProperty propButtonL3;
    BoolProperty propButtonR3;
    // BoolProperty propButtonSelect; //We keep this exclusive for GamepadInput for switching
    // between groups.
    BoolProperty propButtonStart;
    BoolProperty propButtonCenter;
    BoolProperty propButtonGuide;
};

}  // namespace inviwo
