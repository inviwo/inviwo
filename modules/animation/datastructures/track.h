/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_TRACK_H
#define IVW_TRACK_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/io/serialization/serializable.h>

#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/trackobserver.h>

namespace inviwo {

namespace animation {

/**
 * \class Track
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_ANIMATION_API Track : public Serializable,
                                       public TrackObservable,
                                       public KeyframeSequenceObserver {
public:
    Track() = default;
    virtual ~Track() = default;
    Track(const Track&) = delete;
    Track& operator=(const Track&) = delete;

    virtual std::string getClassIdentifier() const = 0;

    virtual void setEnabled(bool enabled) = 0;
    virtual bool isEnabled() const = 0;

    virtual void setIdentifier(const std::string& identifier) = 0;
    virtual const std::string& getIdentifier() const = 0;

    virtual void setName(const std::string& name) = 0;
    virtual const std::string& getName() const = 0;

    virtual void setPriority(size_t priority) = 0;
    virtual size_t getPriority() const = 0;

    virtual Time firstTime() const = 0;
    virtual Time lastTime() const = 0;

    virtual size_t size() const = 0;

    virtual void operator()(Time from, Time to) const = 0;

    virtual KeyframeSequence& operator[](size_t i) = 0;
    virtual const KeyframeSequence& operator[](size_t i) const = 0;

    virtual void add(const KeyframeSequence& sequence) = 0;
    virtual void remove(size_t i) = 0;

    virtual void serialize(Serializer& s) const override = 0;
    virtual void deserialize(Deserializer& d) override = 0;
};

template <typename Key>
class TrackTyped : public Track {
public:
    TrackTyped() = default;
    virtual ~TrackTyped() = default;

    virtual KeyframeSequenceTyped<Key>& operator[](size_t i) override = 0;
    virtual const KeyframeSequenceTyped<Key>& operator[](size_t i) const override = 0;

    virtual void add(const KeyframeSequenceTyped<Key>& sequence) = 0;
};


template <typename Prop, typename Key>
class TrackProperty : public TrackTyped<Key> {
public:
    static_assert(std::is_same<typename std::decay<decltype(std::declval<Prop>().get())>::type,
                               typename Key::value_type>::value,
                  "The value type of Prop has to match that of Key");

    TrackProperty()
        : TrackTyped<Key>()
        , property_(nullptr)
        , identifier_("")
        , name_("") {}

    TrackProperty(Prop* property)
        : TrackTyped<Key>()
        , property_(property)
        , identifier_(property_->getIdentifier())
        , name_(property_->getDisplayName())
        {}
    virtual ~TrackProperty() = default;

    static std::string classIdentifier() {
        auto keyid = Key::classIdentifier();
        std::string id = "org.inviwo.animation.propertytrack.";
        auto res = std::mismatch(id.begin(), id.end(), keyid.begin(), keyid.end());
        id.append(res.second, keyid.end());
        return id;
    };
    virtual std::string getClassIdentifier() const override { return classIdentifier(); }

    virtual void setEnabled(bool enabled) override { enabled_ = enabled; }
    virtual bool isEnabled() const override { return enabled_; }

    virtual void setIdentifier(const std::string& identifier) override {
        if (identifier_ != identifier) {
            identifier_ = identifier;
            notifyIdentifierChanged(this);
        }
    };
    virtual const std::string& getIdentifier() const override { return identifier_; }

    virtual void setName(const std::string& name) override {
        if (name_ != name) {
            name_ = name;
            notifyNameChanged(this);
        }
    }
    virtual const std::string& getName() const override { return name_; }

    virtual void setPriority(size_t priority) override {
        if (priority_ != priority) {
            priority_ = priority;
            notifyPriorityChanged(this);
        }
    };
    virtual size_t getPriority() const override {return priority_;}

    virtual Time firstTime() const override {
        if (sequences_.empty()) {
            return Time{ 0.0 };
        } else {
            return sequences_.front()->getFirst().getTime();
        }
    };
    virtual Time lastTime() const override {
        if (sequences_.empty()) {
            return Time{ 0.0 };
        } else {
            return sequences_.back()->getLast().getTime();
        }
    }

    /**
     * Track of sequences
     * ----------X======X====X-----------X=========X-------X=====X--------
     * |- case 1-|-case 2----------------|-case 2----------|-case 2------|
     *           |-case 2a---|-case 2b---|
     */
    virtual void operator()(Time from, Time to) const override {
        if (!enabled_ || sequences_.empty()) return;

        // 'it' will be the first seq. with a first time larger then 'to'.
        auto it = std::upper_bound(
            sequences_.begin(), sequences_.end(), to,
            [](const auto& time, const auto& seq) { return time < seq->getFirst().getTime(); });

        if (it == sequences_.begin() && from > (*it)->getFirst().getTime()) { // case 1
             property_->set((*it)->getFirst().getValue());      
        } else {                                                              // case 2
            auto& seq1 = *std::prev(it);

            if (to < seq1->getLast().getTime() ) {                              // case 2a
                property_->set((*seq1)(from, to));
            } else {                                                            // case 2b    
                if( from < seq1->getLast().getTime() ) {
                    // We came from before the previous key
                    property_->set(seq1->getLast().getValue()); 
                } else if(it != sequences_.end() &&  from > (*it)->getFirst().getTime()) {
                    // We came form after the next key
                    property_->set((*it)->getFirst().getValue());
                }
                // we moved in an unmarked region, do nothing.
            }
        }
    };

    const Prop* getProperty() const { return property_; }
    Prop* getProperty() {return property_; }

    virtual void add(const KeyframeSequenceTyped<Key>& sequence) override {
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

        auto inserted =
            sequences_.insert(it, std::make_unique<KeyframeSequenceTyped<Key>>(sequence));
        notifyKeyframeSequenceAdded(inserted->get());
    };

    virtual void add(const KeyframeSequence& sequence) override {
        add(dynamic_cast<const KeyframeSequenceTyped<Key>&>(sequence));      
    }

    virtual size_t size() const override {
       return sequences_.size();
    }

    virtual KeyframeSequenceTyped<Key>& operator[](size_t i) override {
        return *sequences_[i];
    }

    virtual const KeyframeSequenceTyped<Key>& operator[](size_t i) const override {
        return *sequences_[i];
    }

    virtual void remove(size_t i) override {
        auto seq = std::move(sequences_[i]);
        sequences_.erase(sequences_.begin()+i);
        notifyKeyframeSequenceRemoved(seq.get());
    }

    virtual void serialize(Serializer& s) const override {
        s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
        s.serialize("identifier", identifier_, SerializationTarget::Attribute);
        s.serialize("name", name_);
        s.serialize("enabled", enabled_);
        s.serialize("priority", priority_);
        s.serialize("property", property_);
        s.serialize("sequences", sequences_, "sequence");
    };
    virtual void deserialize(Deserializer& d) override {
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
            if (old != identifier_) notifyIdentifierChanged(this);
        }
        {
            auto old = name_;
            d.deserialize("name", name_);
            if (old != name_) notifyNameChanged(this);
        }
        {
            auto old = enabled_;
            d.deserialize("enabled", enabled_);
            if (old != enabled_) notifyEnabledChanged(this);
        }
        {
            auto old = priority_;
            d.deserialize("priority", priority_);
            if (old != priority_) notifyPriorityChanged(this);
        }


        using Elem = std::unique_ptr<KeyframeSequenceTyped<Key>>;
        util::IndexedDeserializer<Elem>("sequences", "sequence")
            .onNew([&](Elem& seq) { notifyKeyframeSequenceAdded(seq.get()); })
            .onRemove([&](Elem& seq) { notifyKeyframeSequenceRemoved(seq.get()); })(d, sequences_);

        if (Property* ptr = property_) {
            d.deserialize("property", ptr);
        } else {
            d.deserialize("property", ptr);
            property_ = dynamic_cast<Prop*>(ptr);
        }
    };

private:
    virtual void onKeyframeSequenceMoved(KeyframeSequence* key) override {
        std::stable_sort(sequences_.begin(), sequences_.end(), [](const auto& a, const auto& b) {
            return a->getFirst().getTime() < b->getFirst().getTime();
        });
        /// Do validation? 
    }

    Prop* property_;   ///< non-owning reference
    bool enabled_{true};
    std::string identifier_;
    std::string name_;
    size_t priority_{100};

    // Sorted list of non-overlapping sequences of key frames
    std::vector<std::unique_ptr<KeyframeSequenceTyped<Key>>> sequences_;
};



} // namespace

} // namespace

#endif // IVW_TRACK_H

