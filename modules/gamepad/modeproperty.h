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
#include <gamepad/ordinalcontrolledproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <modules/gamepad/boolcontrolledproperty.h>
#include <gamepad/cameracontrolledproperty.h>

namespace inviwo {

/**
 * \brief ModeProperty defines the modes used in GamepadControlledProperty, a mode contains several GamepadControlledProperty
	with the possibility to add more
 */
class IVW_MODULE_GAMEPAD_API ModeProperty : public CompositeProperty {
public:
	using Types = std::tuple<float, double, int, bool,CameraProperty ,vec2 , vec3 , vec4 , ivec2,ivec3,ivec4 , dvec2 , dvec3, dvec4>;
    ModeProperty(std::string identifier, std::string diplayName);
    virtual ~ModeProperty() = default;
	void buttonPressed(std::string button, bool isHeld);
	void joyStickTouched(std::string joyStick, double value);
	void deserialize(Deserializer& d) override;

private:
    OptionPropertySize_t type_;
    ButtonProperty create_;


	std::vector<GamepadControlledProperty*> properties;
	std::vector<std::function<std::unique_ptr<Property>()>> factory_;

	struct TypeFunctor {
        template <typename T>
        void operator()(ModeProperty& mode) {
            mode.type_.addOption(PropertyTraits<OrdinalProperty<T>>::classIdentifier(),
                                     Defaultvalues<T>::getName(), mode.type_.size());
            mode.factory_.push_back([]() -> std::unique_ptr<Property> {
                return util::make_unique<OrdinalControlledProperty<T>>(Defaultvalues<T>::getName(),
                                                                      Defaultvalues<T>::getName());
            });
        }
		template <>
		void operator() <bool> (ModeProperty& mode) {
			mode.type_.addOption(PropertyTraits<BoolProperty>::classIdentifier(),
                                     Defaultvalues<bool>::getName(), mode.type_.size());
            mode.factory_.push_back([]() -> std::unique_ptr<Property> {
                return util::make_unique<BoolControlledProperty>(Defaultvalues<bool>::getName(),
                                                                      Defaultvalues<bool>::getName());
            });
		}
		template<>
		void operator() < CameraProperty > (ModeProperty& mode) {
			mode.type_.addOption(PropertyTraits<CameraControlledProperty>::classIdentifier(),
                                     "Camera", mode.type_.size());
            mode.factory_.push_back([]() -> std::unique_ptr<Property> {
                return util::make_unique<CameraControlledProperty>("Camera","Camera");
            });

		}
		
    };
};

}  // namespace inviwo
