/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/constraintbehavior.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmfmt.h>
#include <inviwo/core/util/glmcomp.h>
#include <inviwo/core/util/glmmatext.h>

#include <string>
#include <type_traits>

#include <fmt/core.h>
#include <ostream>

namespace inviwo {

/**
 * A helper struct to construct ordinal properties @see OrdinalProperty
 */
template <typename T>
struct OrdinalPropertyState {
    T value{};
    T min = Defaultvalues<T>::getMin();
    ConstraintBehavior minConstraint = ConstraintBehavior::Editable;
    T max = Defaultvalues<T>::getMax();
    ConstraintBehavior maxConstraint = ConstraintBehavior::Editable;
    T increment = Defaultvalues<T>::getInc();
    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput;
    PropertySemantics semantics = defaultSemantics();
    Document help = {};

    auto set(T newValue) -> OrdinalPropertyState {
        value = newValue;
        return *this;
    }
    auto setMin(T newMin) -> OrdinalPropertyState {
        min = newMin;
        return *this;
    }
    auto setMin(ConstraintBehavior newMinConstraint) -> OrdinalPropertyState {
        minConstraint = newMinConstraint;
        return *this;
    }
    auto setMax(T newMax) -> OrdinalPropertyState {
        max = newMax;
        return *this;
    }
    auto setMax(ConstraintBehavior newMaxConstraint) -> OrdinalPropertyState {
        maxConstraint = newMaxConstraint;
        return *this;
    }
    auto setInc(T newIncrement) -> OrdinalPropertyState {
        increment = newIncrement;
        return *this;
    }
    auto set(InvalidationLevel newInvalidationLevel) -> OrdinalPropertyState {
        invalidationLevel = newInvalidationLevel;
        return *this;
    }
    auto set(PropertySemantics newSemantics) -> OrdinalPropertyState {
        semantics = newSemantics;
        return *this;
    }
    auto set(Document newHelp) -> OrdinalPropertyState {
        help = newHelp;
        return *this;
    }

    static PropertySemantics defaultSemantics() {
        if constexpr (util::extent<T, 1>::value > 1) {
            return PropertySemantics::Text;
        } else {
            return PropertySemantics::Default;
        }
    }
};

/**
 * \ingroup properties
 * A property representing an Ordinal value, for example int, floats.
 */
template <typename T>
class OrdinalProperty : public Property {
public:
    using value_type = T;
    using component_type = typename util::value_type<T>::type;

    OrdinalProperty(
        std::string_view identifier, std::string_view displayName, Document help,
        const T& value = Defaultvalues<T>::getVal(),
        const std::pair<T, ConstraintBehavior>& minValue = std::pair{Defaultvalues<T>::getMin(),
                                                                     ConstraintBehavior::Editable},
        const std::pair<T, ConstraintBehavior>& maxValue = std::pair{Defaultvalues<T>::getMax(),
                                                                     ConstraintBehavior::Editable},
        const T& increment = Defaultvalues<T>::getInc(),
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
        PropertySemantics semantics = OrdinalPropertyState<T>::defaultSemantics());

    OrdinalProperty(
        std::string_view identifier, std::string_view displayName,
        const T& value = Defaultvalues<T>::getVal(),
        const std::pair<T, ConstraintBehavior>& minValue = std::pair{Defaultvalues<T>::getMin(),
                                                                     ConstraintBehavior::Editable},
        const std::pair<T, ConstraintBehavior>& maxValue = std::pair{Defaultvalues<T>::getMax(),
                                                                     ConstraintBehavior::Editable},
        const T& increment = Defaultvalues<T>::getInc(),
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
        PropertySemantics semantics = OrdinalPropertyState<T>::defaultSemantics());

    OrdinalProperty(std::string_view identifier, std::string_view displayName, const T& value,
                    const T& minValue, const T& maxValue = Defaultvalues<T>::getMax(),
                    const T& increment = Defaultvalues<T>::getInc(),
                    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                    PropertySemantics semantics = OrdinalPropertyState<T>::defaultSemantics());

    OrdinalProperty(std::string_view identifier, std::string_view displayName,
                    OrdinalPropertyState<T> state);

    OrdinalProperty(const OrdinalProperty<T>& rhs);
    OrdinalProperty<T>& operator=(const T& value);
    virtual OrdinalProperty<T>* clone() const override;
    virtual ~OrdinalProperty();

    operator const T&() const;
    const T& operator*() const;
    const T* operator->() const;

    /**
     * Get the value
     */
    const T& get() const;

    /**
     * Get component 'index' of the value
     */
    component_type get(size_t index) const;

    /**
     * Get component 'i,j' of the value
     */
    component_type get(size_t i, size_t j) const;

    /**
     * Set a new value. The value will be clamped according to the current ConstraintBehaviour
     */
    void set(const T& value);
    /**
     * \brief set all parameters of the ordinal property at the same time with only a
     * single validation. The value will be clamped according to the current ConstraintBehaviour
     */
    void set(const T& value, const T& minVal, const T& maxVal, const T& increment);

    /**
     * Set a new value for component 'index'. The value will be clamped according to the current
     * ConstraintBehaviour
     */
    void set(component_type val, size_t index);

    /**
     * Set a new value for component 'i,j'. The value will be clamped according to the current
     * ConstraintBehaviour
     */
    void set(component_type val, size_t i, size_t j);

    virtual std::string getClassIdentifier() const override;

    const T& getMinValue() const;
    void setMinValue(const T& value);
    ConstraintBehavior getMinConstraintBehaviour() const;

    const T& getMaxValue() const;
    void setMaxValue(const T& value);
    ConstraintBehavior getMaxConstraintBehaviour() const;

    const T& getIncrement() const;
    void setIncrement(const T& value);

    void set(const OrdinalProperty* srcProperty);
    virtual void set(const Property* src) override;

    virtual OrdinalProperty<T>& setCurrentStateAsDefault() override;
    OrdinalProperty<T>& setDefault(const T& value);
    OrdinalProperty<T>& setDefault(const T& value, const T& minVal, const T& maxVal,
                                   const T& increment);
    virtual OrdinalProperty<T>& resetToDefaultState() override;
    virtual bool isDefaultState() const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    static constexpr uvec2 getDim() {
        return uvec2{util::extent<T, 0>::value, util::extent<T, 1>::value};
    }

    virtual Document getDescription() const override;

    /**
     * \brief clamps the given value against the set min/max range
     * @param v value to be clamped
     * @return the clamped value
     */
    T clamp(const T& v) const;

    static bool isLinkingBound(ConstraintBehavior constraint);

    bool isLinkingMinBound() const;
    bool isLinkingMaxBound() const;

    static bool validRange(const T& min, const T& max);

private:
    ValueWrapper<T> value_;
    ValueWrapper<T> minValue_;

    ValueWrapper<T> maxValue_;
    ValueWrapper<T> increment_;
    ValueWrapper<ConstraintBehavior> minConstraint_;
    ValueWrapper<ConstraintBehavior> maxConstraint_;
};

namespace util {

/**
 * A factory function for OrdinalProperties representing Colors
 * When instantiating a Ordinal Property for a color value one would need to write something along
 * there lines
 * ```{.cpp}
 * color("cubeColor", "Cube Color", vec4(0.11f, 0.42f, 0.63f, 1.0f),
 *      {vec4(0.0f), ConstraintBehavior::Immutable},
 *      {vec4(1.0f), ConstraintBehavior::Immutable}, vec4(0.01f),
 *      InvalidationLevel::InvalidOutput, PropertySemantics::Color)
 * ```
 * by using the helper function most of the boilerplate can be removed:
 * ```{.cpp}
 * color{"cubeColor", "Cube Color", util::ordinalColor(0.11f, 0.42f, 0.63f)}
 * ```
 */
IVW_CORE_API OrdinalPropertyState<vec4> ordinalColor(
    float r, float g, float b, float a = 1.0f,
    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);
IVW_CORE_API OrdinalPropertyState<vec4> ordinalColor(
    const vec4& value, InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);
IVW_CORE_API OrdinalPropertyState<vec3> ordinalColor(
    const vec3& value, InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);

IVW_CORE_API OrdinalPropertyState<vec3> ordinalLight(
    const vec3& pos, float min = -100.0, float max = 100.0,
    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);

/**
 * A factory function for configuring a OrdinalProperty representing a generic vector, with a
 * symmetric range around zero, and Ignored boundary constraints. The invalidation level defaults to
 * InvalidOutput, and the property semantics to SpinBox.
 * @param value the default value for the property
 * @param minMax used to construct the range of the property like min = T{-minMax}, max = T{minMax}.
 * The constraint behavior will be Ignore.
 */
template <typename T = double, typename U = T>
OrdinalPropertyState<T> ordinalSymmetricVector(const T& value = {0}, const U& minMax = U{100}) {
    using V = util::value_type_t<T>;
    if constexpr (std::is_floating_point_v<util::value_type_t<T>>) {
        return {value,
                T{-minMax},
                ConstraintBehavior::Ignore,
                T{minMax},
                ConstraintBehavior::Ignore,
                T{static_cast<V>(0.1)},
                InvalidationLevel::InvalidOutput,
                PropertySemantics::SpinBox};
    } else if constexpr (std::is_signed_v<util::value_type_t<T>>) {
        return {value,
                T{-minMax},
                ConstraintBehavior::Ignore,
                T{minMax},
                ConstraintBehavior::Ignore,
                T{static_cast<V>(1)},
                InvalidationLevel::InvalidOutput,
                PropertySemantics::SpinBox};
    } else {
        return {value,
                T{static_cast<V>(0)},
                ConstraintBehavior::Ignore,
                T{minMax},
                ConstraintBehavior::Ignore,
                T{static_cast<V>(1)},
                InvalidationLevel::InvalidOutput,
                PropertySemantics::SpinBox};
    }
}

/**
 * A factory function for configuring a OrdinalProperty representing a count. It will have a
 * Immutable min at zero and an upper Ignored max. The increment will be one. The invalidation level
 * defaults to InvalidOutput, and the property semantics to SpinBox.
 * @param value the default value for the property
 * @param max used to construct the max value. The max constraint behavior will be Ignore.

 */
template <typename T = size_t, typename U = T>
OrdinalPropertyState<T> ordinalCount(const T& value = T{0}, const U& max = U{100}) {
    using V = util::value_type_t<T>;
    return {value,
            T{static_cast<V>(0)},
            ConstraintBehavior::Immutable,
            T{max},
            ConstraintBehavior::Ignore,
            T{static_cast<V>(1)},
            InvalidationLevel::InvalidOutput,
            PropertySemantics::SpinBox};
}


/**
 * A factory function for configuring a OrdinalProperty representing a length. It will have a
 * Immutable min at zero and an upper Ignored max. The invalidation level defaults to InvalidOutput,
 * and the property semantics to SpinBox.
 * @param value the default value for the property
 * @param max used to construct the max value. The max constraint behavior will be Ignore.
 */
template <typename T = double, typename U = T>
OrdinalPropertyState<T> ordinalLength(const T& value = T{0}, const U& max = U{100}) {
    using V = util::value_type_t<T>;
    return {value,
            T{static_cast<V>(0.0)},
            ConstraintBehavior::Immutable,
            T(max),
            ConstraintBehavior::Ignore,
            T{static_cast<V>(0.1)},
            InvalidationLevel::InvalidOutput,
            PropertySemantics::SpinBox};
}

/**
 * A factory function for configuring a OrdinalProperty representing a scale factor. It will have a
 * Immutable min at epsilon and an upper Ignored max. The invalidation level default to
 * InvalidOutput, and the property semantics to Slider.
 * @param value the default value for the property
 * @param max used to construct the max value. The max constraint behavior will be Ignore.
 * @param invalidationLevel property invalidation level, defaults to InvalidOutput
 * @param semantics property semantics, defaults to Slider
 */
template <typename T = double, typename U = T>
OrdinalPropertyState<T> ordinalScale(const T& value = T{0}, const U& max = U{100}) {
    using V = util::value_type_t<T>;
    return {value,
            T{static_cast<V>(100) * std::numeric_limits<V>::epsilon()},
            ConstraintBehavior::Immutable,
            T{max},
            ConstraintBehavior::Ignore,
            T{max / static_cast<U>(256)},
            InvalidationLevel::InvalidOutput,
            PropertySemantics::Default};
}

}  // namespace util

// Scalar properties
using FloatProperty = OrdinalProperty<float>;
using IntProperty = OrdinalProperty<int>;
using IntSizeTProperty = OrdinalProperty<size_t>;
using Int64Property = OrdinalProperty<glm::i64>;
using DoubleProperty = OrdinalProperty<double>;

// Vector properties
using FloatVec2Property = OrdinalProperty<vec2>;
using FloatVec3Property = OrdinalProperty<vec3>;
using FloatVec4Property = OrdinalProperty<vec4>;

using DoubleVec2Property = OrdinalProperty<dvec2>;
using DoubleVec3Property = OrdinalProperty<dvec3>;
using DoubleVec4Property = OrdinalProperty<dvec4>;

using IntVec2Property = OrdinalProperty<ivec2>;
using IntVec3Property = OrdinalProperty<ivec3>;
using IntVec4Property = OrdinalProperty<ivec4>;

using IntSize2Property = OrdinalProperty<size2_t>;
using IntSize3Property = OrdinalProperty<size3_t>;
using IntSize4Property = OrdinalProperty<size4_t>;

// Matrix properties
using FloatMat2Property = OrdinalProperty<mat2>;
using FloatMat3Property = OrdinalProperty<mat3>;
using FloatMat4Property = OrdinalProperty<mat4>;

using DoubleMat2Property = OrdinalProperty<dmat2>;
using DoubleMat3Property = OrdinalProperty<dmat3>;
using DoubleMat4Property = OrdinalProperty<dmat4>;

using DoubleQuaternionProperty = OrdinalProperty<glm::dquat>;
using FloatQuaternionProperty = OrdinalProperty<glm::fquat>;

template <typename T>
struct PropertyTraits<OrdinalProperty<T>> {
    static const std::string& classIdentifier() {
        static const std::string identifier =
            "org.inviwo." + Defaultvalues<T>::getName() + "Property";
        return identifier;
    }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const OrdinalProperty<T>& prop) {
    return os << prop.get();
}

template <typename T>
OrdinalProperty<T>::OrdinalProperty(std::string_view identifier, std::string_view displayName,
                                    Document help, const T& value,
                                    const std::pair<T, ConstraintBehavior>& minValue,
                                    const std::pair<T, ConstraintBehavior>& maxValue,
                                    const T& increment, InvalidationLevel invalidationLevel,
                                    PropertySemantics semantics)
    : Property(identifier, displayName, std::move(help), invalidationLevel, semantics)
    , value_("value", value)
    , minValue_("minvalue", minValue.first)
    , maxValue_("maxvalue", maxValue.first)
    , increment_("increment", increment)
    , minConstraint_{"minConstraint", minValue.second}
    , maxConstraint_{"maxConstraint", maxValue.second} {

    if (!validRange(minValue_, maxValue_) || value_ != clamp(value_)) {
        throw Exception{IVW_CONTEXT,
                        "Invalid range ({} <= {} <= {}) given for \"{}\" ({}Property, {})",
                        minValue_.value,
                        value_.value,
                        maxValue_.value,
                        this->getDisplayName(),
                        Defaultvalues<T>::getName(),
                        this->getPath()};
    }
}

template <typename T>
OrdinalProperty<T>::OrdinalProperty(std::string_view identifier, std::string_view displayName,
                                    const T& value,
                                    const std::pair<T, ConstraintBehavior>& minValue,
                                    const std::pair<T, ConstraintBehavior>& maxValue,
                                    const T& increment, InvalidationLevel invalidationLevel,
                                    PropertySemantics semantics)
    : OrdinalProperty{identifier, displayName,       {},       value, minValue, maxValue,
                      increment,  invalidationLevel, semantics} {}

template <typename T>
OrdinalProperty<T>::OrdinalProperty(std::string_view identifier, std::string_view displayName,
                                    OrdinalPropertyState<T> state)
    : OrdinalProperty{identifier,
                      displayName,
                      state.help,
                      state.value,
                      std::pair{state.min, state.minConstraint},
                      std::pair{state.max, state.maxConstraint},
                      state.increment,
                      state.invalidationLevel,
                      state.semantics} {}

template <typename T>
OrdinalProperty<T>::OrdinalProperty(std::string_view identifier, std::string_view displayName,
                                    const T& value, const T& minValue, const T& maxValue,
                                    const T& increment, InvalidationLevel invalidationLevel,
                                    PropertySemantics semantics)
    : OrdinalProperty{identifier,
                      displayName,
                      {},
                      value,
                      std::pair{minValue, ConstraintBehavior::Editable},
                      std::pair{maxValue, ConstraintBehavior::Editable},
                      increment,
                      invalidationLevel,
                      semantics} {}

template <typename T>
OrdinalProperty<T>::OrdinalProperty(const OrdinalProperty<T>& rhs) = default;

template <typename T>
OrdinalProperty<T>& OrdinalProperty<T>::operator=(const T& value) {
    set(value);
    return *this;
}

template <typename T>
OrdinalProperty<T>* OrdinalProperty<T>::clone() const {
    return new OrdinalProperty<T>(*this);
}

template <typename T>
OrdinalProperty<T>::~OrdinalProperty() = default;

template <typename T>
std::string OrdinalProperty<T>::getClassIdentifier() const {
    return PropertyTraits<OrdinalProperty<T>>::classIdentifier();
}

template <typename T>
OrdinalProperty<T>::operator const T&() const {
    return value_.value;
}

template <typename T>
const T& OrdinalProperty<T>::operator*() const {
    return value_.value;
}

template <typename T>
const T* OrdinalProperty<T>::operator->() const {
    return &value_.value;
}

template <typename T>
const T& OrdinalProperty<T>::get() const {
    return value_.value;
}

template <typename T>
auto OrdinalProperty<T>::get(size_t index) const -> component_type {
    return util::glmcomp(value_.value, index);
}

template <typename T>
auto OrdinalProperty<T>::get(size_t i, size_t j) const -> component_type {
    return util::glmcomp(value_.value, i, j);
}

template <typename T>
void OrdinalProperty<T>::set(const T& value) {
    if (value_.update(clamp(value))) this->propertyModified();
}

template <typename T>
void OrdinalProperty<T>::set(component_type val, size_t index) {
    auto tmp = value_.value;
    util::glmcomp(tmp, index) = val;
    if (value_.update(clamp(tmp))) this->propertyModified();
}

template <typename T>
void OrdinalProperty<T>::set(component_type val, size_t i, size_t j) {
    auto tmp = value_.value;
    util::glmcomp(tmp, i, j) = val;
    if (value_.update(clamp(tmp))) this->propertyModified();
}

template <typename T>
void OrdinalProperty<T>::set(const T& value, const T& minVal, const T& maxVal, const T& increment) {
    if (!validRange(minVal, maxVal)) {
        throw Exception{IVW_CONTEXT, "Invalid range given for \"{}\" ({}Property, {})",
                        this->getDisplayName(), Defaultvalues<T>::getName(), this->getPath()};
    }

    bool modified = false;
    modified |= minValue_.update(minVal);
    modified |= maxValue_.update(maxVal);
    modified |= increment_.update(increment);
    modified |= value_.update(clamp(value));
    if (modified) this->propertyModified();
}

template <typename T>
void OrdinalProperty<T>::set(const OrdinalProperty* srcProperty) {
    bool modified = false;
    if (isLinkingMinBound()) modified |= minValue_.update(srcProperty->minValue_);
    if (isLinkingMaxBound()) modified |= maxValue_.update(srcProperty->maxValue_);
    modified |= increment_.update(srcProperty->increment_);
    modified |= value_.update(clamp(srcProperty->value_));
    if (modified) this->propertyModified();
}

template <typename T>
void OrdinalProperty<T>::set(const Property* srcProperty) {
    if (auto prop = dynamic_cast<const OrdinalProperty<T>*>(srcProperty)) {
        set(prop);
    }
}

template <typename T>
const T& OrdinalProperty<T>::getMinValue() const {
    return minValue_;
}

template <typename T>
void OrdinalProperty<T>::setMinValue(const T& newMinValue) {
    bool modified = false;
    modified |= minValue_.update(newMinValue);
    // Make sure min < max
    modified |= maxValue_.update(glm::max(maxValue_.value, minValue_.value));
    modified |= value_.update(clamp(value_.value));
    if (modified) this->propertyModified();
}

template <typename T>
ConstraintBehavior OrdinalProperty<T>::getMinConstraintBehaviour() const {
    return minConstraint_;
}

template <typename T>
const T& OrdinalProperty<T>::getMaxValue() const {
    return maxValue_;
}

template <typename T>
void OrdinalProperty<T>::setMaxValue(const T& newMaxValue) {
    bool modified = false;
    modified |= maxValue_.update(newMaxValue);
    // Make sure min < max
    modified |= minValue_.update(glm::min(minValue_.value, maxValue_.value));
    modified |= value_.update(clamp(value_.value));
    if (modified) this->propertyModified();
}

template <typename T>
ConstraintBehavior OrdinalProperty<T>::getMaxConstraintBehaviour() const {
    return maxConstraint_;
}

template <typename T>
const T& OrdinalProperty<T>::getIncrement() const {
    return increment_;
}

template <typename T>
void OrdinalProperty<T>::setIncrement(const T& newInc) {
    if (increment_.update(newInc)) this->propertyModified();
}

template <typename T>
OrdinalProperty<T>& OrdinalProperty<T>::resetToDefaultState() {
    bool modified = false;
    modified |= minValue_.reset();
    modified |= maxValue_.reset();
    modified |= increment_.reset();
    modified |= value_.reset();
    if (modified) this->propertyModified();
    return *this;
}

template <typename T>
bool OrdinalProperty<T>::isDefaultState() const {
    return value_.isDefault() && increment_.isDefault() && minValue_.isDefault() &&
           maxValue_.isDefault();
}

template <typename T>
OrdinalProperty<T>& OrdinalProperty<T>::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    value_.setAsDefault();
    minValue_.setAsDefault();
    maxValue_.setAsDefault();
    increment_.setAsDefault();
    return *this;
}

template <typename T>
OrdinalProperty<T>& OrdinalProperty<T>::setDefault(const T& value) {
    value_.defaultValue = value;
    return *this;
}
template <typename T>
OrdinalProperty<T>& OrdinalProperty<T>::setDefault(const T& value, const T& minVal, const T& maxVal,
                                                   const T& increment) {

    value_.defaultValue = value;
    minValue_.defaultValue = minVal;
    maxValue_.defaultValue = maxVal;
    increment_.defaultValue = increment;
    return *this;
}

template <typename T>
void OrdinalProperty<T>::serialize(Serializer& s) const {
    Property::serialize(s);

    minConstraint_.serialize(s, this->serializationMode_);
    maxConstraint_.serialize(s, this->serializationMode_);
    minValue_.serialize(s, this->serializationMode_);
    maxValue_.serialize(s, this->serializationMode_);
    increment_.serialize(s, this->serializationMode_);
    value_.serialize(s, this->serializationMode_);
}

template <typename T>
void OrdinalProperty<T>::deserialize(Deserializer& d) {
    Property::deserialize(d);

    bool modified = false;
    modified |= minConstraint_.deserialize(d, this->serializationMode_);
    modified |= maxConstraint_.deserialize(d, this->serializationMode_);
    modified |= minValue_.deserialize(d, this->serializationMode_);
    modified |= maxValue_.deserialize(d, this->serializationMode_);
    modified |= increment_.deserialize(d, this->serializationMode_);
    modified |= value_.deserialize(d, this->serializationMode_);
    if (modified) this->propertyModified();
}

template <typename T>
T OrdinalProperty<T>::clamp(const T& v) const {
    if (minConstraint_ != ConstraintBehavior::Ignore &&
        maxConstraint_ != ConstraintBehavior::Ignore) {
        return glm::clamp(v, minValue_.value, maxValue_.value);
    } else if (minConstraint_ != ConstraintBehavior::Ignore) {
        return glm::max(v, minValue_.value);
    } else if (maxConstraint_ != ConstraintBehavior::Ignore) {
        return glm::min(v, maxValue_.value);
    } else {
        return v;
    }
}

template <typename T>
bool OrdinalProperty<T>::isLinkingBound(ConstraintBehavior constraint) {
    return constraint == ConstraintBehavior::Editable || constraint == ConstraintBehavior::Ignore;
}

template <typename T>
bool OrdinalProperty<T>::isLinkingMinBound() const {
    return isLinkingBound(minConstraint_);
}

template <typename T>
bool OrdinalProperty<T>::isLinkingMaxBound() const {
    return isLinkingBound(maxConstraint_);
}

template <typename T>
bool OrdinalProperty<T>::validRange(const T& min, const T& max) {
    bool validRange = true;
    for (size_t i = 0; i < util::flat_extent<T>::value; i++) {
        validRange &= util::glmcomp(min, i) <= util::glmcomp(max, i);
    }
    return validRange;
}

template <typename T>
Document OrdinalProperty<T>::getDescription() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc = Property::getDescription();

    utildoc::TableBuilder tb(doc.handle(), P::end());
    tb(H("#"), H("Value"), H(fmt::format("Min ({})", minConstraint_)),
       H(fmt::format("Max ({})", maxConstraint_)), H("Inc"));
    for (size_t i = 0; i < util::flat_extent<T>::value; i++) {
        tb(H(i), util::glmcomp(value_.value, i), util::glmcomp(minValue_.value, i),
           util::glmcomp(maxValue_.value, i), util::glmcomp(increment_.value, i));
    }

    return doc;
}

/// @cond
// Scalar properties
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<float>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<int>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<size_t>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<glm::i64>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<double>;

// Vector properties
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<vec2>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<vec3>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<vec4>;

extern template class IVW_CORE_TMPL_EXP OrdinalProperty<dvec2>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<dvec3>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<dvec4>;

extern template class IVW_CORE_TMPL_EXP OrdinalProperty<ivec2>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<ivec3>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<ivec4>;

extern template class IVW_CORE_TMPL_EXP OrdinalProperty<size2_t>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<size3_t>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<size4_t>;

// Matrix properties
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<mat2>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<mat3>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<mat4>;

extern template class IVW_CORE_TMPL_EXP OrdinalProperty<dmat2>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<dmat3>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<dmat4>;

extern template class IVW_CORE_TMPL_EXP OrdinalProperty<glm::dquat>;
extern template class IVW_CORE_TMPL_EXP OrdinalProperty<glm::fquat>;
/// @endcond

}  // namespace inviwo

template <typename T>
struct fmt::formatter<inviwo::OrdinalProperty<T>> : fmt::formatter<fmt::string_view> {
    template <typename FormatContext>
    auto format(const inviwo::OrdinalProperty<T>& prop, FormatContext& ctx) const {
        fmt::memory_buffer buff;
        fmt::format_to(std::back_inserter(buff), "{}", prop.get());
        return formatter<fmt::string_view>::format(fmt::string_view(buff.data(), buff.size()), ctx);
    }
};
