/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_ORDINALPROPERTYANIMATOR_H
#define IVW_ORDINALPROPERTYANIMATOR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/timer.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>

#include <glm/vector_relational.hpp>

#include <tuple>

namespace inviwo {

class BaseOrdinalAnimationProperty : public CompositeProperty {
public:
    using CompositeProperty::CompositeProperty;
    virtual ~BaseOrdinalAnimationProperty() = default;
    virtual void update() = 0;
};

enum class BoundaryType { Stop, Periodic, Mirror };

template <typename T>
class OrdinalAnimationProperty : public BaseOrdinalAnimationProperty {
public:
    virtual std::string getClassIdentifier() const override;
    using valueType = T;

    OrdinalAnimationProperty(const std::string& identifier, const std::string& displayName);
    OrdinalAnimationProperty(const OrdinalAnimationProperty& rhs);
    OrdinalAnimationProperty& operator=(const OrdinalAnimationProperty& that) = default;
    virtual OrdinalAnimationProperty* clone() const override;
    virtual ~OrdinalAnimationProperty() = default;

    void setLimits();

    virtual void update() override;

    OrdinalProperty<T> value_;
    OrdinalProperty<T> delta_;
    TemplateOptionProperty<BoundaryType> boundary_;
    BoolProperty active_;
};

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             BoundaryType bt) {
    switch (bt) {
        case BoundaryType::Stop:
            ss << "Stop";
            break;
        case BoundaryType::Periodic:
            ss << "Periodic";
            break;
        case BoundaryType::Mirror:
            ss << "Mirror";
            break;
        default:
            break;
    }
    return ss;
}

template <typename T>
OrdinalAnimationProperty<T>::OrdinalAnimationProperty(const std::string& identifier,
                                                      const std::string& displayName)
    : BaseOrdinalAnimationProperty(identifier, displayName)
    , value_("value", "Value")
    , delta_("delta", "Delta")
    , boundary_("boundary", "Boundary",
                {BoundaryType::Stop, BoundaryType::Periodic, BoundaryType::Mirror}, 0)
    , active_("active", "Active", true) {

    addProperty(value_);
    addProperty(delta_);
    addProperty(boundary_);
    addProperty(active_);
}

template <typename T>
OrdinalAnimationProperty<T>::OrdinalAnimationProperty(const OrdinalAnimationProperty& rhs)
    : BaseOrdinalAnimationProperty(rhs)
    , value_{rhs.value_}
    , delta_{rhs.delta_}
    , boundary_{rhs.boundary_}
    , active_{rhs.active_} {

    addProperty(value_);
    addProperty(delta_);
    addProperty(boundary_);
    addProperty(active_);
}

template <typename T>
OrdinalAnimationProperty<T>* OrdinalAnimationProperty<T>::clone() const {
    return new OrdinalAnimationProperty<T>(*this);
}

template <typename T>
struct PropertyTraits<OrdinalAnimationProperty<T>> {
    static std::string classIdentifier() {
        return "org.inviwo.OrdinalAnimationProperty." + Defaultvalues<T>::getName();
    }
};

template <typename T>
std::string OrdinalAnimationProperty<T>::getClassIdentifier() const {
    return PropertyTraits<OrdinalAnimationProperty<T>>::classIdentifier();
}

namespace detail {
template <typename T, typename std::enable_if<util::rank<T>::value == 1, int>::type = 0>
T pbc(const T& val, const T& min, const T& max) {
    const auto minmask = glm::lessThan(val, min);
    const auto maxmask = glm::greaterThanEqual(val, max);
    const auto range = max - min;
    return val + T{minmask} * range - T{maxmask} * range;
}
template <typename T, typename std::enable_if<util::rank<T>::value == 0, int>::type = 0>
T pbc(const T& val, const T& min, const T& max) {
    if (val < min) {
        return val + (max - min);
    } else if (val >= max) {
        return val - (max - min);
    } else {
        return val;
    }
}

template <typename T, typename std::enable_if<util::rank<T>::value == 1, int>::type = 0>
T mirror(const T& val, const T& min, const T& max) {
    const auto minmask = glm::lessThan(val, min);
    const auto maxmask = glm::greaterThanEqual(val, max);
    const auto mask = minmask || maxmask;
    return T{-1} * T{mask} + (T{1} - T{mask});
}
template <typename T, typename std::enable_if<util::rank<T>::value == 0, int>::type = 0>
T mirror(const T& val, const T& min, const T& max) {
    if (val < min || val > max) {
        return T{-1};
    } else {
        return T{1};
    }
}

}  // namespace detail

template <typename T>
void OrdinalAnimationProperty<T>::update() {
    if (!active_) return;

    T p = value_.get();
    T d = delta_.get();
    T r = p + d;

    switch (boundary_.get()) {
        case BoundaryType::Stop:
            r = glm::clamp(r, value_.getMinValue(), value_.getMaxValue());
            break;
        case BoundaryType::Periodic:
            r = detail::pbc(r, value_.getMinValue(), value_.getMaxValue());
            break;
        case BoundaryType::Mirror: {
            auto m = detail::mirror(r, value_.getMinValue(), value_.getMaxValue());
            delta_.set(m * delta_.get());
            r = glm::clamp(r, value_.getMinValue(), value_.getMaxValue());
            break;
        }
        default:
            break;
    }
    if (r != p) {
        value_.set(r);
    }
}

template <typename T>
void OrdinalAnimationProperty<T>::setLimits() {
    using P = typename util::value_type<T>::type;

    T max = value_.getMaxValue();

    T dmin = delta_.getMinValue();
    T dmax = delta_.getMaxValue();

    T newMin = -T{static_cast<P>(0.1)} * max;
    T newMax = T{static_cast<P>(0.1)} * max;

    if (dmin != newMin) {
        delta_.setMinValue(newMin);
    }
    if (dmax != newMax) {
        delta_.setMaxValue(newMax);
    }
}

/** \docpage{org.inviwo.OrdinalPropertyAnimator, Property Animator}
 * ![](org.inviwo.OrdinalPropertyAnimator.png?classIdentifier=org.inviwo.OrdinalPropertyAnimator)
 * Generate a periodically uppdating property. Usefull for animations
 *
 * ### Properties
 *   * __Property__ The type of ordinal property
 *   * __Add__ Add property.
 *   * __Delay (ms)__ elay between updates.
 *   * __Play/Pause__ Activate animation.
 *
 */
class IVW_MODULE_BASE_API OrdinalPropertyAnimator : public Processor {
public:
    using Types =
        std::tuple<float, vec2, vec3, vec4, double, dvec2, dvec3, dvec4, int, ivec2, ivec3, ivec4>;

    OrdinalPropertyAnimator();
    virtual ~OrdinalPropertyAnimator() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void deserialize(Deserializer& d) override;

private:
    OptionPropertySize_t type_;
    ButtonProperty create_;
    IntProperty delay_;
    ButtonProperty play_;
    Timer timer_;

    std::vector<std::function<std::unique_ptr<Property>()>> factory_;
    std::vector<BaseOrdinalAnimationProperty*> props_;

    struct TypeFunctor {
        template <typename T>
        void operator()(OrdinalPropertyAnimator& animator) {
            animator.type_.addOption(PropertyTraits<OrdinalProperty<T>>::classIdentifier(),
                                     Defaultvalues<T>::getName(), animator.type_.size());
            animator.factory_.push_back([]() -> std::unique_ptr<Property> {
                return std::make_unique<OrdinalAnimationProperty<T>>(Defaultvalues<T>::getName(),
                                                                     Defaultvalues<T>::getName());
            });
        }
    };
};

}  // namespace inviwo

#endif  // IVW_ORDINALPROPERTYANIMATOR_H
