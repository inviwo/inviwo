/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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

namespace inviwo {

/**
 * \ingroup properties
 * A property representing a range.
 */
template <typename T>
class MinMaxProperty : public TemplateProperty<glm::tvec2<T, glm::defaultp> > {
public:
    typedef glm::tvec2<T, glm::defaultp> range_type;
    InviwoPropertyInfo();

    MinMaxProperty(std::string identifier, std::string displayName,
                   T valueMin = Defaultvalues<T>::getMin(), T valueMax = Defaultvalues<T>::getMax(),
                   T rangeMin = Defaultvalues<T>::getMin(), T rangeMax = Defaultvalues<T>::getMax(),
                   T increment = Defaultvalues<T>::getInc(), T minSeperation = 0,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    MinMaxProperty(const MinMaxProperty& rhs) = default;
    MinMaxProperty& operator=(const MinMaxProperty& that) = default;
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

    void setStart(const T& value);
    void setEnd(const T& value);

    void setRangeMin(const T& value);
    void setRangeMax(const T& value);
    void setIncrement(const T& value);
    void setMinSeparation(const T& value);

    void setRange(const range_type& value);

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
    void setRangeNormalized(const range_type& newRange);

    const BaseCallBack* onRangeChange(std::function<void()> callback);
    void removeOnRangeChange(const BaseCallBack* callback);

    virtual void setCurrentStateAsDefault() override;
    virtual void resetToDefaultState() override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& s) override;

    static uvec2 getDim() { return Defaultvalues<T>::getDim(); }

    virtual Document getDescription() const override;

protected:
    /**
     * \brief validate the given value against the set min/max range
     *
     * @param v   value to be validated
     * @return returns the pair { modified, valid value } where modified indicates
     *            whether the given value was adjusted. The new value is stored as
     *            second parameter. In case there was not modification, valid value
     *            is equal to TemplateProperty<range_type>::value_.
     */
    auto validateValues(const range_type& v) -> std::pair<bool, range_type>;

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
PropertyClassIdentifier(MinMaxProperty<T>,
                        "org.inviwo." + Defaultvalues<T>::getName() + "MinMaxProperty");

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
    if (value == TemplateProperty<range_type>::value_.value) {
        return;
    }
    auto retVal = validateValues(value);
    if (retVal.first) {
        TemplateProperty<range_type>::value_ = retVal.second;
        MinMaxProperty<T>::propertyModified();
    }
}

template <typename T>
void MinMaxProperty<T>::setStart(const T& value) {
    auto val = TemplateProperty<range_type>::value_.value;
    val.x = value;
    set(val);
}

template <typename T>
void MinMaxProperty<T>::setEnd(const T& value) {
    auto val = TemplateProperty<range_type>::value_.value;
    val.y = value;
    set(val);
}

template <typename T>
void MinMaxProperty<T>::set(const Property* srcProperty) {
    if (auto prop = dynamic_cast<const MinMaxProperty<T>*>(srcProperty)) {
        bool rangeChanged = false;
        if (range_.value != prop->range_.value) {
            range_.value = prop->range_.value;
            rangeChanged = true;
        }
        increment_.value = prop->increment_.value;
        minSeparation_.value = prop->minSeparation_.value;
        TemplateProperty<range_type>::set(prop);
        if (rangeChanged) {
            onRangeChangeCallback_.invokeAll();
        }
    }
}

template <typename T>
void MinMaxProperty<T>::setRangeMin(const T& value) {
    if (range_.value.x == value) return;

    range_.value.x = value;
    // ensure that rangeMax is greater equal than rangeMin
    range_.value.y = std::max(range_.value.x, range_.value.y);

    value_.value = validateValues(value_.value).second;
    MinMaxProperty<T>::propertyModified();
    onRangeChangeCallback_.invokeAll();
}

template <typename T>
void MinMaxProperty<T>::setRangeMax(const T& value) {
    if (range_.value.y == value) return;

    range_.value.y = value;
    // ensure that rangeMin is less equal than rangeMax
    range_.value.x = std::min(range_.value.x, range_.value.y);

    value_.value = validateValues(value_.value).second;
    MinMaxProperty<T>::propertyModified();
    onRangeChangeCallback_.invokeAll();
}

template <typename T>
void MinMaxProperty<T>::setIncrement(const T& value) {
    if (increment_ == value) return;
    increment_ = value;
    MinMaxProperty<T>::propertyModified();
}

template <typename T>
void MinMaxProperty<T>::setMinSeparation(const T& value) {
    if (minSeparation_ == value) return;
    minSeparation_ = value;

    // ensure that min separation is not larger than the entire range
    if (minSeparation_ > (range_.value.y - range_.value.x)) {
        minSeparation_ = range_.value.y - range_.value.x;
    }

    value_.value = validateValues(value_.value).second;
    MinMaxProperty<T>::propertyModified();
}

template <typename T>
void MinMaxProperty<T>::setRange(const range_type& value) {
    if (value == range_.value) return;

    if (value.x < value.y) {
        range_.value = value;
    } else {
        range_.value = range_type(value.y, value.x);
    }

    value_.value = validateValues(value_.value).second;
    MinMaxProperty<T>::propertyModified();
    onRangeChangeCallback_.invokeAll();
}

template <typename T>
void MinMaxProperty<T>::set(const range_type& value, const range_type& range, const T& increment,
                            const T& minSep) {
    bool rangeModified = false;
    bool modified = false;

    if (range != range_.value) {
        if (range.x < range.y) {
            range_.value = range;
        } else {
            range_.value = range_type(range.y, range.x);
        }
        rangeModified = true;
    }
    if (increment != increment_) {
        increment_ = increment;
        modified = true;
    }
    if (minSep != minSeparation_) {
        minSeparation_.value = minSep;
        modified = true;
    }
    auto retVal = validateValues(value);
    if (retVal.first) {
        value_.value = retVal.second;
        modified = true;
    }

    if (modified || rangeModified) {
        MinMaxProperty<T>::propertyModified();
        if (rangeModified) {
            onRangeChangeCallback_.invokeAll();
        }
    }
}

template <typename T>
void MinMaxProperty<T>::set(const T& start, const T& end, const T& rangeMin, const T& rangeMax,
                            const T& increment, const T& minSep) {
    set(range_type(start, end), range_type(rangeMin, rangeMax), increment, minSep);
}

template <typename T>
void MinMaxProperty<T>::setRangeNormalized(const range_type& newRange) {
    dvec2 val = this->get();

    val = (val - static_cast<double>(range_.value.x)) /
          (static_cast<double>(range_.value.y) - static_cast<double>(range_.value.x));
    setRange(newRange);

    range_type newVal =
        val * (static_cast<double>(range_.value.y) - static_cast<double>(range_.value.x)) +
        static_cast<double>(range_.value.x);

    this->set(newVal);
}

template <typename T>
void MinMaxProperty<T>::resetToDefaultState() {
    range_.reset();
    increment_.reset();
    minSeparation_.reset();
    TemplateProperty<range_type>::resetToDefaultState();
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
void MinMaxProperty<T>::setCurrentStateAsDefault() {
    TemplateProperty<range_type>::setCurrentStateAsDefault();
    range_.setAsDefault();
    increment_.setAsDefault();
    minSeparation_.setAsDefault();
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
auto MinMaxProperty<T>::validateValues(const range_type& v) -> std::pair<bool, range_type> {
    range_type val(glm::clamp(v, range_type(range_.value.x), range_type(range_.value.y)));
    if (val.x > val.y) std::swap(val.x, val.y);

    // check whether updated min/max values are separated properly, i.e. > minSeparation_
    if (glm::abs(val.y - val.x) < minSeparation_ - glm::epsilon<T>()) {
        // adjust max value if possible, i.e. less equal than max range
        if (val.x + minSeparation_ < range_.value.y + glm::epsilon<T>()) {
            val.y = std::max(val.x + minSeparation_.value, val.y);
        } else {
            // otherwise adjust min value (min separation is at most rangeMax - rangeMin)
            val.y = range_.value.y;
            val.x = range_.value.y - minSeparation_;
        }
    }

    return {(val != value_.value), val};
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
