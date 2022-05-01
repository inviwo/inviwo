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

#include <inviwo/gamepad/gamepadmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/gamepad/properties/gamepadvaluesproperty.h>
#include <inviwo/gamepad/properties/gamepadtextproperty.h>

#include <string>
#include <tuple>
#include <functional>

namespace inviwo {

/// Base class for property for mapping an ordinal to gamepad axes.
class IVW_MODULE_GAMEPAD_API BaseOrdinalGamepadProperty : public CompositeProperty {
public:
    using CompositeProperty::CompositeProperty;
    virtual ~BaseOrdinalGamepadProperty() = default;

    /// Adds its description to the given string.
    virtual void GetDescription(std::string& Description) = 0;

    /// Updates the ordinal value.
    virtual void ContinuousUpdate(const GamepadValuesProperty& gvp, const float fSensitivityFactor,
                                  const float iSensitivityFactor) = 0;
    // virtual void DiscreteUpdate(const float DPADXValue, const float DPADYValue) = 0;
};

enum class GamepadOrdinalAxes {
    AxisLeftX,
    AxisLeftY,
    AxisRightX,
    AxisRightY,
    ButtonLR2,
    DPADX,
    DPADY
};

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             GamepadOrdinalAxes goa) {
    switch (goa) {
        case GamepadOrdinalAxes::AxisLeftX:
            ss << "Left X";
            break;
        case GamepadOrdinalAxes::AxisLeftY:
            ss << "Left Y";
            break;
        case GamepadOrdinalAxes::AxisRightX:
            ss << "Right X";
            break;
        case GamepadOrdinalAxes::AxisRightY:
            ss << "Right Y";
            break;
        case GamepadOrdinalAxes::ButtonLR2:
            ss << "Triggers";
            break;
        case GamepadOrdinalAxes::DPADX:
            ss << "DPAD X";
            break;
        case GamepadOrdinalAxes::DPADY:
            ss << "DPAD Y";
            break;
        default:
            break;
    }
    return ss;
}

/// Property for mapping an ordinal to gamepad axes.
template <typename T>
class OrdinalGamepadProperty : public BaseOrdinalGamepadProperty {
    // Friends / Types
public:
public:
    virtual std::string getClassIdentifier() const override;
    using valueType = T;

    OrdinalGamepadProperty(const std::string& identifier, const std::string& displayName);
    OrdinalGamepadProperty(const OrdinalGamepadProperty& rhs);
    OrdinalGamepadProperty& operator=(const OrdinalGamepadProperty& that) = default;
    virtual OrdinalGamepadProperty* clone() const override;
    virtual ~OrdinalGamepadProperty() = default;

    /// Adds its description to the given string.
    virtual void GetDescription(std::string& Description) override;

    /// Continuously called by a function triggered itself by a timer
    virtual void ContinuousUpdate(const GamepadValuesProperty& gvp, const float fSensitivityFactor,
                                  const float iSensitivityFactor) override;

    /////Only called when a button on the DPAD is pressed
    // virtual void DiscreteUpdate(const float DPADXValue, const float DPADYValue);

    OrdinalProperty<T> propValue;
};

template <typename T>
OrdinalGamepadProperty<T>::OrdinalGamepadProperty(const std::string& identifier,
                                                  const std::string& displayName)
    : BaseOrdinalGamepadProperty(identifier, displayName), propValue("value", "Value") {
    addProperty(propValue);

    // Number of elements in our type (e.g., vec3 has 3 floats)
    constexpr size_t nelem = util::flat_extent<T>::value;

    // Create as many dropdown lists to select a gamepad axis per element
    for (size_t i(0); i < nelem; i++) {
        const std::string Identifier = "axis" + std::to_string(i);
        const std::string DisplayName = "Axis " + std::to_string(i);

        const std::vector<GamepadOrdinalAxes> AllAxes = {
            GamepadOrdinalAxes::AxisLeftX,  GamepadOrdinalAxes::AxisLeftY,
            GamepadOrdinalAxes::AxisRightX, GamepadOrdinalAxes::AxisRightY,
            GamepadOrdinalAxes::ButtonLR2,  GamepadOrdinalAxes::DPADX,
            GamepadOrdinalAxes::DPADY};

        const TemplateOptionProperty<GamepadOrdinalAxes> temp(Identifier, DisplayName, AllAxes, 0);
        addProperty(std::make_unique<TemplateOptionProperty<GamepadOrdinalAxes>>(temp));
    }
}

template <typename T>
OrdinalGamepadProperty<T>::OrdinalGamepadProperty(const OrdinalGamepadProperty& rhs)
    : BaseOrdinalGamepadProperty(rhs), propValue{rhs.propValue} {
    addProperty(propValue);
}

template <typename T>
OrdinalGamepadProperty<T>* OrdinalGamepadProperty<T>::clone() const {
    return new OrdinalGamepadProperty<T>(*this);
}

template <typename T>
struct PropertyTraits<OrdinalGamepadProperty<T>> {
    static std::string classIdentifier() {
        return "org.inviwo.OrdinalGamepadProperty." + Defaultvalues<T>::getName();
    }
};

template <typename T>
std::string OrdinalGamepadProperty<T>::getClassIdentifier() const {
    return PropertyTraits<OrdinalGamepadProperty<T>>::classIdentifier();
}

template <typename T>
void inviwo::OrdinalGamepadProperty<T>::GetDescription(std::string& Description) {
    std::ostringstream Text;
    Text << "\n" << getDisplayName() << ":\n";

    for (auto itProp = cbegin(); itProp != cend(); itProp++) {
        auto pAxisProp = dynamic_cast<TemplateOptionProperty<GamepadOrdinalAxes>*>(*itProp);
        if (pAxisProp) {
            GamepadOrdinalAxes SelectedAxis = pAxisProp->getSelectedValue();
            Text << SelectedAxis << "\n";
        }
    }

    Description += Text.str();
}

template <typename T>
void OrdinalGamepadProperty<T>::ContinuousUpdate(const GamepadValuesProperty& gvp,
                                                 const float fSensitivityFactor,
                                                 const float iSensitivityFactor) {
    // Number of elements in our type (e.g., vec3 has 3 floats)
    constexpr size_t nelem = util::flat_extent<T>::value;

    // Collect the input from the gamepad
    T IncrementFactor(0);
    size_t idx(0);
    for (auto itProp = cbegin(); itProp != cend(); itProp++) {
        auto pAxisProp = dynamic_cast<TemplateOptionProperty<GamepadOrdinalAxes>*>(*itProp);
        if (pAxisProp) {
            float ReadValue(0);
            GamepadOrdinalAxes SelectedAxis = pAxisProp->getSelectedValue();
            switch (SelectedAxis) {
                case GamepadOrdinalAxes::AxisLeftX:
                    ReadValue = gvp.propAxisLeftX.get() * fSensitivityFactor;
                    break;
                case GamepadOrdinalAxes::AxisLeftY:
                    ReadValue = -gvp.propAxisLeftY.get() * fSensitivityFactor;
                    break;
                case GamepadOrdinalAxes::AxisRightX:
                    ReadValue = gvp.propAxisRightX.get() * fSensitivityFactor;
                    break;
                case GamepadOrdinalAxes::AxisRightY:
                    ReadValue = -gvp.propAxisRightY.get() * fSensitivityFactor;
                    break;
                case GamepadOrdinalAxes::ButtonLR2:
                    ReadValue =
                        (gvp.propButtonR2.get() - gvp.propButtonL2.get()) * fSensitivityFactor;
                    break;
                case GamepadOrdinalAxes::DPADX:
                    ReadValue = ((float)gvp.propButtonRight - (float)gvp.propButtonLeft) *
                                iSensitivityFactor;
                    break;
                case GamepadOrdinalAxes::DPADY:
                    ReadValue =
                        ((float)gvp.propButtonUp - (float)gvp.propButtonDown) * iSensitivityFactor;
                    break;
                    // default:
                    //    ReadValue = 0;
            }

            assert(idx < nelem);
            util::glmcomp(IncrementFactor, idx) = (util::value_type<T>::type)ReadValue;
            idx++;
        }
    }

    // Get, increment, set the value
    if (IncrementFactor != T(0)) {
        T Value = propValue.get();
        Value += IncrementFactor * propValue.getIncrement();
        propValue.set(Value);
    }
}

/*
 * It is interesting to consider a discrete update per button press,
 * but it turned out to be tedious as a user. The code should remain
 * in case we want this back.
 */

// template <typename T>
// void OrdinalGamepadProperty<T>::DiscreteUpdate(const float DPADXValue, const float DPADYValue)
//{
//    //Number of elements in our type (e.g., vec3 has 3 floats)
//    constexpr size_t nelem = util::flat_extent<T>::value;
//
//    //Collect the input from the gamepad
//    T IncrementFactor(0);
//    size_t idx(0);
//    for(auto itProp = cbegin();itProp!=cend();itProp++)
//    {
//        auto pAxisProp = dynamic_cast<TemplateOptionProperty<GamepadOrdinalAxes>*>(*itProp);
//        if (pAxisProp)
//        {
//            float ReadValue(0);
//            GamepadOrdinalAxes SelectedAxis = pAxisProp->getSelectedValue();
//            switch (SelectedAxis)
//            {
//                case GamepadOrdinalAxes::DPADX:
//                    ReadValue = DPADXValue;
//                    break;
//                case GamepadOrdinalAxes::DPADY:
//                    ReadValue = DPADYValue;
//                    break;
//                //default:
//                //    ReadValue = 0;
//            }
//
//            assert(idx < nelem);
//            util::glmcomp(IncrementFactor, idx) = (util::value_type<T>::type)ReadValue;
//            idx++;
//        }
//    }
//
//    //Get, increment, set the value
//    T Value = propValue.get();
//    Value += IncrementFactor * propValue.getIncrement();
//    propValue.set(Value);
//}

/** \docpage{org.inviwo.GamepadOrdinals, Gamepad Ordinals}
 * ![](org.inviwo.GamepadOrdinals.png?classIdentifier=org.inviwo.GamepadOrdinals)
 *
 * Allows to steer ordinal values with a gamepad.
 *
 * ### Properties
 *   * __Frames per Second__ How often an update shall occur.
 *   * __Sensitivity__ How much the values should change per update. This can be increased and
 * decreased with X and B.
 *   * __Mappings__ Lists several ordinals with the possibility to map them to an axis.
 *   * __Gamepad__ Connection to a GamepadInput module.
 *   * __Help and Notifications__ Help texts and notifications.
 *
 */
class IVW_MODULE_GAMEPAD_API GamepadOrdinals : public Processor {
    // Types / Friends
public:
    using Types =
        std::tuple<float, vec2, vec3, vec4, double, dvec2, dvec3, dvec4, int, ivec2, ivec3, ivec4>;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    static const std::string helpText_;

    // Constructor/Destructor
public:
    GamepadOrdinals();
    virtual ~GamepadOrdinals() override = default;

    // Methods
protected:
    /// Called by the timer. Updating numbers happens here.
    void Tick();

    /////Called after pressing the DPAD. Updating numbers happens here as well.
    // void DPADPressed(const float x, const float y);

private:
    /// Helper to add prefabs to the ListProperty propMappings for many different ordinal types.
    struct TypeFunctor {
        template <typename T>
        void operator()(GamepadOrdinals& GPOrdProcessor) {
            // Defaultvalues<T>::getName() would lead to something like "FloatVec3"
            // - the final digit would be changed by ListProperty
            // PropertyTraits<OrdinalProperty<T>>::classIdentifier() leads to
            // "org.inviwo.FloatVec3Property"

            std::string Basename = PropertyTraits<OrdinalProperty<T>>::classIdentifier();
            size_t idLastDot = Basename.rfind('.');
            assert(idLastDot != std::string::npos);
            assert(idLastDot + 1 < Basename.length());
            Basename = Basename.substr(idLastDot + 1);
            std::string Name = Basename;
            std::string Ident = toLower(Basename) + "0";

            // Add the prefab to the ListProperty
            GPOrdProcessor.propMappings.addPrefab(
                std::make_unique<OrdinalGamepadProperty<T>>(Ident, Name));
        }
    };

    // Attributes
public:
    /// Update speed.
    /// This is the desired frames per second.
    /// We will query the gamepad and update the values as often as this property suggests and the
    /// system allows.
    IntSizeTProperty propFPS;

    /// How strong we change the values in one step.
    /// Zero means, we do not change the values at all.
    /// One means, we change the values by 1,
    /// which means something different for each ordinal.
    FloatProperty propSensitivity;

    /// The user adds properties here and maps them to axes on the gamepad.
    ListProperty propMappings;

    /// Connection to the gamepad
    GamepadValuesProperty propGamepad;

    /// For help and notifications.
    GamepadTextProperty propText;

private:
    /// Calls the Tick() function in regular intervals.
    Timer Ticker;
};

}  // namespace inviwo
