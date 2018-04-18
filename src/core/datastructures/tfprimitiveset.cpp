/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <inviwo/core/datastructures/tfprimitiveset.h>

#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/interpolation.h>

#include <algorithm>

namespace inviwo {

void TFPrimitiveSetObserver::onTFPrimitiveAdded(TFPrimitive*) {}

void TFPrimitiveSetObserver::onTFPrimitiveRemoved(TFPrimitive*) {}

void TFPrimitiveSetObserver::onTFPrimitiveChanged(const TFPrimitive*) {}

void TFPrimitiveSetObservable::notifyTFPrimitiveAdded(TFPrimitive* p) {
    forEachObserver([&](TFPrimitiveSetObserver* o) { o->onTFPrimitiveAdded(p); });
}

void TFPrimitiveSetObservable::notifyTFPrimitiveRemoved(TFPrimitive* p) {
    forEachObserver([&](TFPrimitiveSetObserver* o) { o->onTFPrimitiveRemoved(p); });
}

void TFPrimitiveSetObservable::notifyTFPrimitiveChanged(const TFPrimitive* p) {
    forEachObserver([&](TFPrimitiveSetObserver* o) { o->onTFPrimitiveChanged(p); });
}

TFPrimitiveSet::TFPrimitiveSet(const std::vector<TFPrimitiveData>& values, TFPrimitiveSetType type)
    : type_(type) {
    add(values);
}

TFPrimitiveSet::TFPrimitiveSet(const TFPrimitiveSet& rhs)
    : type_(rhs.type_)
    , serializationKey_(rhs.serializationKey_)
    , serializationItemKey_(rhs.serializationItemKey_) {
    for (auto& v : rhs.values_) {
        add(*v);
    }
}

TFPrimitiveSet::TFPrimitiveSet(TFPrimitiveSet&& rhs)
    : values_(std::move(rhs.values_))
    , sorted_(std::move(rhs.sorted_))
    , type_(rhs.type_)
    , serializationKey_(std::move(rhs.serializationKey_))
    , serializationItemKey_(std::move(rhs.serializationItemKey_)) {}

TFPrimitiveSet& TFPrimitiveSet::operator=(const TFPrimitiveSet& rhs) {
    if (this != &rhs) {
        serializationKey_ = rhs.serializationKey_;
        serializationItemKey_ = rhs.serializationItemKey_;

        for (size_t i = 0; i < std::min(values_.size(), rhs.values_.size()); i++) {
            *values_[i] = *rhs.values_[i];
        }
        for (size_t i = std::min(values_.size(), rhs.values_.size()); i < rhs.values_.size(); i++) {
            add(*rhs.values_[i]);
        }
        while (values_.size() > rhs.values_.size()) {
            remove(--values_.end());
        }
        invalidate();
    }
    return *this;
}

size_t TFPrimitiveSet::size() const { return values_.size(); }

TFPrimitive* TFPrimitiveSet::operator[](size_t i) { return values_[i].get(); }

const TFPrimitive* TFPrimitiveSet::operator[](size_t i) const { return values_[i].get(); }

TFPrimitive* TFPrimitiveSet::get(size_t i) { return values_[i].get(); }

const TFPrimitive* TFPrimitiveSet::get(size_t i) const { return values_[i].get(); }

std::vector<TFPrimitiveData> TFPrimitiveSet::get() const {
    std::vector<TFPrimitiveData> values;
    std::transform(values_.begin(), values_.end(), std::back_inserter(values),
                   [](auto& v) { return v->getData(); });
    return values;
}

std::vector<TFPrimitiveData> TFPrimitiveSet::getSorted() const {
    std::vector<TFPrimitiveData> values;
    std::transform(sorted_.begin(), sorted_.end(), std::back_inserter(values),
                   [](auto& v) { return v->getData(); });
    return values;
}

std::pair<std::vector<float>, std::vector<vec4>> TFPrimitiveSet::getVectors() const {
    std::pair<std::vector<float>, std::vector<vec4>> result;
    result.first.reserve(sorted_.size());
    result.second.reserve(sorted_.size());
    for (auto& v : sorted_) {
        result.first.push_back(v->getPosition());
        result.second.push_back(v->getColor());
    }

    return result;
}

void TFPrimitiveSet::add(const TFPrimitive& primitive) {
    add(util::make_unique<TFPrimitive>(primitive));
}

void TFPrimitiveSet::add(const vec2& pos) {
    const vec4 color(vec3(interpolateColor(pos.x)), pos.y);
    add(util::make_unique<TFPrimitive>(pos.x, color));
}

void TFPrimitiveSet::add(const TFPrimitiveData& data) { add(util::make_unique<TFPrimitive>(data)); }

void TFPrimitiveSet::add(const std::vector<TFPrimitiveData>& primitives) {
    for (auto& v : primitives) {
        add(v);
    }
}

void TFPrimitiveSet::remove(TFPrimitive* primitive) {
    auto it = std::find_if(values_.begin(), values_.end(),
                           [&](const auto& v) { return primitive == v.get(); });

    remove(it);
}

void TFPrimitiveSet::add(std::unique_ptr<TFPrimitive> primitive) {
    if ((type_ == TFPrimitiveSetType::Relative) &&
        ((primitive->getPosition() < 0.0f) || (primitive->getPosition() > 1.0f))) {
        throw RangeException("Adding TFPrimitive at " + std::to_string(primitive->getPosition()) +
                                 " outside of range [0,1]",
                             IvwContext);
    }

    primitive->addObserver(this);
    auto it = std::upper_bound(sorted_.begin(), sorted_.end(), primitive.get(), comparePtr{});
    sorted_.insert(it, primitive.get());
    values_.push_back(std::move(primitive));

    invalidate();
    notifyTFPrimitiveAdded(values_.back().get());
}

void TFPrimitiveSet::remove(std::vector<std::unique_ptr<TFPrimitive>>::iterator it) {
    if (it != values_.end()) {
        // make sure we call the destructor after we have removed the point from points_
        auto dp = std::move(*it);
        values_.erase(it);
        util::erase_remove(sorted_, dp.get());
        invalidate();
        notifyTFPrimitiveRemoved(dp.get());
    }
}

void TFPrimitiveSet::clear() {
    while (values_.size() > 0) {
        remove(--values_.end());
    }
}

void TFPrimitiveSet::onTFPrimitiveChange(const TFPrimitive* p) {
    sort();
    invalidate();
    notifyTFPrimitiveChanged(p);
}

void TFPrimitiveSet::serialize(Serializer& s) const {
    s.serialize(serializationKey_, values_, serializationItemKey_);
}

void TFPrimitiveSet::deserialize(Deserializer& d) {
    util::IndexedDeserializer<std::unique_ptr<TFPrimitive>>(serializationKey_,
                                                            serializationItemKey_)
        .onNew([&](std::unique_ptr<TFPrimitive>& v) {
            v->addObserver(this);
            auto it = std::upper_bound(sorted_.begin(), sorted_.end(), v.get(), comparePtr{});
            sorted_.insert(it, v.get());
            notifyTFPrimitiveAdded(v.get());
        })
        .onRemove([&](std::unique_ptr<TFPrimitive>& v) {
            util::erase_remove(sorted_, v.get());
            notifyTFPrimitiveRemoved(v.get());
        })(d, values_);
    invalidate();
}

void TFPrimitiveSet::sort() { std::stable_sort(sorted_.begin(), sorted_.end(), comparePtr{}); }

vec4 TFPrimitiveSet::interpolateColor(float v) const {
    if (sorted_.empty()) return vec4(1.0f);

    if (v <= 0.0f) {
        return sorted_.front()->getColor();
    } else if (v >= 1.0f) {
        return sorted_.back()->getColor();
    }

    auto it = std::upper_bound(sorted_.begin(), sorted_.end(), v,
                               [](float val, const auto& p) { return val < p->getPosition(); });

    if (it == sorted_.begin()) {
        return sorted_.front()->getColor();
    } else if (it == sorted_.end()) {
        return sorted_.back()->getColor();
    }

    auto next = it--;
    float x = (v - (*it)->getPosition()) / ((*next)->getPosition() - (*it)->getPosition());
    return Interpolation<vec4, float>::linear((*it)->getColor(), (*next)->getColor(), x);
}

void TFPrimitiveSet::setSerializationKey(const std::string& key, const std::string& itemKey) {
    serializationKey_ = key;
    serializationItemKey_ = itemKey;
}

bool operator==(const TFPrimitiveSet& lhs, const TFPrimitiveSet& rhs) {
    if (lhs.sorted_.size() != rhs.sorted_.size()) {
        return false;
    }
    for (auto&& i : util::zip(lhs.sorted_, rhs.sorted_)) {
        if (get<0>(i) != get<1>(i)) {
            return false;
        }
    }
    return true;
}

bool operator!=(const TFPrimitiveSet& lhs, const TFPrimitiveSet& rhs) {
    return !operator==(lhs, rhs);
}

}  // namespace inviwo
