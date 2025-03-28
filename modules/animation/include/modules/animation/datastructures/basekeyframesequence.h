/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <inviwo/core/io/serialization/deserializer.h>          // for Deserializer, IndexedDese...
#include <inviwo/core/io/serialization/serializer.h>            // for Serializer
#include <inviwo/core/util/exception.h>                         // for Exception
#include <inviwo/core/util/indirectiterator.h>                  // for makeIndirectIterator, Ind...
#include <inviwo/core/util/stdextensions.h>                     // for dynamic_unique_ptr_cast
#include <modules/animation/datastructures/animationtime.h>     // for Seconds
#include <modules/animation/datastructures/keyframeobserver.h>  // for KeyframeObserver
#include <modules/animation/datastructures/keyframesequence.h>  // for KeyframeSequence

#include <algorithm>    // for min, find_if, stable_sort
#include <cstddef>      // for size_t
#include <memory>       // for unique_ptr, make_unique
#include <string_view>  // for string_view
#include <type_traits>  // for is_base_of
#include <utility>      // for move
#include <vector>       // for vector

namespace inviwo {

namespace animation {
class Keyframe;

template <typename Key>
class BaseKeyframeSequence : public KeyframeSequence, public KeyframeObserver {
public:
    using key_type = Key;
    using value_type = typename Key::value_type;
    using iterator = util::IndirectIterator<typename std::vector<std::unique_ptr<Key>>::iterator>;
    using const_iterator =
        util::IndirectIterator<typename std::vector<std::unique_ptr<Key>>::const_iterator>;

    static_assert(std::is_base_of<Keyframe, Key>::value, "Key has to derive from Keyframe");

    BaseKeyframeSequence();
    BaseKeyframeSequence(std::vector<std::unique_ptr<Key>> keyframes);

    /**
     * Remove all keyframes and call KeyframeObserver::notifyKeyframeRemoved
     */
    virtual ~BaseKeyframeSequence();

    /**
     * Return number of keyframes in the sequence.
     */
    virtual size_t size() const override { return keyframes_.size(); }

    virtual const Key& operator[](size_t i) const override;
    virtual Key& operator[](size_t i) override;

    virtual const Key& getFirst() const override;
    virtual Key& getFirst() override;
    virtual const Key& getLast() const override;
    virtual Key& getLast() override;

    virtual std::optional<Seconds> getPrevTime(Seconds at) const final;
    virtual std::optional<Seconds> getNextTime(Seconds at) const final;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    /**
     * Add Keyframe and call KeyframeObserver::notifyKeyframeAdded
     * @throw Exception if Keyframe is not compatible with BaseKeyframeSequence<Key>
     */
    virtual Key* add(std::unique_ptr<Keyframe> key) override;
    /**
     * Add Keyframe and call KeyframeObserver::notifyKeyframeAdded
     */
    Key* add(std::unique_ptr<Key> key);

    /**
     * Remove Keyframe and call KeyframeObserver::notifyKeyframeRemoved
     */
    virtual std::unique_ptr<Keyframe> remove(size_t i) override;
    virtual std::unique_ptr<Keyframe> remove(Keyframe* key) override;

    virtual bool isSelected() const override;
    virtual void setSelected(bool selected) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    BaseKeyframeSequence(const BaseKeyframeSequence& rhs);
    BaseKeyframeSequence& operator=(const BaseKeyframeSequence& that);
    virtual void onKeyframeTimeChanged(Keyframe* key, Seconds oldTime) override;

    std::vector<std::unique_ptr<Key>> keyframes_;
    bool isSelected_{false};
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template <typename Key>
BaseKeyframeSequence<Key>::BaseKeyframeSequence() : KeyframeSequence(), keyframes_{} {}

template <typename Key>
BaseKeyframeSequence<Key>::BaseKeyframeSequence(std::vector<std::unique_ptr<Key>> keyframes)
    : KeyframeSequence(), keyframes_{std::move(keyframes)} {
    for (auto& key : keyframes_) {
        key->addObserver(this);
    }
}

template <typename Key>
BaseKeyframeSequence<Key>::BaseKeyframeSequence(const BaseKeyframeSequence<Key>& rhs)
    : KeyframeSequence(rhs) {
    for (const auto& key : rhs.keyframes_) {
        add(std::make_unique<Key>(*key));
    }
}

template <typename Key>
BaseKeyframeSequence<Key>& BaseKeyframeSequence<Key>::operator=(
    const BaseKeyframeSequence<Key>& that) {
    if (this != &that) {
        setSelected(that.isSelected_);
        for (size_t i = 0; i < std::min(keyframes_.size(), that.keyframes_.size()); i++) {
            *keyframes_[i] = *that.keyframes_[i];
        }
        for (size_t i = std::min(keyframes_.size(), that.keyframes_.size());
             i < that.keyframes_.size(); i++) {
            keyframes_.push_back(std::make_unique<Key>(*(that.keyframes_[i])));
            notifyKeyframeAdded(keyframes_.back().get(), this);
        }
        while (keyframes_.size() > that.keyframes_.size()) {
            auto key = std::move(keyframes_.back());
            keyframes_.pop_back();
            notifyKeyframeRemoved(key.get(), this);
        }
    }
    return *this;
}

template <typename Key>
BaseKeyframeSequence<Key>::~BaseKeyframeSequence() {
    while (size() > 0) {
        // Remove and notify that keyframe is removed.
        remove(size() - 1);
    }
}

template <typename Key>
void BaseKeyframeSequence<Key>::onKeyframeTimeChanged(Keyframe* key, Seconds /*oldTime*/) {
    std::stable_sort(keyframes_.begin(), keyframes_.end(),
                     [](const auto& a, const auto& b) { return *a < *b; });

    if (keyframes_.front().get() == key || keyframes_.back().get() == key) {
        notifyKeyframeSequenceMoved(this);
    }
}

template <typename Key>
Key* BaseKeyframeSequence<Key>::add(std::unique_ptr<Keyframe> key) {
    if (auto k = util::dynamic_unique_ptr_cast<Key>(std::move(key))) {
        return add(std::move(k));
    } else {
        throw Exception("Invalid key type");
    }
    return nullptr;
}

template <typename Key>
Key* BaseKeyframeSequence<Key>::add(std::unique_ptr<Key> key) {
    auto it =
        keyframes_.insert(std::upper_bound(keyframes_.begin(), keyframes_.end(), key,
                                           [](const auto& a, const auto& b) { return *a < *b; }),
                          std::move(key));

    (*it)->addObserver(this);
    notifyKeyframeAdded(it->get(), this);
    return it->get();
}

template <typename Key>
std::unique_ptr<Keyframe> BaseKeyframeSequence<Key>::remove(size_t i) {
    if (i < keyframes_.size()) {
        auto key = std::move(keyframes_[i]);
        keyframes_.erase(keyframes_.begin() + i);
        notifyKeyframeRemoved(key.get(), this);
        return key;
    } else {
        return nullptr;
    }
}

template <typename Key>
std::unique_ptr<Keyframe> BaseKeyframeSequence<Key>::remove(Keyframe* key) {
    auto it = std::find_if(keyframes_.begin(), keyframes_.end(),
                           [&](auto& elem) { return elem.get() == key; });
    if (it != keyframes_.end()) {
        auto res = std::move(*it);
        keyframes_.erase(it);
        notifyKeyframeRemoved(res.get(), this);
        return res;
    } else {
        return nullptr;
    }
}

template <typename Key>
Key& BaseKeyframeSequence<Key>::getLast() {
    return *keyframes_.back();
}

template <typename Key>
const Key& BaseKeyframeSequence<Key>::getLast() const {
    return *keyframes_.back();
}

template <typename Key>
Key& BaseKeyframeSequence<Key>::getFirst() {
    return *keyframes_.front();
}

template <typename Key>
const Key& animation::BaseKeyframeSequence<Key>::getFirst() const {
    return *keyframes_.front();
}
template <typename Key>
std::optional<Seconds> animation::BaseKeyframeSequence<Key>::getPrevTime(Seconds at) const {
    auto it = std::lower_bound(
        begin(), end(), at, [](const auto& key, const auto& time) { return key.getTime() < time; });

    if (it != begin()) {
        return std::prev(it)->getTime();
    } else {
        return std::nullopt;
    }
}

template <typename Key>
std::optional<Seconds> animation::BaseKeyframeSequence<Key>::getNextTime(Seconds at) const {
    auto it = std::upper_bound(
        begin(), end(), at, [](const auto& time, const auto& key) { return time < key.getTime(); });

    if (it != end()) {
        return it->getTime();
    } else {
        return std::nullopt;
    }
}

template <typename Key>
Key& BaseKeyframeSequence<Key>::operator[](size_t i) {
    return *keyframes_[i];
}

template <typename Key>
const Key& BaseKeyframeSequence<Key>::operator[](size_t i) const {
    return *keyframes_[i];
}

template <typename Key>
auto BaseKeyframeSequence<Key>::begin() -> iterator {
    return util::makeIndirectIterator<true>(keyframes_.begin());
}

template <typename Key>
auto BaseKeyframeSequence<Key>::begin() const -> const_iterator {
    return util::makeIndirectIterator<true>(keyframes_.begin());
}

template <typename Key>
auto BaseKeyframeSequence<Key>::end() -> iterator {
    return util::makeIndirectIterator<true>(keyframes_.end());
}

template <typename Key>
auto BaseKeyframeSequence<Key>::end() const -> const_iterator {
    return util::makeIndirectIterator<true>(keyframes_.end());
}

template <typename Key>
bool BaseKeyframeSequence<Key>::isSelected() const {
    return isSelected_;
}

template <typename Key>
void BaseKeyframeSequence<Key>::setSelected(bool selected) {
    if (selected != isSelected_) {
        isSelected_ = selected;
        notifyKeyframeSequenceSelectionChanged(this);
    }
}

template <typename Key>
void BaseKeyframeSequence<Key>::serialize(Serializer& s) const {
    s.serialize("selected", isSelected_);
    s.serialize("keyframes", keyframes_, "keyframe");
}

template <typename Key>
void BaseKeyframeSequence<Key>::deserialize(Deserializer& d) {
    {
        bool isSelected = isSelected_;
        d.deserialize("selected", isSelected);
        setSelected(isSelected);
    }

    d.deserialize("keyframes", keyframes_, "keyframe",
                  deserializer::IndexFunctions{.makeNew = []() { return std::unique_ptr<Key>{}; },
                                               .onNew =
                                                   [&](std::unique_ptr<Key>& key, size_t) {
                                                       notifyKeyframeAdded(key.get(), this);
                                                       key->addObserver(this);
                                                   },
                                               .onRemove =
                                                   [&](std::unique_ptr<Key>& key) {
                                                       notifyKeyframeRemoved(key.get(), this);
                                                   }});
}

#endif

}  // namespace animation

}  // namespace inviwo
