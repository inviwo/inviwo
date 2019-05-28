/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_PROPERTYTRACK_H
#define IVW_PROPERTYTRACK_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/templateproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <modules/animation/datastructures/valuekeyframe.h>
#include <modules/animation/datastructures/basetrack.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/valuekeyframesequence.h>
#include <modules/animation/interpolation/linearinterpolation.h>

namespace inviwo {

namespace animation {

namespace detail {

/**
 * Helper function for inviwo::animation::PropertyTrack::setOtherProperty
 * @see inviwo::animation::BasePropertyTrack::setOtherProperty
 */
template <typename T>
void setOtherPropertyHelper(TemplateProperty<T>* property, ValueKeyframe<T>* keyframe) {
    property->set(keyframe->getValue());
}

/**
 * Helper function for inviwo::animation::PropertyTrack::setOtherProperty
 * @see inviwo::animation::BasePropertyTrack::setOtherProperty
 */
template <typename T>
void setOtherPropertyHelper(TemplateOptionProperty<T>* property, ValueKeyframe<T>* keyframe) {
    property->setSelectedValue(keyframe->getValue());
}

/**
 * Helper function for inviwo::animation::PropertyTrack::updateKeyframeFromProperty
 * @see inviwo::animation::BasePropertyTrack::updateKeyframeFromProperty
 */
template <typename T>
void updateKeyframeFromPropertyHelper(TemplateProperty<T>* property, ValueKeyframe<T>* keyframe) {
    keyframe->setValue(property->get());
}

/**
 * Helper function for inviwo::animation::PropertyTrack::updateKeyframeFromProperty
 * @see inviwo::animation::BasePropertyTrack::updateKeyframeFromProperty
 */
template <typename T>
void updateKeyframeFromPropertyHelper(TemplateOptionProperty<T>* property,
                                      ValueKeyframe<T>* keyframe) {
    keyframe->setValue(property->getSelectedValue());
}

}  // namespace detail

/** \class BasePropertyTrack
 * Interface for tracks based on a Property.
 * Exposes functions for adding a KeyFrame and KeyFrameSequence
 * using the current values of the Property.
 * @see Track
 * @see PropertyTrack
 * @see Property
 */
class IVW_MODULE_ANIMATION_API BasePropertyTrack {
public:
    virtual ~BasePropertyTrack() = default;
    virtual void setProperty(Property* property) = 0;
    virtual const Property* getProperty() const = 0;
    virtual Property* getProperty() = 0;
    virtual const std::string& getIdentifier() const = 0;
    /*
     * Add KeyFrame at specified time using the current value of supplied property.
     * All keyframes in one sequence uses the same interpolation so the provided interpolation
     * will only be used if a new sequence need to be created.
     *
     * See example below on which KeyFrameSequence the KeyFrame is added:
     * Track of sequences
     *                  Sequence 1                   Sequence 2        Seq. 3
     * Track: ----------X======X====X-----------------X=========X-------X=====X--------
     *        |- case 1-|-case 2----------------------|-case 2----------|-case 3------|
     *                  |-case 2a-----------|-case 2b-|
     *
     * Case 0: No sequence exist. Create sequence at time and use provided interpolation for
     * sequence. Case 1: Time is before first sequence add KeyFrame to Sequence 1. Case 2: Time is
     * inside an existing sequence, add KeyFrame to this sequence. Case 2a: Time is closest to
     * Sequence 1, add it to this sequence. Case 2b: Time is closest to Sequence 2, add it to this
     * sequence. Case 3: Time is in, or after, last sequence, add it to this sequence.
     *
     * @param proprty Property to create key frame value from.
     * @param time at which KeyFrame should be added.
     * @param interpolation to use if a new sequence is created
     * @throw Exception If supplied property is not of same type as BasePropertyTrack::getProperty
     * @throw Exception If Interpolation type is invalid for property value.
     */
    virtual void addKeyFrameUsingPropertyValue(const Property* property, Seconds time,
                                               std::unique_ptr<Interpolation> interpolation) = 0;
    /*
     * Add KeyFrame at specified time using the current value of the property.
     * @see addKeyFrameUsingPropertyValue(const Property* property, Seconds time,
     * std::unique_ptr<Interpolation> interpolation)
     * @param time at which KeyFrame should be added.
     * @param interpolation to use if a new sequence is created
     */
    virtual void addKeyFrameUsingPropertyValue(Seconds time,
                                               std::unique_ptr<Interpolation> interpolation) = 0;
    /*
     * Add KeyFrameSequence at specified time using the current value of the property.
     * @param time at which KeyFrame should be added.
     * @param interpolation to use for the new sequence.
     * @throw Exception If a sequence already exist at time
     */
    virtual void addSequenceUsingPropertyValue(Seconds time,
                                               std::unique_ptr<Interpolation> interpolation) = 0;
    virtual Track* toTrack() = 0;

    virtual void setOtherProperty(Property*, Keyframe*){};            // Should this be pure virtual
    virtual void updateKeyframeFromProperty(Property*, Keyframe*){};  // Should this be pure
};

/** \class PropertyTrack
 * Implementation of BasePropertyTrack and TrackTyped based on templates parameter types for
 * Property and KeyFrame.
 * Exposes functions for adding a KeyFrame and KeyFrameSequence
 * using the current values of the Property.
 * @see Track
 * @see PropertyTrack
 * @see Property
 */
template <typename Prop, typename Key>
class PropertyTrack : public BaseTrack<KeyframeSequenceTyped<Key>>, public BasePropertyTrack {
public:
    static_assert(std::is_same<typename std::decay<decltype(std::declval<Prop>().get())>::type,
                               typename Key::value_type>::value,
                  "The value type of Prop has to match that of Key");

    PropertyTrack();
    PropertyTrack(Prop* property);
    /**
     * Remove all keyframe sequences and call TrackObserver::notifyKeyframeSequenceRemoved
     */
    virtual ~PropertyTrack();

    static std::string classIdentifier();
    virtual std::string getClassIdentifier() const override;

    virtual AnimationTimeState operator()(Seconds from, Seconds to,
                                          AnimationState state) const override;

    virtual const Prop* getProperty() const override;
    virtual Prop* getProperty() override;
    virtual void setProperty(Property* property) override;
    virtual const std::string& getIdentifier() const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual void addKeyFrameUsingPropertyValue(
        const Property* property, Seconds time,
        std::unique_ptr<Interpolation> interpolation) override;
    virtual void addKeyFrameUsingPropertyValue(
        Seconds time, std::unique_ptr<Interpolation> interpolation) override;
    virtual void addSequenceUsingPropertyValue(
        Seconds time, std::unique_ptr<Interpolation> interpolation) override;

    // BasePropertyTrack overload
    virtual Track* toTrack() override;

    /**
     * \brief Helper function to set a property (other than the property owned by the track) from a
     * keyframe
     *
     * Called from inviwo::animation:: KeyframeEditorWidget when creating the widget
     *
     * @param dstProperty The property to set
     * @param keyframe The keyframe to set from
     */
    void setOtherProperty(Property* dstProperty, Keyframe* keyframe) override {
        IVW_ASSERT(dstProperty->getClassIdentifier() == PropertyTraits<Prop>::classIdentifier(),
                   "Incorrect Property type");
        detail::setOtherPropertyHelper(static_cast<Prop*>(dstProperty),
                                       static_cast<Key*>(keyframe));
    }

    /**
     * \brief Helper function to update the value if a keyframe from a property (other than the
     * property owned by the track)
     *
     * Called from inviwo::animation::KeyframeEditorWidget when the value of the keyframe is updated
     * through the widget
     *
     * @param srcProperty The property to set from
     * @param keyframe The keyframe to set
     */
    void updateKeyframeFromProperty(Property* srcProperty, Keyframe* keyframe) override {
        ivwAssert(srcProperty->getClassIdentifier() == PropertyTraits<Prop>::classIdentifier(),
                  "Incorrect Property type");
        detail::updateKeyframeFromPropertyHelper(static_cast<Prop*>(srcProperty),
                                                 static_cast<Key*>(keyframe));
    }

private:
    Prop* property_;  ///< non-owning reference
};

template <typename Prop, typename Key>
bool operator==(const PropertyTrack<Prop, Key>& a, const PropertyTrack<Prop, Key>& b) {
    return std::equal(a.begin(), a.end(), a.begin(), b.end());
}
template <typename Prop, typename Key>
bool operator!=(const PropertyTrack<Prop, Key>& a, const PropertyTrack<Prop, Key>& b) {
    return !(a == b);
}

template <typename Prop, typename Key>
Track* PropertyTrack<Prop, Key>::toTrack() {
    return this;
}

template <typename Prop, typename Key>
PropertyTrack<Prop, Key>::PropertyTrack()
    : BaseTrack<KeyframeSequenceTyped<Key>>{"", "", 100}, property_(nullptr) {}

template <typename Prop, typename Key>
PropertyTrack<Prop, Key>::PropertyTrack(Prop* property)
    : BaseTrack<KeyframeSequenceTyped<Key>>{property->getIdentifier(), property->getDisplayName(),
                                            100}
    , property_(property) {}

template <typename Prop, typename Key>
PropertyTrack<Prop, Key>::~PropertyTrack() = default;

template <typename Prop, typename Key>
std::string PropertyTrack<Prop, Key>::classIdentifier() {
    // Use property class identifier since multiple properties
    // may have the same key (data type)
    std::string id =
        "org.inviwo.animation.PropertyTrack.for. " + PropertyTraits<Prop>::classIdentifier();
    return id;
}

template <typename Prop, typename Key>
const std::string& PropertyTrack<Prop, Key>::getIdentifier() const {
    return BaseTrack<KeyframeSequenceTyped<Key>>::getIdentifier();
}

template <typename Prop, typename Key>
std::string PropertyTrack<Prop, Key>::getClassIdentifier() const {
    return classIdentifier();
}

template <typename Prop, typename Key>
Prop* PropertyTrack<Prop, Key>::getProperty() {
    return property_;
}

template <typename Prop, typename Key>
const Prop* PropertyTrack<Prop, Key>::getProperty() const {
    return property_;
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::setProperty(Property* property) {
    if (auto prop = dynamic_cast<Prop*>(property)) {
        property_ = prop;
        this->setIdentifier(property_->getIdentifier());
        this->setName(property_->getDisplayName());
    } else {
        throw Exception("Invalid property set to track", IVW_CONTEXT);
    }
}

/**
 * Track of sequences
 * ----------X======X====X-----------X=========X-------X=====X--------
 * |- case 1-|-case 2----------------|-case 2----------|-case 2------|
 *           |-case 2a---|-case 2b---|
 */
template <typename Prop, typename Key>
AnimationTimeState PropertyTrack<Prop, Key>::operator()(Seconds from, Seconds to,
                                                        AnimationState state) const {
    if (!this->isEnabled() || this->empty()) return {to, state};

    // 'it' will be the first seq. with a first time larger then 'to'.
    auto it = std::upper_bound(this->begin(), this->end(), to,
                               [](const auto& a, const auto& b) { return a < b; });

    if (it == this->begin()) {
        if (from > it->getFirstTime()) {  // case 1
            property_->set(it->getFirst().getValue());
        }
    } else {  // case 2
        auto& seq1 = *std::prev(it);

        if (to < seq1.getLastTime()) {  // case 2a
            property_->set(seq1(from, to));
        } else {  // case 2b
            if (from < seq1.getLastTime()) {
                // We came from before the previous key
                property_->set(seq1.getLast().getValue());
            } else if (it != this->end() && from > it->getFirstTime()) {
                // We came form after the next key
                property_->set(it->getFirst().getValue());
            }
            // we moved in an unmarked region, do nothing.
        }
    }
    return {to, state};
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::addKeyFrameUsingPropertyValue(
    const Property* property, Seconds time, std::unique_ptr<Interpolation> interpolation) {
    auto prop = dynamic_cast<const Prop*>(property);
    if (!prop) {
        throw Exception("Cannot add key frame from property type " +
                        property->getClassIdentifier() + " for " + property_->getClassIdentifier());
    }
    if (this->empty()) {
        // Use provided interpolation if we can
        if (auto ip = dynamic_cast<InterpolationTyped<Key>*>(interpolation.get())) {
            interpolation.release();

            std::vector<std::unique_ptr<Key>> keys;
            keys.push_back(std::make_unique<Key>(time, prop->get()));
            auto sequence = std::make_unique<KeyframeSequenceTyped<Key>>(
                std::move(keys), std::unique_ptr<InterpolationTyped<Key>>(ip));
            this->add(std::move(sequence));
        } else {
            throw Exception("Invalid interpolation " + interpolation->getClassIdentifier() +
                            " for " + getClassIdentifier());
        }

    } else {
        this->addToClosestSequence(std::make_unique<Key>(time, prop->get()));
    }
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::addKeyFrameUsingPropertyValue(
    Seconds time, std::unique_ptr<Interpolation> interpolation) {
    addKeyFrameUsingPropertyValue(property_, time, std::move(interpolation));
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::addSequenceUsingPropertyValue(
    Seconds time, std::unique_ptr<Interpolation> interpolation) {
    if (auto ip = dynamic_cast<InterpolationTyped<Key>*>(interpolation.get())) {
        interpolation.release();

        std::vector<std::unique_ptr<Key>> keys;
        keys.push_back(std::make_unique<Key>(time, property_->get()));
        auto sequence = std::make_unique<KeyframeSequenceTyped<Key>>(
            std::move(keys), std::unique_ptr<InterpolationTyped<Key>>(ip));
        this->add(std::move(sequence));

    } else {
        throw Exception("Invalid interpolation " + interpolation->getClassIdentifier() + " for " +
                        getClassIdentifier());
    }
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::serialize(Serializer& s) const {
    BaseTrack<KeyframeSequenceTyped<Key>>::serialize(s);
    s.serialize("property", property_);
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::deserialize(Deserializer& d) {
    BaseTrack<KeyframeSequenceTyped<Key>>::deserialize(d);
    d.deserializeAs<Property>("property", property_);
}

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_PROPERTYTRACK_H
