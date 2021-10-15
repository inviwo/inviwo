/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/assertion.h>

#include <string>
#include <functional>

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace inviwo {

/**
 * A helper struct to construct ordinal properties @see OrdinalRefProperty
 */
template <typename T>
struct OrdinalRefPropertyState {
    std::pair<T, ConstraintBehavior> minValue =
        std::pair{Defaultvalues<T>::getMin(), ConstraintBehavior::Editable};
    std::pair<T, ConstraintBehavior> maxValue =
        std::pair{Defaultvalues<T>::getMax(), ConstraintBehavior::Editable};
    T increment = Defaultvalues<T>::getInc();
    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput;
    PropertySemantics semantics = defaultSemantics();

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
 * A property representing a reference to an Ordinal value, for example a int, float, or vec3.
 * The property does not hold or own the value it self. It only contain a set and get callback
 * function. It does handle min/max/increment for the value.
 */
template <typename T>
class OrdinalRefProperty : public Property {
public:
    using value_type = T;
    using component_type = typename util::value_type<T>::type;

    OrdinalRefProperty(
        const std::string& identifier, const std::string& displayName, std::function<T()> get,
        std::function<void(const T&)> set,
        const std::pair<T, ConstraintBehavior>& minValue = std::pair{Defaultvalues<T>::getMin(),
                                                                     ConstraintBehavior::Editable},
        const std::pair<T, ConstraintBehavior>& maxValue = std::pair{Defaultvalues<T>::getMax(),
                                                                     ConstraintBehavior::Editable},
        const T& increment = Defaultvalues<T>::getInc(),
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
        PropertySemantics semantics = OrdinalRefPropertyState<T>::defaultSemantics());

    OrdinalRefProperty(const std::string& identifier, const std::string& displayName,
                       std::function<const T&()> get, std::function<void(const T&)> set,
                       OrdinalRefPropertyState<T> state);

    OrdinalRefProperty(const OrdinalRefProperty<T>& rhs);
    OrdinalRefProperty<T>& operator=(const T& value);
    virtual OrdinalRefProperty<T>* clone() const override;
    virtual ~OrdinalRefProperty();

    operator T() const;
    T operator*() const;

    /**
     * Get the value
     */
    T get() const;

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

    void set(const OrdinalRefProperty* srcProperty);
    virtual void set(const Property* src) override;

    virtual OrdinalRefProperty<T>& setCurrentStateAsDefault() override;
    virtual OrdinalRefProperty<T>& resetToDefaultState() override;
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

    void setGetAndSet(std::function<T()> get, std::function<void(const T&)> set) {
        IVW_ASSERT(get_, "The getter has to be valid");
        IVW_ASSERT(set_, "The setter has to be valid");
        IVW_ASSERT(get, "The getter has to be valid");
        IVW_ASSERT(set, "The setter has to be valid");

        auto val = get_();
        get_ = std::move(get);
        set_ = std::move(set);
        if (val != get_()) {
            propertyModified();
        }
    }

private:
    std::function<T()> get_;
    std::function<void(const T&)> set_;
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
 * color{"cubeColor", "Cube Color", util::ordinalColor()}
 * ```
 */
IVW_CORE_API OrdinalRefPropertyState<vec4> ordinalRefColor(
    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput);
}  // namespace util

using FloatRefProperty = OrdinalRefProperty<float>;
using FloatVec2RefProperty = OrdinalRefProperty<vec2>;
using FloatVec3RefProperty = OrdinalRefProperty<vec3>;
using FloatVec4RefProperty = OrdinalRefProperty<vec4>;

using DoubleRefProperty = OrdinalRefProperty<double>;
using DoubleVec2RefProperty = OrdinalRefProperty<dvec2>;
using DoubleVec3RefProperty = OrdinalRefProperty<dvec3>;
using DoubleVec4RefProperty = OrdinalRefProperty<dvec4>;

using IntRefProperty = OrdinalRefProperty<int>;
using IntVec2RefProperty = OrdinalRefProperty<ivec2>;
using IntVec3RefProperty = OrdinalRefProperty<ivec3>;
using IntVec4RefProperty = OrdinalRefProperty<ivec4>;

using IntSizeTRefProperty = OrdinalRefProperty<size_t>;
using IntSize2RefProperty = OrdinalRefProperty<size2_t>;
using IntSize3RefProperty = OrdinalRefProperty<size3_t>;
using IntSize4RefProperty = OrdinalRefProperty<size4_t>;

template <typename T>
struct PropertyTraits<OrdinalRefProperty<T>> {
    static const std::string& classIdentifier() {
        static const std::string identifier =
            "org.inviwo." + Defaultvalues<T>::getName() + "RefProperty";
        return identifier;
    }
};

template <typename CTy, typename CTr, typename T>
std::basic_ostream<CTy, CTr>& operator<<(std::basic_ostream<CTy, CTr>& os,
                                         const OrdinalRefProperty<T>& prop) {
    return os << prop.get();
}

template <typename T>
OrdinalRefProperty<T>::OrdinalRefProperty(const std::string& identifier,
                                          const std::string& displayName, std::function<T()> get,
                                          std::function<void(const T&)> set,
                                          const std::pair<T, ConstraintBehavior>& minValue,
                                          const std::pair<T, ConstraintBehavior>& maxValue,
                                          const T& increment, InvalidationLevel invalidationLevel,
                                          PropertySemantics semantics)
    : Property(identifier, displayName, invalidationLevel, semantics)
    , get_{std::move(get)}
    , set_{std::move(set)}
    , minValue_("minvalue", minValue.first)
    , maxValue_("maxvalue", maxValue.first)
    , increment_("increment", increment)
    , minConstraint_{"minConstraint", minValue.second}
    , maxConstraint_{"maxConstraint", maxValue.second} {

    IVW_ASSERT(get_, "The getter has to be valid");
    IVW_ASSERT(set_, "The setter has to be valid");

    if (!validRange(minValue_, maxValue_) || get_() != clamp(get_())) {
        throw Exception{
            fmt::format("Invalid range ({} <= {} <= {}) given for \"{}\" ({}Property, {})",
                        minValue_.value, get_(), maxValue_.value, this->getDisplayName(),
                        Defaultvalues<T>::getName(), this->getPath()),
            IVW_CONTEXT};
    }
}

template <typename T>
OrdinalRefProperty<T>::OrdinalRefProperty(const std::string& identifier,
                                          const std::string& displayName,
                                          std::function<const T&()> get,
                                          std::function<void(const T&)> set,
                                          OrdinalRefPropertyState<T> state)
    : OrdinalRefProperty{identifier,
                         displayName,
                         get,
                         set,
                         state.minValue,
                         state.maxValue,
                         state.increment,
                         state.invalidationLevel,
                         state.semantics} {}

template <typename T>
OrdinalRefProperty<T>::OrdinalRefProperty(const OrdinalRefProperty<T>& rhs) = default;

template <typename T>
OrdinalRefProperty<T>& OrdinalRefProperty<T>::operator=(const T& value) {
    IVW_ASSERT(set_, "The setter has to be valid");
    set(value);
    return *this;
}

template <typename T>
OrdinalRefProperty<T>* OrdinalRefProperty<T>::clone() const {
    return new OrdinalRefProperty<T>(*this);
}

template <typename T>
OrdinalRefProperty<T>::~OrdinalRefProperty() = default;

template <typename T>
std::string OrdinalRefProperty<T>::getClassIdentifier() const {
    return PropertyTraits<OrdinalRefProperty<T>>::classIdentifier();
}

template <typename T>
OrdinalRefProperty<T>::operator T() const {
    IVW_ASSERT(get_, "The getter has to be valid");
    return get_();
}

template <typename T>
auto OrdinalRefProperty<T>::operator*() const -> T {
    IVW_ASSERT(get_, "The getter has to be valid");
    return get_();
}

template <typename T>
auto OrdinalRefProperty<T>::get() const -> T {
    IVW_ASSERT(get_, "The getter has to be valid");
    return get_();
}

template <typename T>
auto OrdinalRefProperty<T>::get(size_t index) const -> component_type {
    IVW_ASSERT(get_, "The getter has to be valid");
    const auto val = get_();
    return util::glmcomp(val, index);
}

template <typename T>
auto OrdinalRefProperty<T>::get(size_t i, size_t j) const -> component_type {
    IVW_ASSERT(get_, "The getter has to be valid");
    const auto val = get_();
    return util::glmcomp(val, i, j);
}

template <typename T>
void OrdinalRefProperty<T>::set(const T& value) {
    IVW_ASSERT(get_, "The getter has to be valid");
    IVW_ASSERT(set_, "The setter has to be valid");
    const auto val = clamp(value);
    if (val != get_()) {
        set_(val);
        this->propertyModified();
    }
}

template <typename T>
void OrdinalRefProperty<T>::set(component_type val, size_t index) {
    IVW_ASSERT(get_, "The getter has to be valid");
    IVW_ASSERT(set_, "The setter has to be valid");
    auto tmp = get_();
    util::glmcomp(tmp, index) = val;
    set(tmp);
}

template <typename T>
void OrdinalRefProperty<T>::set(component_type val, size_t i, size_t j) {
    IVW_ASSERT(get_, "The getter has to be valid");
    IVW_ASSERT(set_, "The setter has to be valid");
    auto tmp = get_();
    util::glmcomp(tmp, i, j) = val;
    set(tmp);
}

template <typename T>
void OrdinalRefProperty<T>::set(const T& value, const T& minVal, const T& maxVal,
                                const T& increment) {
    IVW_ASSERT(get_, "The getter has to be valid");
    IVW_ASSERT(set_, "The setter has to be valid");
    if (!validRange(minVal, maxVal)) {
        throw Exception{
            fmt::format("Invalid range given for \"{}\" ({}Property, {})", this->getDisplayName(),
                        Defaultvalues<T>::getName(), this->getPath()),
            IVW_CONTEXT};
    }

    bool modified = false;
    modified |= minValue_.update(minVal);
    modified |= maxValue_.update(maxVal);
    modified |= increment_.update(increment);

    auto val = clamp(value);  // make sure we clamp after we update min/max
    if (val != get_()) {
        set_(val);
        modified = true;
    }

    if (modified) this->propertyModified();
}

template <typename T>
void OrdinalRefProperty<T>::set(const OrdinalRefProperty* srcProperty) {
    IVW_ASSERT(get_, "The getter has to be valid");
    IVW_ASSERT(set_, "The setter has to be valid");
    bool modified = false;
    if (isLinkingMinBound()) modified |= minValue_.update(srcProperty->minValue_);
    if (isLinkingMaxBound()) modified |= maxValue_.update(srcProperty->maxValue_);
    modified |= increment_.update(srcProperty->increment_);

    auto val = clamp(srcProperty->get_());  // make sure we clamp after we update min/max
    if (val != get_()) {
        set_(val);
        modified = true;
    }

    if (modified) this->propertyModified();
}

template <typename T>
void OrdinalRefProperty<T>::set(const Property* srcProperty) {
    if (auto prop = dynamic_cast<const OrdinalRefProperty<T>*>(srcProperty)) {
        set(prop);
    }
}

template <typename T>
auto OrdinalRefProperty<T>::getMinValue() const -> const T& {
    return minValue_;
}

template <typename T>
void OrdinalRefProperty<T>::setMinValue(const T& newMinValue) {
    IVW_ASSERT(get_, "The getter has to be valid");
    IVW_ASSERT(set_, "The setter has to be valid");
    bool modified = false;
    modified |= minValue_.update(newMinValue);
    // Make sure min < max
    modified |= maxValue_.update(glm::max(maxValue_.value, minValue_.value));

    auto val = clamp(get_());  // make sure we clamp after we update min/max
    if (val != get_()) {
        set_(val);
        modified = true;
    }
    if (modified) this->propertyModified();
}

template <typename T>
ConstraintBehavior OrdinalRefProperty<T>::getMinConstraintBehaviour() const {
    return minConstraint_;
}

template <typename T>
auto OrdinalRefProperty<T>::getMaxValue() const -> const T& {
    return maxValue_;
}

template <typename T>
void OrdinalRefProperty<T>::setMaxValue(const T& newMaxValue) {
    IVW_ASSERT(get_, "The getter has to be valid");
    IVW_ASSERT(set_, "The setter has to be valid");
    bool modified = false;
    modified |= maxValue_.update(newMaxValue);
    // Make sure min < max
    modified |= minValue_.update(glm::min(minValue_.value, maxValue_.value));

    auto val = clamp(get_());  // make sure we clamp after we update min/max
    if (val != get_()) {
        set_(val);
        modified = true;
    }

    if (modified) this->propertyModified();
}

template <typename T>
ConstraintBehavior OrdinalRefProperty<T>::getMaxConstraintBehaviour() const {
    return maxConstraint_;
}

template <typename T>
auto OrdinalRefProperty<T>::getIncrement() const -> const T& {
    return increment_;
}

template <typename T>
void OrdinalRefProperty<T>::setIncrement(const T& newInc) {
    if (increment_.update(newInc)) this->propertyModified();
}

template <typename T>
OrdinalRefProperty<T>& OrdinalRefProperty<T>::resetToDefaultState() {
    bool modified = false;
    modified |= minValue_.reset();
    modified |= maxValue_.reset();
    modified |= increment_.reset();
    if (modified) this->propertyModified();
    return *this;
}

template <typename T>
bool OrdinalRefProperty<T>::isDefaultState() const {
    return increment_.isDefault() && minValue_.isDefault() && maxValue_.isDefault();
}

template <typename T>
OrdinalRefProperty<T>& OrdinalRefProperty<T>::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    minValue_.setAsDefault();
    maxValue_.setAsDefault();
    increment_.setAsDefault();
    return *this;
}

template <typename T>
void OrdinalRefProperty<T>::serialize(Serializer& s) const {
    Property::serialize(s);

    minConstraint_.serialize(s, this->serializationMode_);
    maxConstraint_.serialize(s, this->serializationMode_);
    minValue_.serialize(s, this->serializationMode_);
    maxValue_.serialize(s, this->serializationMode_);
    increment_.serialize(s, this->serializationMode_);
}

template <typename T>
void OrdinalRefProperty<T>::deserialize(Deserializer& d) {
    Property::deserialize(d);

    bool modified = false;
    modified |= minConstraint_.deserialize(d, this->serializationMode_);
    modified |= maxConstraint_.deserialize(d, this->serializationMode_);
    modified |= minValue_.deserialize(d, this->serializationMode_);
    modified |= maxValue_.deserialize(d, this->serializationMode_);
    modified |= increment_.deserialize(d, this->serializationMode_);
    if (modified) this->propertyModified();
}

template <typename T>
auto OrdinalRefProperty<T>::clamp(const T& v) const -> T {
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
bool OrdinalRefProperty<T>::isLinkingBound(ConstraintBehavior constraint) {
    return constraint == ConstraintBehavior::Editable || constraint == ConstraintBehavior::Ignore;
}

template <typename T>
bool OrdinalRefProperty<T>::isLinkingMinBound() const {
    return isLinkingBound(minConstraint_);
}

template <typename T>
bool OrdinalRefProperty<T>::isLinkingMaxBound() const {
    return isLinkingBound(maxConstraint_);
}

template <typename T>
bool OrdinalRefProperty<T>::validRange(const T& min, const T& max) {
    bool validRange = true;
    for (size_t i = 0; i < util::flat_extent<T>::value; i++) {
        validRange &= util::glmcomp(min, i) <= util::glmcomp(max, i);
    }
    return validRange;
}

template <typename T>
Document OrdinalRefProperty<T>::getDescription() const {
    IVW_ASSERT(get_, "The getter has to be valid");
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc = Property::getDescription();
    auto b = doc.get({P("html"), P("body")});

    utildoc::TableBuilder tb(b, P::end());
    tb(H("#"), H("Value"), H(fmt::format("Min ({})", minConstraint_)),
       H(fmt::format("Max ({})", maxConstraint_)), H("Inc"));
    const auto val = get_();
    for (size_t i = 0; i < util::flat_extent<T>::value; i++) {
        tb(H(i), util::glmcomp(val, i), util::glmcomp(minValue_.value, i),
           util::glmcomp(maxValue_.value, i), util::glmcomp(increment_.value, i));
    }

    return doc;
}

/// @cond
// Scalar properties
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<float>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<int>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<size_t>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<glm::i64>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<double>;

// Vector properties
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<vec2>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<vec3>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<vec4>;

extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<dvec2>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<dvec3>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<dvec4>;

extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<ivec2>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<ivec3>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<ivec4>;

extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<size2_t>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<size3_t>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<size4_t>;

// Matrix properties
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<mat2>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<mat3>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<mat4>;

extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<dmat2>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<dmat3>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<dmat4>;

extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<glm::dquat>;
extern template class IVW_CORE_TMPL_EXP OrdinalRefProperty<glm::fquat>;
/// @endcond

}  // namespace inviwo
