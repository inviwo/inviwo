/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_KEYFRAME_H
#define IVW_KEYFRAME_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <modules/animation/datastructures/keyframeobserver.h>
#include <modules/animation/datastructures/animationtime.h>

namespace inviwo {

namespace animation {

/**
 * The Keyframe is a element of a KeyframeSequence. The base interface Keyframe only provied
 * a Time property.
 */
class IVW_MODULE_ANIMATION_API Keyframe : public Serializable, public KeyframeObservable {
public:
    Keyframe() = default;
    virtual ~Keyframe() = default;

    virtual void setTime(Seconds time) = 0;
    virtual Seconds getTime() const = 0;

    virtual void serialize(Serializer& s) const override = 0;
    virtual void deserialize(Deserializer& d) override = 0;

    virtual std::string getClassIdentifier() const = 0;
};

IVW_MODULE_ANIMATION_API bool operator<(const Keyframe& a, const Keyframe& b);
IVW_MODULE_ANIMATION_API bool operator<=(const Keyframe& a, const Keyframe& b);
IVW_MODULE_ANIMATION_API bool operator>(const Keyframe& a, const Keyframe& b);
IVW_MODULE_ANIMATION_API bool operator>=(const Keyframe& a, const Keyframe& b);

template <typename T>
class ValueKeyframe : public Keyframe {
public:
    using value_type = T;
    ValueKeyframe() = default;

    ValueKeyframe(Seconds time, const T& value) : time_(time), value_(value) {}
    virtual ~ValueKeyframe() = default;

    ValueKeyframe(const ValueKeyframe& rhs) = default;
    ValueKeyframe& operator=(const ValueKeyframe& that) {
        if (this != &that) {
            value_ = that.value_;
            setTime(that.time_);
        }
        return *this;
    }

    virtual void setTime(Seconds time) override {
        if (time != time_) {
            auto oldTime = time_;
            time_ = time;
            notifKeyframeTimeChanged(this, oldTime);
        }
    }
    virtual Seconds getTime() const override { return time_; }

    const T& getValue() const { return value_; }
    T& getValue() { return value_; }

    void setValue(const T& value) { value_ = value; }

    static std::string classIdentifier();
    virtual std::string getClassIdentifier() const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    Seconds time_{0.0};
    T value_{0};
};

template <typename T>
std::string inviwo::animation::ValueKeyframe<T>::classIdentifier() {
    return "org.inviwo.animation.ValueKeyframe." + Defaultvalues<T>::getName();
}

template <typename T>
std::string inviwo::animation::ValueKeyframe<T>::getClassIdentifier() const {
    return classIdentifier();
}

template <typename T>
bool operator==(const ValueKeyframe<T>& a, const ValueKeyframe<T>& b) {
    return a.getTime() == b.getTime() && a.getValue() == b.getValue();
}
template <typename T>
bool operator!=(const ValueKeyframe<T>& a, const ValueKeyframe<T>& b) {
    return !(a == b);
}

template <typename T>
void ValueKeyframe<T>::serialize(Serializer& s) const {
    // s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("time", time_.count());
    s.serialize("value", value_);
}

template <typename T>
void ValueKeyframe<T>::deserialize(Deserializer& d) {
    /* std::string className;
     d.deserialize("type", className, SerializationTarget::Attribute);
     if (className != getClassIdentifier()) {
         throw SerializationException(
             "Deserialized keyframe: " + getClassIdentifier() +
                 " from a serialized keyframe with a different class identifier: " + className,
             IvwContext);
     }*/
    double tmp = time_.count();
    d.deserialize("time", tmp);
    time_ = Seconds{tmp};
    d.deserialize("value", value_);
}

}  // namespace

}  // namespace

#endif  // IVW_KEYFRAME_H
