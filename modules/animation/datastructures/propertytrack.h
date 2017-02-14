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

#ifndef IVW_PROPERTYTRACK_H
#define IVW_PROPERTYTRACK_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/property.h>

#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/keyframesequence.h>

namespace inviwo {

namespace animation {

class IVW_MODULE_ANIMATION_API BasePropertyTrack {
public:
    virtual ~BasePropertyTrack() = default;
    virtual void setProperty(Property* property) = 0;
    virtual const Property* getProperty() const = 0;
    virtual Property* getProperty() = 0;
    virtual const std::string& getIdentifier() const = 0;
    virtual void addKeyFrameUsingPropertyValue(Seconds time) = 0;
    virtual void addSequenceUsingPropertyValue(Seconds time) = 0;
    virtual Track* toTrack() = 0;
};

template <typename Prop, typename Key>
class PropertyTrack : public TrackTyped<Key>, public BasePropertyTrack {
public:
    static_assert(std::is_same<typename std::decay<decltype(std::declval<Prop>().get())>::type,
                               typename Key::value_type>::value,
                  "The value type of Prop has to match that of Key");

    PropertyTrack();

    PropertyTrack(Prop* property);
    virtual ~PropertyTrack() = default;

    static std::string classIdentifier();

    virtual std::string getClassIdentifier() const override;

    virtual void setEnabled(bool enabled) override;
    virtual bool isEnabled() const override;

    virtual void setIdentifier(const std::string& identifier) override;

    virtual const std::string& getIdentifier() const override;

    virtual void setName(const std::string& name) override;
    virtual const std::string& getName() const override;

    virtual void setPriority(size_t priority) override;

    virtual size_t getPriority() const override;

    virtual Seconds firstTime() const override;

    virtual Seconds lastTime() const override;

    virtual AniamtionTimeState operator()(Seconds from, Seconds to,
                                          AnimationState state) const override;

    virtual const Prop* getProperty() const override;
    virtual Prop* getProperty() override;

    virtual void addTyped(const KeyframeSequenceTyped<Key>& sequence) override;

    virtual void add(const KeyframeSequence& sequence) override;

    virtual size_t size() const override;

    virtual KeyframeSequenceTyped<Key>& operator[](size_t i) override;

    virtual const KeyframeSequenceTyped<Key>& operator[](size_t i) const override;

    virtual void remove(size_t i) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual void setProperty(Property* property) override;

    virtual void addKeyFrameUsingPropertyValue(Seconds time) override;
    virtual void addSequenceUsingPropertyValue(Seconds time) override;

    virtual Track* toTrack() override;

private:
    virtual void onKeyframeSequenceMoved(KeyframeSequence* seq) override;

    Prop* property_;  ///< non-owning reference
    bool enabled_{true};
    std::string identifier_;
    std::string name_;
    size_t priority_{100};

    // Sorted list of non-overlapping sequences of key frames
    std::vector<std::unique_ptr<KeyframeSequenceTyped<Key>>> sequences_;
};


template <typename Prop, typename Key>
Track* inviwo::animation::PropertyTrack<Prop, Key>::toTrack() {
    return this;
}

template <typename Prop, typename Key>
PropertyTrack<Prop, Key>::PropertyTrack(Prop* property)
    : TrackTyped<Key>()
    , property_(property)
    , identifier_(property_->getIdentifier())
    , name_(property_->getDisplayName()) {}

template <typename Prop, typename Key>
PropertyTrack<Prop, Key>::PropertyTrack()
    : TrackTyped<Key>(), property_(nullptr), identifier_(""), name_("") {}

template <typename Prop, typename Key>
std::string PropertyTrack<Prop, Key>::classIdentifier() {
    auto keyid = Key::classIdentifier();
    std::string id = "org.inviwo.animation.propertytrack.";
    auto res = std::mismatch(id.begin(), id.end(), keyid.begin(), keyid.end());
    id.append(res.second, keyid.end());
    return id;
}

template <typename Prop, typename Key>
std::string PropertyTrack<Prop, Key>::getClassIdentifier() const {
    return classIdentifier();
}

template <typename Prop, typename Key>
size_t PropertyTrack<Prop, Key>::getPriority() const {
    return priority_;
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::setPriority(size_t priority) {
    if (priority_ != priority) {
        priority_ = priority;
        this->notifyPriorityChanged(this);
    }
}

template <typename Prop, typename Key>
const std::string& PropertyTrack<Prop, Key>::getName() const {
    return name_;
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::setName(const std::string& name) {
    if (name_ != name) {
        name_ = name;
        this->notifyNameChanged(this);
    }
}

template <typename Prop, typename Key>
const std::string& PropertyTrack<Prop, Key>::getIdentifier() const {
    return identifier_;
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::setIdentifier(const std::string& identifier) {
    if (identifier_ != identifier) {
        identifier_ = identifier;
        this->notifyIdentifierChanged(this);
    }
}

template <typename Prop, typename Key>
bool PropertyTrack<Prop, Key>::isEnabled() const {
    return enabled_;
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::setEnabled(bool enabled) {
    enabled_ = enabled;
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
        setIdentifier(property_->getIdentifier());
        setName(property_->getDisplayName());
    } else {
        throw Exception("Invalid property set to track", IvwContext);
    }
}

template <typename Prop, typename Key>
const KeyframeSequenceTyped<Key>& PropertyTrack<Prop, Key>::operator[](size_t i) const {
    return *sequences_[i];
}

template <typename Prop, typename Key>
KeyframeSequenceTyped<Key>& PropertyTrack<Prop, Key>::operator[](size_t i) {
    return *sequences_[i];
}

template <typename Prop, typename Key>
size_t PropertyTrack<Prop, Key>::size() const {
    return sequences_.size();
}


/**
 * Track of sequences
 * ----------X======X====X-----------X=========X-------X=====X--------
 * |- case 1-|-case 2----------------|-case 2----------|-case 2------|
 *           |-case 2a---|-case 2b---|
 */
template <typename Prop, typename Key>
AniamtionTimeState PropertyTrack<Prop, Key>::operator()(Seconds from, Seconds to,
                                                        AnimationState state) const {
    if (!enabled_ || sequences_.empty()) return {to, state};

    // 'it' will be the first seq. with a first time larger then 'to'.
    auto it = std::upper_bound(
        sequences_.begin(), sequences_.end(), to,
        [](const auto& time, const auto& seq) { return time < seq->getFirst().getTime(); });

    if (it == sequences_.begin()) {
        if (from > (*it)->getFirst().getTime()) {  // case 1
            property_->set((*it)->getFirst().getValue());
        }
    } else {  // case 2
        auto& seq1 = *std::prev(it);

        if (to < seq1->getLast().getTime()) {  // case 2a
            property_->set((*seq1)(from, to));
        } else {  // case 2b
            if (from < seq1->getLast().getTime()) {
                // We came from before the previous key
                property_->set(seq1->getLast().getValue());
            } else if (it != sequences_.end() && from > (*it)->getFirst().getTime()) {
                // We came form after the next key
                property_->set((*it)->getFirst().getValue());
            }
            // we moved in an unmarked region, do nothing.
        }
    }
    return {to, state};
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::add(const KeyframeSequence& sequence) {
    addTyped(dynamic_cast<const KeyframeSequenceTyped<Key>&>(sequence));
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::addTyped(const KeyframeSequenceTyped<Key>& sequence) {
    auto it = std::upper_bound(
        sequences_.begin(), sequences_.end(), sequence.getFirst().getTime(),
        [](const auto& time, const auto& seq) { return time < seq->getFirst().getTime(); });

    if (it != sequences_.begin()) {
        if ((*std::prev(it))->getLast().getTime() > sequence.getFirst().getTime()) {
            throw Exception("Overlapping Sequence", IvwContext);
        }
    }
    if (it != sequences_.end() && (*it)->getFirst().getTime() < sequence.getLast().getTime()) {
        throw Exception("Overlapping Sequence", IvwContext);
    }

    auto inserted = sequences_.insert(it, std::make_unique<KeyframeSequenceTyped<Key>>(sequence));
    this->notifyKeyframeSequenceAdded(this, inserted->get());
    (*inserted)->addObserver(this);
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::remove(size_t i) {
    auto seq = std::move(sequences_[i]);
    sequences_.erase(sequences_.begin() + i);
    this->notifyKeyframeSequenceRemoved(this, seq.get());
}

template <typename Prop, typename Key>
Seconds PropertyTrack<Prop, Key>::lastTime() const {
    if (sequences_.empty()) {
        return Seconds{0.0};
    } else {
        return sequences_.back()->getLast().getTime();
    }
}

template <typename Prop, typename Key>
Seconds PropertyTrack<Prop, Key>::firstTime() const {
    if (sequences_.empty()) {
        return Seconds{0.0};
    } else {
        return sequences_.front()->getFirst().getTime();
    }
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::onKeyframeSequenceMoved(KeyframeSequence* key) {
    std::stable_sort(sequences_.begin(), sequences_.end(), [](const auto& a, const auto& b) {
        return a->getFirst().getTime() < b->getFirst().getTime();
    });
    /// Do validation?

    auto it = std::find_if(sequences_.begin(), sequences_.end(), [&](const auto& item){
        return item.get() == key;
    });
    if (it != sequences_.end()) {
        if (it == sequences_.begin()) {
            this->notifyFirstMoved(this);
        } else if (it == std::prev(sequences_.end())) {
            this->notifyLastMoved(this);
        }
    }
}

/**
 * Track of sequences
 * ----------X======X====X-----------------X=========X-------X=====X--------
 * |- case 1-|-case 2----------------------|-case 2----------|-case 3------|
 *           |-case 2a-----------|-case 2b-|
 */
template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::addKeyFrameUsingPropertyValue(Seconds time) {
    if (sequences_.empty()) {
        KeyframeSequenceTyped<Key> sequence({{time, property_->get()}},
                                            std::make_unique<LinearInterpolation<Key>>());
        addTyped(sequence);
    } else {
        // 'it' will be the first seq. with a first time larger then 'to'.
        auto it = std::upper_bound(
            sequences_.begin(), sequences_.end(), time,
            [](const auto& t, const auto& seq) { return t < seq->getFirst().getTime(); });

        if (it == sequences_.begin()) {   // case 1
            sequences_[0]->add(Key{time, property_->get()});
        } else if (it == sequences_.end()) { // case 3
            auto& seq1 = *std::prev(it);
            seq1->add(Key{time, property_->get()});
        } else {                          // case 2
            auto& seq1 = *std::prev(it);
            auto& seq2 = *it;
            if ((time - seq1->getLast().getTime()) < (seq2->getFirst().getTime() - time)) {
                seq1->add(Key{time, property_->get()});  // case 2a
            } else {
                seq2->add(Key{time, property_->get()});  // case 2b
            }
        }
    }
}


template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::addSequenceUsingPropertyValue(Seconds time) {
    KeyframeSequenceTyped<Key> sequence({ {time, property_->get()} },
                                        std::make_unique<LinearInterpolation<Key>>());
    addTyped(sequence);
}


template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("identifier", identifier_, SerializationTarget::Attribute);
    s.serialize("name", name_);
    s.serialize("enabled", enabled_);
    s.serialize("priority", priority_);
    s.serialize("property", property_);
    s.serialize("sequences", sequences_, "sequence");
}

template <typename Prop, typename Key>
void PropertyTrack<Prop, Key>::deserialize(Deserializer& d) {
    std::string className;
    d.deserialize("type", className, SerializationTarget::Attribute);
    if (className != getClassIdentifier()) {
        throw SerializationException(
            "Deserialized animation track: " + getClassIdentifier() +
                " from a serialized track with a different class identifier: " + className,
            IvwContext);
    }
    {
        auto old = identifier_;
        d.deserialize("identifier", identifier_, SerializationTarget::Attribute);
        if (old != identifier_) this->notifyIdentifierChanged(this);
    }
    {
        auto old = name_;
        d.deserialize("name", name_);
        if (old != name_) this->notifyNameChanged(this);
    }
    {
        auto old = enabled_;
        d.deserialize("enabled", enabled_);
        if (old != enabled_) this->notifyEnabledChanged(this);
    }
    {
        auto old = priority_;
        d.deserialize("priority", priority_);
        if (old != priority_) this->notifyPriorityChanged(this);
    }

    using Elem = std::unique_ptr<KeyframeSequenceTyped<Key>>;
    util::IndexedDeserializer<Elem>("sequences", "sequence")
        .onNew([&](Elem& seq) {
            this->notifyKeyframeSequenceAdded(this, seq.get());
            seq->addObserver(this);
        })
        .onRemove([&](Elem& seq) { this->notifyKeyframeSequenceRemoved(this, seq.get()); })(
            d, sequences_);

    d.deserializeAs<Property>("property", property_);
}

} // namespace

} // namespace

#endif // IVW_PROPERTYTRACK_H

