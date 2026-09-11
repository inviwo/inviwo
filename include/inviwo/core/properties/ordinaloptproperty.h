/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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
#include <inviwo/core/properties/valuewrapper.h>
#include <inviwo/core/properties/constraintbehavior.h>
#include <inviwo/core/util/defaultvalues.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmfmt.h>
#include <inviwo/core/util/glmcomp.h>
#include <inviwo/core/util/glmmatext.h>
#include <inviwo/core/util/docutils.h>

#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include <fmt/base.h>

namespace inviwo {

/**
 * A helper struct to construct optional ordinal properties @see OrdinalOptProperty
 */
template <typename T>
struct OrdinalOptPropertyState {
    std::optional<T> value = std::nullopt;
    T min = Defaultvalues<T>::getMin();
    ConstraintBehavior minConstraint = ConstraintBehavior::Editable;
    T max = Defaultvalues<T>::getMax();
    ConstraintBehavior maxConstraint = ConstraintBehavior::Editable;
    T increment = Defaultvalues<T>::getInc();
    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput;
    PropertySemantics semantics = defaultSemantics();
    Document help = {};
    ReadOnly readOnly = ReadOnly::No;

    auto set(std::optional<T> newValue) -> OrdinalOptPropertyState& {
        value = std::move(newValue);
        return *this;
    }
    auto setMin(T newMin) -> OrdinalOptPropertyState& {
        min = newMin;
        return *this;
    }
    auto setMin(ConstraintBehavior newMinConstraint) -> OrdinalOptPropertyState& {
        minConstraint = newMinConstraint;
        return *this;
    }
    auto setMax(T newMax) -> OrdinalOptPropertyState& {
        max = newMax;
        return *this;
    }
    auto setMax(ConstraintBehavior newMaxConstraint) -> OrdinalOptPropertyState& {
        maxConstraint = newMaxConstraint;
        return *this;
    }
    auto setInc(T newIncrement) -> OrdinalOptPropertyState& {
        increment = newIncrement;
        return *this;
    }
    auto set(InvalidationLevel newInvalidationLevel) -> OrdinalOptPropertyState& {
        invalidationLevel = newInvalidationLevel;
        return *this;
    }
    auto set(PropertySemantics newSemantics) -> OrdinalOptPropertyState& {
        semantics = std::move(newSemantics);
        return *this;
    }
    auto set(Document newHelp) -> OrdinalOptPropertyState& {
        help = std::move(newHelp);
        return *this;
    }
    auto set(ReadOnly newReadOnly) -> OrdinalOptPropertyState& {
        readOnly = newReadOnly;
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
 * @ingroup properties
 * A property representing an optional ordinal value, i.e. `std::optional<T>` where @p T is an
 * ordinal value like int, float, or vec3. In contrast to OrdinalProperty the value can be "empty"
 * (std::nullopt) in addition to holding a value. The min, max, and increment are always present and
 * apply to the contained value when the property holds one.
 */
template <typename T>
class OrdinalOptProperty : public Property {
public:
    using value_type = std::optional<T>;
    using component_type = typename util::value_type<T>::type;

    OrdinalOptProperty(
        std::string_view identifier, std::string_view displayName, Document help,
        const std::optional<T>& value = std::nullopt,
        const std::pair<T, ConstraintBehavior>& minValue = std::pair{Defaultvalues<T>::getMin(),
                                                                     ConstraintBehavior::Editable},
        const std::pair<T, ConstraintBehavior>& maxValue = std::pair{Defaultvalues<T>::getMax(),
                                                                     ConstraintBehavior::Editable},
        const T& increment = Defaultvalues<T>::getInc(),
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
        PropertySemantics semantics = OrdinalOptPropertyState<T>::defaultSemantics(),
        ReadOnly readOnly = ReadOnly::No);

    OrdinalOptProperty(
        std::string_view identifier, std::string_view displayName,
        const std::optional<T>& value = std::nullopt,
        const std::pair<T, ConstraintBehavior>& minValue = std::pair{Defaultvalues<T>::getMin(),
                                                                     ConstraintBehavior::Editable},
        const std::pair<T, ConstraintBehavior>& maxValue = std::pair{Defaultvalues<T>::getMax(),
                                                                     ConstraintBehavior::Editable},
        const T& increment = Defaultvalues<T>::getInc(),
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
        PropertySemantics semantics = OrdinalOptPropertyState<T>::defaultSemantics());

    OrdinalOptProperty(std::string_view identifier, std::string_view displayName,
                       OrdinalOptPropertyState<T> state);

    OrdinalOptProperty(const OrdinalOptProperty<T>& rhs) = default;
    OrdinalOptProperty(OrdinalOptProperty<T>&& rhs) = default;
    OrdinalOptProperty<T>& operator=(const OrdinalOptProperty<T>& that) = delete;
    OrdinalOptProperty<T>& operator=(OrdinalOptProperty<T>&& that) = delete;
    OrdinalOptProperty<T>& operator=(const std::optional<T>& value);
    OrdinalOptProperty<T>& operator=(const T& value);
    virtual OrdinalOptProperty<T>* clone() const override;
    virtual ~OrdinalOptProperty();

    explicit operator bool() const;

    /**
     * Get the value, std::nullopt if the property is empty
     */
    const std::optional<T>& get() const;

    /**
     * Whether the property currently holds a value
     */
    bool hasValue() const;

    /**
     * Get the contained value or, if empty, a sensible fallback (the default value if set,
     * otherwise the type default). Useful for editing/display purposes.
     */
    T value() const;

    /**
     * Get component 'index' of the contained value (or of the fallback when empty)
     */
    auto get(size_t index) const -> component_type;

    /**
     * Get component 'i,j' of the contained value (or of the fallback when empty)
     */
    auto get(size_t i, size_t j) const -> component_type;

    /**
     * Set a new value. The value will be clamped according to the current ConstraintBehaviour.
     * Passing std::nullopt clears the property.
     */
    void set(const std::optional<T>& value);
    void set(const T& value);

    /**
     * @brief set the value and all range parameters at the same time with only a single
     * validation. The value will be clamped according to the current ConstraintBehaviour.
     */
    void set(const std::optional<T>& value, const T& minVal, const T& maxVal, const T& increment);

    /**
     * Clear the property, i.e. set it to std::nullopt
     */
    void clear();

    /**
     * Set a new value for component 'index'. This will engage the property if it was empty, using
     * the current fallback value for the remaining components. The value will be clamped according
     * to the current ConstraintBehaviour.
     */
    void set(component_type val, size_t index);

    /**
     * Set a new value for component 'i,j'. This will engage the property if it was empty, using the
     * current fallback value for the remaining components.
     */
    void set(component_type val, size_t i, size_t j);

    virtual std::string_view getClassIdentifier() const override;

    const T& getMinValue() const;
    void setMinValue(const T& newMinValue);
    ConstraintBehavior getMinConstraintBehaviour() const;

    const T& getMaxValue() const;
    void setMaxValue(const T& newMaxValue);
    ConstraintBehavior getMaxConstraintBehaviour() const;

    const T& getIncrement() const;
    void setIncrement(const T& newInc);

    void set(const OrdinalOptProperty* srcProperty);
    virtual void set(const Property* src) override;

    virtual OrdinalOptProperty<T>& setCurrentStateAsDefault() override;
    OrdinalOptProperty<T>& setDefault(const std::optional<T>& value);
    virtual OrdinalOptProperty<T>& resetToDefaultState() override;
    virtual bool isDefaultState() const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    static constexpr uvec2 getDim() {
        return uvec2{util::extent<T, 0>::value, util::extent<T, 1>::value};
    }

    virtual Document getDescription() const override;

    /**
     * @brief clamps the given value against the set min/max range
     * @param v value to be clamped
     * @return the clamped value
     */
    T clamp(const T& v) const;

    static bool isLinkingBound(ConstraintBehavior constraint);

    bool isLinkingMinBound() const;
    bool isLinkingMaxBound() const;

    static bool validRange(const T& min, const T& max);

private:
    bool updateValue(const std::optional<T>& newValue);

    std::optional<T> value_;
    std::optional<T> defaultValue_;
    ValueWrapper<T> minValue_;
    ValueWrapper<T> maxValue_;
    ValueWrapper<T> increment_;
    ValueWrapper<ConstraintBehavior> minConstraint_;
    ValueWrapper<ConstraintBehavior> maxConstraint_;
};

namespace util {

namespace detail {
template <typename T>
inline constexpr bool isOptional = false;
template <typename T>
inline constexpr bool isOptional<std::optional<T>> = true;
}  // namespace detail

/**
 * A factory function for OrdinalOptProperties representing Colors, @see util::ordinalColor
 * ```{.cpp}
 * color{"cubeColor", "Cube Color", util::ordinalOptColor(0.11f, 0.42f, 0.63f)}
 * ```
 * Without arguments the property will start out empty, that is std::nullopt.
 */
IVW_CORE_API OrdinalOptPropertyState<vec4> ordinalOptColor(
    float r, float g, float b, float a = 1.0f,
    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);
IVW_CORE_API OrdinalOptPropertyState<vec4> ordinalOptColor(
    const vec4& value, InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);
IVW_CORE_API OrdinalOptPropertyState<vec3> ordinalOptColor(
    const vec3& value, InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);
IVW_CORE_API OrdinalOptPropertyState<vec4> ordinalOptColor(
    const std::optional<vec4>& value = std::nullopt,
    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);
IVW_CORE_API OrdinalOptPropertyState<vec3> ordinalOptColor(
    const std::optional<vec3>& value,
    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);

IVW_CORE_API OrdinalOptPropertyState<vec3> ordinalOptLight(
    const std::optional<vec3>& pos = std::nullopt, float min = -100.0, float max = 100.0,
    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);

/**
 * A factory function for configuring a OrdinalOptProperty representing a generic vector, with a
 * symmetric range around zero, and Ignored boundary constraints. The invalidation level defaults to
 * InvalidOutput, and the property semantics to Default.
 * @param value the default value for the property, std::nullopt means the property is empty
 * @param minMax used to construct the range of the property like min = T{-minMax}, max = T{minMax}.
 * The constraint behavior will be Ignore.
 */
template <typename T = double, typename U = T>
OrdinalOptPropertyState<T> ordinalOptSymmetricVector(const std::optional<T>& value = std::nullopt,
                                                     const U& minMax = U{100}) {
    using V = util::value_type_t<T>;
    if constexpr (std::is_floating_point_v<util::value_type_t<T>>) {
        return {value,
                T{-minMax},
                ConstraintBehavior::Ignore,
                T{minMax},
                ConstraintBehavior::Ignore,
                T{static_cast<V>(0.1)},
                InvalidationLevel::InvalidOutput,
                PropertySemantics::Default};
    } else if constexpr (std::is_signed_v<util::value_type_t<T>>) {
        return {value,
                T{-minMax},
                ConstraintBehavior::Ignore,
                T{minMax},
                ConstraintBehavior::Ignore,
                T{static_cast<V>(1)},
                InvalidationLevel::InvalidOutput,
                PropertySemantics::Default};
    } else {
        return {value,
                T{static_cast<V>(0)},
                ConstraintBehavior::Ignore,
                T{minMax},
                ConstraintBehavior::Ignore,
                T{static_cast<V>(1)},
                InvalidationLevel::InvalidOutput,
                PropertySemantics::Default};
    }
}
/// @copydoc util::ordinalOptSymmetricVector
template <typename T, typename U = T>
    requires(!detail::isOptional<T>)
OrdinalOptPropertyState<T> ordinalOptSymmetricVector(const T& value, const U& minMax = U{100}) {
    return ordinalOptSymmetricVector(std::optional<T>{value}, minMax);
}

/**
 * A factory function for configuring a OrdinalOptProperty representing a count. It will have a
 * Immutable min at zero and an upper Ignored max. The increment will be one. The invalidation level
 * defaults to InvalidOutput, and the property semantics to Default.
 * @param value the default value for the property, std::nullopt means the property is empty
 * @param max used to construct the max value. The max constraint behavior will be Ignore.
 */
template <typename T = size_t, typename U = T>
OrdinalOptPropertyState<T> ordinalOptCount(const std::optional<T>& value = std::nullopt,
                                           const U& max = U{100}) {
    using V = util::value_type_t<T>;
    return {value,
            T{static_cast<V>(0)},
            ConstraintBehavior::Immutable,
            T{max},
            ConstraintBehavior::Ignore,
            T{static_cast<V>(1)},
            InvalidationLevel::InvalidOutput,
            PropertySemantics::Default};
}
/// @copydoc util::ordinalOptCount
template <typename T, typename U = T>
    requires(!detail::isOptional<T>)
OrdinalOptPropertyState<T> ordinalOptCount(const T& value, const U& max = U{100}) {
    return ordinalOptCount(std::optional<T>{value}, max);
}

/**
 * A factory function for configuring a OrdinalOptProperty representing a length. It will have a
 * Immutable min at zero and an upper Ignored max. The invalidation level defaults to InvalidOutput,
 * and the property semantics to Default.
 * @param value the default value for the property, std::nullopt means the property is empty
 * @param max used to construct the max value. The max constraint behavior will be Ignore.
 */
template <typename T = double, typename U = T>
OrdinalOptPropertyState<T> ordinalOptLength(const std::optional<T>& value = std::nullopt,
                                            const U& max = U{100}) {
    using V = util::value_type_t<T>;
    return {value,
            T{static_cast<V>(0.0)},
            ConstraintBehavior::Immutable,
            T(max),
            ConstraintBehavior::Ignore,
            T{static_cast<V>(0.1)},
            InvalidationLevel::InvalidOutput,
            PropertySemantics::Default};
}
/// @copydoc util::ordinalOptLength
template <typename T, typename U = T>
    requires(!detail::isOptional<T>)
OrdinalOptPropertyState<T> ordinalOptLength(const T& value, const U& max = U{100}) {
    return ordinalOptLength(std::optional<T>{value}, max);
}

/**
 * A factory function for configuring a OrdinalOptProperty representing a scale factor. It will have
 * a Immutable min at epsilon and an upper Ignored max. The invalidation level default to
 * InvalidOutput, and the property semantics to Default.
 * @param value the default value for the property, std::nullopt means the property is empty
 * @param max used to construct the max value. The max constraint behavior will be Ignore.
 */
template <typename T = double, typename U = T>
OrdinalOptPropertyState<T> ordinalOptScale(const std::optional<T>& value = std::nullopt,
                                           const U& max = U{100}) {
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
/// @copydoc util::ordinalOptScale
template <typename T, typename U = T>
    requires(!detail::isOptional<T>)
OrdinalOptPropertyState<T> ordinalOptScale(const T& value, const U& max = U{100}) {
    return ordinalOptScale(std::optional<T>{value}, max);
}

/**
 * A factory function for configuring a OrdinalOptProperty representing a generic matrix initialized
 * with @p value, with a symmetric range around zero, and Ignored boundary constraints.
 * The invalidation level defaults to InvalidOutput, and the property semantics to Default.
 * @param value the default value for the property, std::nullopt means the property is empty
 * @param minMax used to construct the range of the property like min = util::filled<M>(-minMax),
 * max = util::filled<M>(minMax). The constraint behavior will be Ignore.
 */
template <typename M, typename U = typename M::value_type>
    requires(util::rank_v<M> > 1)
OrdinalOptPropertyState<M> ordinalOptMatrix(const std::optional<M>& value = std::nullopt,
                                            const U& minMax = U{100}) {
    using V = util::value_type_t<M>;
    if constexpr (std::is_floating_point_v<util::value_type_t<V>>) {
        return {value,
                M{V{0}} - minMax,
                ConstraintBehavior::Ignore,
                M{V{0}} + minMax,
                ConstraintBehavior::Ignore,
                M{V{0}} + static_cast<V>(0.1),
                InvalidationLevel::InvalidOutput,
                PropertySemantics::Default};
    } else if constexpr (std::is_signed_v<util::value_type_t<V>>) {
        return {value,
                M{V{0}} - minMax,
                ConstraintBehavior::Ignore,
                M{V{0}} + minMax,
                ConstraintBehavior::Ignore,
                M{V{0}} + static_cast<V>(1),
                InvalidationLevel::InvalidOutput,
                PropertySemantics::Default};
    } else {
        return {value,
                M{V{0}},
                ConstraintBehavior::Ignore,
                M{minMax},
                ConstraintBehavior::Ignore,
                M{V{0}} + static_cast<V>(1),
                InvalidationLevel::InvalidOutput,
                PropertySemantics::Default};
    }
}
/// @copydoc util::ordinalOptMatrix
template <typename M, typename U = typename M::value_type>
    requires(util::rank_v<M> > 1)
OrdinalOptPropertyState<M> ordinalOptMatrix(const M& value, const U& minMax = U{100}) {
    return ordinalOptMatrix<M>(std::optional<M>{value}, minMax);
}

/**
 * A factory function for configuring a OrdinalOptProperty representing a generic matrix filled with
 * the given value @p value, with a symmetric range around zero, and Ignored boundary constraints.
 * The invalidation level defaults to InvalidOutput, and the property semantics to Default.
 * @param value the default value for the property, std::nullopt means the property is empty
 * @param minMax used to construct the range of the property like min = util::filled<M>(-minMax),
 * max = util::filled<M>(minMax). The constraint behavior will be Ignore.
 */
template <typename M, typename T = typename M::value_type, typename U = T>
    requires(util::rank_v<M> > 1)
OrdinalOptPropertyState<M> ordinalOptFilledMatrix(const std::optional<T>& value = std::nullopt,
                                                  const U& minMax = U{100}) {
    using V = util::value_type_t<M>;
    const std::optional<M> matrix =
        value ? std::optional<M>{M{V{0}} + static_cast<V>(*value)} : std::nullopt;
    return ordinalOptMatrix<M>(matrix, minMax);
}
/// @copydoc util::ordinalOptFilledMatrix
template <typename M, typename T, typename U = T>
    requires(util::rank_v<M> > 1 && !detail::isOptional<T>)
OrdinalOptPropertyState<M> ordinalOptFilledMatrix(const T& value, const U& minMax = U{100}) {
    return ordinalOptFilledMatrix<M>(std::optional<T>{value}, minMax);
}

}  // namespace util

template <typename T>
struct PropertyTraits<OrdinalOptProperty<T>> {
    static constexpr std::string_view classIdentifier() {
        static const auto identifier = "org.inviwo." + Defaultvalues<T>::getName() + "OptProperty";
        return identifier;
    }
};

template <typename T>
OrdinalOptProperty<T>::OrdinalOptProperty(std::string_view identifier, std::string_view displayName,
                                          Document help, const std::optional<T>& value,
                                          const std::pair<T, ConstraintBehavior>& minValue,
                                          const std::pair<T, ConstraintBehavior>& maxValue,
                                          const T& increment, InvalidationLevel invalidationLevel,
                                          PropertySemantics semantics, ReadOnly readOnly)
    : Property(identifier, displayName, std::move(help), invalidationLevel, semantics, readOnly)
    , value_{value}
    , defaultValue_{value}
    , minValue_("minvalue", minValue.first)
    , maxValue_("maxvalue", maxValue.first)
    , increment_("increment", increment)
    , minConstraint_{"minConstraint", minValue.second}
    , maxConstraint_{"maxConstraint", maxValue.second} {

    if (!validRange(minValue_, maxValue_) || (value_ && *value_ != clamp(*value_))) {
        throw Exception{SourceContext{},
                        "Invalid range ({} <= value <= {}) given for \"{}\" ({}OptProperty, {})",
                        minValue_.value,
                        maxValue_.value,
                        this->getDisplayName(),
                        Defaultvalues<T>::getName(),
                        this->getPath()};
    }
}

template <typename T>
OrdinalOptProperty<T>::OrdinalOptProperty(std::string_view identifier, std::string_view displayName,
                                          const std::optional<T>& value,
                                          const std::pair<T, ConstraintBehavior>& minValue,
                                          const std::pair<T, ConstraintBehavior>& maxValue,
                                          const T& increment, InvalidationLevel invalidationLevel,
                                          PropertySemantics semantics)
    : OrdinalOptProperty{identifier, displayName, Document{},        value,    minValue,
                         maxValue,   increment,   invalidationLevel, semantics} {}

template <typename T>
OrdinalOptProperty<T>::OrdinalOptProperty(std::string_view identifier, std::string_view displayName,
                                          OrdinalOptPropertyState<T> state)
    : OrdinalOptProperty{identifier,
                         displayName,
                         state.help,
                         state.value,
                         std::pair{state.min, state.minConstraint},
                         std::pair{state.max, state.maxConstraint},
                         state.increment,
                         state.invalidationLevel,
                         state.semantics,
                         state.readOnly} {}

template <typename T>
OrdinalOptProperty<T>& OrdinalOptProperty<T>::operator=(const std::optional<T>& value) {
    set(value);
    return *this;
}

template <typename T>
OrdinalOptProperty<T>& OrdinalOptProperty<T>::operator=(const T& value) {
    set(value);
    return *this;
}

template <typename T>
OrdinalOptProperty<T>* OrdinalOptProperty<T>::clone() const {
    return new OrdinalOptProperty<T>(*this);
}

template <typename T>
OrdinalOptProperty<T>::~OrdinalOptProperty() = default;

template <typename T>
std::string_view OrdinalOptProperty<T>::getClassIdentifier() const {
    return PropertyTraits<OrdinalOptProperty<T>>::classIdentifier();
}

template <typename T>
OrdinalOptProperty<T>::operator bool() const {
    return value_.has_value();
}

template <typename T>
const std::optional<T>& OrdinalOptProperty<T>::get() const {
    return value_;
}

template <typename T>
bool OrdinalOptProperty<T>::hasValue() const {
    return value_.has_value();
}

template <typename T>
T OrdinalOptProperty<T>::value() const {
    if (value_) return *value_;
    if (defaultValue_) return clamp(*defaultValue_);
    return clamp(Defaultvalues<T>::getVal());
}

template <typename T>
auto OrdinalOptProperty<T>::get(size_t index) const -> component_type {
    const T v = value();
    return util::glmcomp(v, index);
}

template <typename T>
auto OrdinalOptProperty<T>::get(size_t i, size_t j) const -> component_type {
    const T v = value();
    return util::glmcomp(v, i, j);
}

template <typename T>
bool OrdinalOptProperty<T>::updateValue(const std::optional<T>& newValue) {
    if (value_ != newValue) {
        value_ = newValue;
        return true;
    }
    return false;
}

template <typename T>
void OrdinalOptProperty<T>::set(const std::optional<T>& value) {
    const std::optional<T> newValue = value ? std::optional<T>{clamp(*value)} : std::nullopt;
    if (updateValue(newValue)) this->propertyModified();
}

template <typename T>
void OrdinalOptProperty<T>::set(const T& value) {
    if (updateValue(std::optional<T>{clamp(value)})) this->propertyModified();
}

template <typename T>
void OrdinalOptProperty<T>::clear() {
    if (updateValue(std::nullopt)) this->propertyModified();
}

template <typename T>
void OrdinalOptProperty<T>::set(component_type val, size_t index) {
    auto tmp = value();
    util::glmcomp(tmp, index) = val;
    set(tmp);
}

template <typename T>
void OrdinalOptProperty<T>::set(component_type val, size_t i, size_t j) {
    auto tmp = value();
    util::glmcomp(tmp, i, j) = val;
    set(tmp);
}

template <typename T>
void OrdinalOptProperty<T>::set(const std::optional<T>& value, const T& minVal, const T& maxVal,
                                const T& increment) {
    if (!validRange(minVal, maxVal)) {
        throw Exception{SourceContext{}, "Invalid range given for \"{}\" ({}OptProperty, {})",
                        this->getDisplayName(), Defaultvalues<T>::getName(), this->getPath()};
    }

    bool modified = false;
    modified |= minValue_.update(minVal);
    modified |= maxValue_.update(maxVal);
    modified |= increment_.update(increment);
    const std::optional<T> newValue = value ? std::optional<T>{clamp(*value)} : std::nullopt;
    modified |= updateValue(newValue);
    if (modified) this->propertyModified();
}

template <typename T>
void OrdinalOptProperty<T>::set(const OrdinalOptProperty* srcProperty) {
    bool modified = false;
    if (isLinkingMinBound()) modified |= minValue_.update(srcProperty->minValue_);
    if (isLinkingMaxBound()) modified |= maxValue_.update(srcProperty->maxValue_);
    modified |= increment_.update(srcProperty->increment_);
    const auto& src = srcProperty->value_;
    const std::optional<T> newValue = src ? std::optional<T>{clamp(*src)} : std::nullopt;
    modified |= updateValue(newValue);
    if (modified) this->propertyModified();
}

template <typename T>
void OrdinalOptProperty<T>::set(const Property* srcProperty) {
    if (auto prop = dynamic_cast<const OrdinalOptProperty<T>*>(srcProperty)) {
        set(prop);
    }
}

template <typename T>
const T& OrdinalOptProperty<T>::getMinValue() const {
    return minValue_;
}

template <typename T>
void OrdinalOptProperty<T>::setMinValue(const T& newMinValue) {
    bool modified = false;
    modified |= minValue_.update(newMinValue);
    modified |= maxValue_.update(glm::max(maxValue_.value, minValue_.value));
    if (value_) modified |= updateValue(std::optional<T>{clamp(*value_)});
    if (modified) this->propertyModified();
}

template <typename T>
ConstraintBehavior OrdinalOptProperty<T>::getMinConstraintBehaviour() const {
    return minConstraint_;
}

template <typename T>
const T& OrdinalOptProperty<T>::getMaxValue() const {
    return maxValue_;
}

template <typename T>
void OrdinalOptProperty<T>::setMaxValue(const T& newMaxValue) {
    bool modified = false;
    modified |= maxValue_.update(newMaxValue);
    modified |= minValue_.update(glm::min(minValue_.value, maxValue_.value));
    if (value_) modified |= updateValue(std::optional<T>{clamp(*value_)});
    if (modified) this->propertyModified();
}

template <typename T>
ConstraintBehavior OrdinalOptProperty<T>::getMaxConstraintBehaviour() const {
    return maxConstraint_;
}

template <typename T>
const T& OrdinalOptProperty<T>::getIncrement() const {
    return increment_;
}

template <typename T>
void OrdinalOptProperty<T>::setIncrement(const T& newInc) {
    if (increment_.update(newInc)) this->propertyModified();
}

template <typename T>
OrdinalOptProperty<T>& OrdinalOptProperty<T>::resetToDefaultState() {
    bool modified = false;
    modified |= minValue_.reset();
    modified |= maxValue_.reset();
    modified |= increment_.reset();
    modified |= updateValue(defaultValue_);
    if (modified) this->propertyModified();
    return *this;
}

template <typename T>
bool OrdinalOptProperty<T>::isDefaultState() const {
    return value_ == defaultValue_ && increment_.isDefault() && minValue_.isDefault() &&
           maxValue_.isDefault();
}

template <typename T>
OrdinalOptProperty<T>& OrdinalOptProperty<T>::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    defaultValue_ = value_;
    minValue_.setAsDefault();
    maxValue_.setAsDefault();
    increment_.setAsDefault();
    return *this;
}

template <typename T>
OrdinalOptProperty<T>& OrdinalOptProperty<T>::setDefault(const std::optional<T>& value) {
    defaultValue_ = value;
    return *this;
}

template <typename T>
void OrdinalOptProperty<T>::serialize(Serializer& s) const {
    Property::serialize(s);

    minConstraint_.serialize(s, this->serializationMode_);
    maxConstraint_.serialize(s, this->serializationMode_);
    minValue_.serialize(s, this->serializationMode_);
    maxValue_.serialize(s, this->serializationMode_);
    increment_.serialize(s, this->serializationMode_);

    // std::optional has no generic Serializer support, serialize a presence flag plus the value
    if (this->serializationMode_ == PropertySerializationMode::None) return;
    if (this->serializationMode_ == PropertySerializationMode::All || value_ != defaultValue_) {
        s.serialize("hasValue", value_.has_value());
        if (value_) s.serialize("value", *value_);
    }
}

template <typename T>
void OrdinalOptProperty<T>::deserialize(Deserializer& d) {
    Property::deserialize(d);

    bool modified = false;
    modified |= minConstraint_.deserialize(d, this->serializationMode_);
    modified |= maxConstraint_.deserialize(d, this->serializationMode_);
    modified |= minValue_.deserialize(d, this->serializationMode_);
    modified |= maxValue_.deserialize(d, this->serializationMode_);
    modified |= increment_.deserialize(d, this->serializationMode_);

    if (this->serializationMode_ != PropertySerializationMode::None) {
        if (d.hasElement("hasValue")) {
            bool has = value_.has_value();
            d.deserialize("hasValue", has);
            if (has) {
                T tmp = value();
                d.deserialize("value", tmp);
                modified |= updateValue(std::optional<T>{tmp});
            } else {
                modified |= updateValue(std::nullopt);
            }
        } else {
            modified |= updateValue(defaultValue_);
        }
    }
    if (modified) this->propertyModified();
}

template <typename T>
T OrdinalOptProperty<T>::clamp(const T& v) const {
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
bool OrdinalOptProperty<T>::isLinkingBound(ConstraintBehavior constraint) {
    return constraint == ConstraintBehavior::Editable || constraint == ConstraintBehavior::Ignore;
}

template <typename T>
bool OrdinalOptProperty<T>::isLinkingMinBound() const {
    return isLinkingBound(minConstraint_);
}

template <typename T>
bool OrdinalOptProperty<T>::isLinkingMaxBound() const {
    return isLinkingBound(maxConstraint_);
}

template <typename T>
bool OrdinalOptProperty<T>::validRange(const T& min, const T& max) {
    bool validRange = true;
    for (size_t i = 0; i < util::flat_extent<T>::value; i++) {
        validRange &= util::glmcomp(min, i) <= util::glmcomp(max, i);
    }
    return validRange;
}

template <typename T>
Document OrdinalOptProperty<T>::getDescription() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc = Property::getDescription();

    utildoc::TableBuilder tb(doc.handle(), P::end());
    tb(H("Has value"), value_.has_value() ? "yes" : "no");

    const T val = value();
    utildoc::TableBuilder tbv(doc.handle(), P::end());
    tbv(H("#"), H("Value"), H(fmt::format("Min ({})", minConstraint_)),
        H(fmt::format("Max ({})", maxConstraint_)), H("Inc"));
    for (size_t i = 0; i < util::flat_extent<T>::value; i++) {
        tbv(H(i), util::glmcomp(val, i), util::glmcomp(minValue_.value, i),
            util::glmcomp(maxValue_.value, i), util::glmcomp(increment_.value, i));
    }

    return doc;
}

/// @cond
// Scalar properties
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<float>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<int>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<size_t>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<glm::i64>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<double>;

// Vector properties
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<vec2>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<vec3>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<vec4>;

extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<dvec2>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<dvec3>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<dvec4>;

extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<ivec2>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<ivec3>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<ivec4>;

extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<size2_t>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<size3_t>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<size4_t>;

// Matrix properties
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<mat2>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<mat3>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<mat4>;

extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<dmat2>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<dmat3>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<dmat4>;

extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<glm::dquat>;
extern template class IVW_CORE_TMPL_EXP OrdinalOptProperty<glm::fquat>;
/// @endcond

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <typename T>
struct fmt::formatter<inviwo::OrdinalOptProperty<T>> : fmt::formatter<fmt::string_view> {
    template <typename FormatContext>
    auto format(const inviwo::OrdinalOptProperty<T>& prop, FormatContext& ctx) const {
        fmt::memory_buffer buff;
        if (auto val = prop.get()) {
            fmt::format_to(std::back_inserter(buff), "{}", *val);
        } else {
            fmt::format_to(std::back_inserter(buff), "{}", "unset");
        }
        return formatter<fmt::string_view>::format(fmt::string_view(buff.data(), buff.size()), ctx);
    }
};
#endif
