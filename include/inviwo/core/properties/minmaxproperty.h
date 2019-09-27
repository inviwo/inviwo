/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_MINMAXPROPERTY_H
#define IVW_MINMAXPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/templateproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <algorithm>
#include <limits>

namespace inviwo {

/**
 * \ingroup properties
 * A property representing a range.
 */
template <typename T>
class MinMaxProperty : public TemplateProperty<glm::tvec2<T, glm::defaultp>> {
public:
    typedef glm::tvec2<T, glm::defaultp> range_type;
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    MinMaxProperty(std::string identifier, std::string displayName,
                   T valueMin = Defaultvalues<T>::getMin(), T valueMax = Defaultvalues<T>::getMax(),
                   T rangeMin = Defaultvalues<T>::getMin(), T rangeMax = Defaultvalues<T>::getMax(),
                   T increment = Defaultvalues<T>::getInc(), T minSeperation = 0,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    MinMaxProperty(const MinMaxProperty& rhs) = default;
    MinMaxProperty& operator=(const range_type& value);

    virtual MinMaxProperty<T>* clone() const override;
    virtual ~MinMaxProperty() = default;

    T getRangeMin() const;
    T getRangeMax() const;
    T getIncrement() const;
    T getMinSeparation() const;

    T getStart() const;
    T getEnd() const;

    range_type getRange() const;

    virtual void set(const range_type& value) override;
    virtual void set(const Property* srcProperty) override;

    MinMaxProperty<T>& setStart(const T& value);
    MinMaxProperty<T>& setEnd(const T& value);

    MinMaxProperty<T>& setRangeMin(const T& value);
    MinMaxProperty<T>& setRangeMax(const T& value);
    MinMaxProperty<T>& setIncrement(const T& value);
    MinMaxProperty<T>& setMinSeparation(const T& value);

    MinMaxProperty<T>& setRange(const range_type& value);

    /**
     * \brief set all parameters of the range property at the same time with only a
     * single validation.
     * This circumvents problems when updating both range and value, e.g. changing them
     * from 0 - 100 to 1000 - 2000.
     */
    void set(const range_type& value, const range_type& range, const T& increment, const T& minSep);
    void set(const T& start, const T& end, const T& rangeMin, const T& rangeMax, const T& increment,
             const T& minSep);

    // set a new range, and maintains the same relative values as before.
    MinMaxProperty<T>& setRangeNormalized(const range_type& newRange);

    const BaseCallBack* onRangeChange(std::function<void()> callback);
    void removeOnRangeChange(const BaseCallBack* callback);

    virtual MinMaxProperty<T>& setCurrentStateAsDefault() override;
    virtual MinMaxProperty<T>& resetToDefaultState() override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& s) override;

    static uvec2 getDim() { return Defaultvalues<T>::getDim(); }

    virtual Document getDescription() const override;

protected:
    /**
     * \brief clamp the given value against the set min/max range
     *
     * @param v   value to be clamped
     * @return returns a valid value within the min max range
     */
    range_type clamp(const range_type& v) const;
    T limitSeparation(T sep) const;

private:
    using TemplateProperty<range_type>::value_;

    ValueWrapper<range_type> range_;
    ValueWrapper<T> increment_;
    ValueWrapper<T> minSeparation_;

    CallBackList onRangeChangeCallback_;
};

using FloatMinMaxProperty = MinMaxProperty<float>;
using DoubleMinMaxProperty = MinMaxProperty<double>;
using IntSizeTMinMaxProperty = MinMaxProperty<size_t>;
using Int64MinMaxProperty = MinMaxProperty<glm::i64>;
using IntMinMaxProperty = MinMaxProperty<int>;

template <typename T>
struct PropertyTraits<MinMaxProperty<T>> {
    static std::string classIdentifier() {
        return "org.inviwo." + Defaultvalues<T>::getName() + "MinMaxProperty";
    }
};

template <typename T>
MinMaxProperty<T>::MinMaxProperty(std::string identifier, std::string displayName, T valueMin,
                                  T valueMax, T rangeMin, T rangeMax, T increment, T minSeparation,
                                  InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : TemplateProperty<range_type>(identifier, displayName, range_type(valueMin, valueMax),
                                   invalidationLevel, semantics)
    , range_("range", range_type(rangeMin, rangeMax))
    , increment_("increment", increment)
    , minSeparation_("minSeparation", minSeparation) {
    // invariant: range_.x <= value_.x <= value_.y + minseperation <= range_.y
    // Assume minimum range, i.e. range_.x, is correct.
    value_.value.x = std::max(value_.value.x, range_.value.x);
    value_.value.y = std::max(value_.value.y, value_.value.x + minSeparation_.value);
    range_.value.y = std::max(range_.value.y, value_.value.y);
}

template <typename T>
MinMaxProperty<T>& MinMaxProperty<T>::operator=(const range_type& value) {
    TemplateProperty<range_type>::operator=(value);
    return *this;
}

template <typename T>
MinMaxProperty<T>* MinMaxProperty<T>::clone() const {
    return new MinMaxProperty<T>(*this);
}

template <typename T>
std::string MinMaxProperty<T>::getClassIdentifier() const {
    return PropertyTraits<MinMaxProperty<T>>::classIdentifier();
}

template <typename T>
T MinMaxProperty<T>::getStart() const {
    return value_.value.x;
}

template <typename T>
T MinMaxProperty<T>::getEnd() const {
    return value_.value.y;
}

template <typename T>
T MinMaxProperty<T>::getRangeMin() const {
    return range_.value.x;
}

template <typename T>
T MinMaxProperty<T>::getRangeMax() const {
    return range_.value.y;
}

template <typename T>
T MinMaxProperty<T>::getIncrement() const {
    return increment_;
}

template <typename T>
T MinMaxProperty<T>::getMinSeparation() const {
    return minSeparation_;
}

template <typename T>
glm::tvec2<T, glm::defaultp> MinMaxProperty<T>::getRange() const {
    return range_;
}

template <typename T>
void MinMaxProperty<T>::set(const range_type& value) {
    if (value_.update(clamp(value))) this->propertyModified();
}

template <typename T>
MinMaxProperty<T>& MinMaxProperty<T>::setStart(const T& value) {
    set(range_type{value, this->value_.value.y});
    return *this;
}

template <typename T>
MinMaxProperty<T>& MinMaxProperty<T>::setEnd(const T& value) {
    set(range_type{this->value_.value.x, value});
    return *this;
}

template <typename T>
void MinMaxProperty<T>::set(const Property* srcProperty) {
    if (auto prop = dynamic_cast<const MinMaxProperty<T>*>(srcProperty)) {
        bool modified = false;
        modified |= range_.update(prop->range_);
        const bool rangeChanged = modified;
        modified |= increment_.update(prop->increment_);
        modified |= minSeparation_.update(prop->minSeparation_);
        modified |= value_.update(clamp(prop->value_));
        if (modified) this->propertyModified();
        if (rangeChanged) onRangeChangeCallback_.invokeAll();
    } else {
        TemplateProperty<range_type>::set(srcProperty);
    }
}

template <typename T>
MinMaxProperty<T>& MinMaxProperty<T>::setRangeMin(const T& newMin) {
    return setRange({newMin, std::max(newMin, range_.value.y)});
}

template <typename T>
MinMaxProperty<T>& MinMaxProperty<T>::setRangeMax(const T& newMax) {
    return setRange({std::min(range_.value.x, newMax), newMax});
}

template <typename T>
MinMaxProperty<T>& MinMaxProperty<T>::setIncrement(const T& newIncrement) {
    if (increment_.update(newIncrement)) this->propertyModified();
    return *this;
}

template <typename T>
MinMaxProperty<T>& MinMaxProperty<T>::setMinSeparation(const T& newMinSeparation) {
    bool modified = false;
    modified |= minSeparation_.update(limitSeparation(newMinSeparation));
    modified |= value_.update(clamp(value_.value));
    if (modified) this->propertyModified();
    return *this;
}

template <typename T>
MinMaxProperty<T>& MinMaxProperty<T>::setRange(const range_type& newRange) {
    if (range_.update({glm::min(newRange.x, newRange.y), glm::max(newRange.x, newRange.y)})) {
        value_.update(clamp(value_));

        this->propertyModified();
        onRangeChangeCallback_.invokeAll();
    }
    return *this;
}

template <typename T>
void MinMaxProperty<T>::set(const range_type& newValue, const range_type& newRange,
                            const T& newIncrement, const T& newMinSep) {

    bool modified = false;
    modified |= range_.update({glm::min(newRange.x, newRange.y), glm::max(newRange.x, newRange.y)});
    const bool rangeModified = modified;

    modified |= increment_.update(newIncrement);
    modified |= minSeparation_.update(limitSeparation(newMinSep));
    modified |= value_.update(clamp(newValue));

    if (modified) this->propertyModified();
    if (rangeModified) onRangeChangeCallback_.invokeAll();
}

template <typename T>
void MinMaxProperty<T>::set(const T& start, const T& end, const T& rangeMin, const T& rangeMax,
                            const T& increment, const T& minSep) {
    set(range_type(start, end), range_type(rangeMin, rangeMax), increment, minSep);
}

template <typename T>
MinMaxProperty<T>& MinMaxProperty<T>::setRangeNormalized(const range_type& newRange) {
    const auto nomalizedValue =
        (dvec2{value_.value} - static_cast<double>(range_.value.x)) /
        (static_cast<double>(range_.value.y) - static_cast<double>(range_.value.x));

    if (range_.update({glm::min(newRange.x, newRange.y), glm::max(newRange.x, newRange.y)})) {
        const range_type newVal = nomalizedValue * (static_cast<double>(range_.value.y) -
                                                    static_cast<double>(range_.value.x)) +
                                  static_cast<double>(range_.value.x);

        value_.update(clamp(newVal));
        this->propertyModified();
        onRangeChangeCallback_.invokeAll();
    }

    return *this;
}

template <typename T>
MinMaxProperty<T>& MinMaxProperty<T>::resetToDefaultState() {
    bool modified = false;
    modified |= range_.reset();
    modified |= increment_.reset();
    modified |= minSeparation_.reset();
    modified |= value_.reset();
    if (modified) this->propertyModified();
    return *this;
}

template <typename T>
const BaseCallBack* MinMaxProperty<T>::onRangeChange(std::function<void()> callback) {
    return onRangeChangeCallback_.addLambdaCallback(callback);
}

template <typename T>
void MinMaxProperty<T>::removeOnRangeChange(const BaseCallBack* callback) {
    onRangeChangeCallback_.remove(callback);
}

template <typename T>
MinMaxProperty<T>& MinMaxProperty<T>::setCurrentStateAsDefault() {
    TemplateProperty<range_type>::setCurrentStateAsDefault();
    range_.setAsDefault();
    increment_.setAsDefault();
    minSeparation_.setAsDefault();
    return *this;
}

template <typename T>
void MinMaxProperty<T>::serialize(Serializer& s) const {
    Property::serialize(s);

    range_.serialize(s, this->serializationMode_);
    increment_.serialize(s, this->serializationMode_);
    minSeparation_.serialize(s, this->serializationMode_);
    value_.serialize(s, this->serializationMode_);
}

template <typename T>
void MinMaxProperty<T>::deserialize(Deserializer& d) {
    Property::deserialize(d);

    bool modified = false;
    modified |= range_.deserialize(d, this->serializationMode_);
    modified |= increment_.deserialize(d, this->serializationMode_);
    modified |= minSeparation_.deserialize(d, this->serializationMode_);
    modified |= value_.deserialize(d, this->serializationMode_);
    if (modified) MinMaxProperty<T>::propertyModified();
}

template <typename T>
auto MinMaxProperty<T>::clamp(const range_type& v) const -> range_type {
    range_type val(glm::clamp(v, range_type(range_.value.x), range_type(range_.value.y)));
    if (val.x > val.y) std::swap(val.x, val.y);

    // check whether updated min/max values are separated properly, i.e. > minSeparation_
    if (glm::abs(val.y - val.x) < minSeparation_ - std::numeric_limits<T>::epsilon()) {
        // adjust max value if possible, i.e. less equal than max range
        if (val.x + minSeparation_ < range_.value.y + std::numeric_limits<T>::epsilon()) {
            val.y = glm::max(val.x + minSeparation_.value, val.y);
        } else {
            // otherwise adjust min value (min separation is at most rangeMax - rangeMin)
            val.y = range_.value.y;
            val.x = range_.value.y - minSeparation_;
        }
    }

    return val;
}

template <typename T>
T MinMaxProperty<T>::limitSeparation(T sep) const {
    // ensure that min separation is not larger than the entire range
    return sep < range_.value.y - range_.value.x ? sep : range_.value.y - range_.value.x;
}

template <typename T>
Document MinMaxProperty<T>::getDescription() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc = TemplateProperty<range_type>::getDescription();

    auto b = doc.get({P("html"), P("body")});
    utildoc::TableBuilder tb(b, P::end());
    tb(H("Min"), H("Start"), H("Stop"), H("Max"));
    tb(range_.value[0], value_.value[0], value_.value[1], range_.value[1]);

    utildoc::TableBuilder tb2(b, P::end());
    util::for_each_argument([&tb2](auto p) { tb2(H(camelCaseToHeader(p.name)), p.value); },
                            increment_, minSeparation_);

    return doc;
}

}  // namespace inviwo

#endif  // IVW_MINMAXPROPERTY_H
