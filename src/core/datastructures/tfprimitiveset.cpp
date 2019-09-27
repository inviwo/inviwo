/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <algorithm>
#include <set>

namespace inviwo {

void TFPrimitiveSetObserver::onTFPrimitiveAdded(TFPrimitive&) {}

void TFPrimitiveSetObserver::onTFPrimitiveRemoved(TFPrimitive&) {}

void TFPrimitiveSetObserver::onTFPrimitiveChanged(const TFPrimitive&) {}

void TFPrimitiveSetObserver::onTFTypeChanged(const TFPrimitiveSet&) {}

void TFPrimitiveSetObservable::notifyTFPrimitiveAdded(TFPrimitive& p) {
    forEachObserver([&](TFPrimitiveSetObserver* o) { o->onTFPrimitiveAdded(p); });
}

void TFPrimitiveSetObservable::notifyTFPrimitiveRemoved(TFPrimitive& p) {
    forEachObserver([&](TFPrimitiveSetObserver* o) { o->onTFPrimitiveRemoved(p); });
}

void TFPrimitiveSetObservable::notifyTFPrimitiveChanged(const TFPrimitive& p) {
    forEachObserver([&](TFPrimitiveSetObserver* o) { o->onTFPrimitiveChanged(p); });
}

void TFPrimitiveSetObservable::notifyTFTypeChanged(const TFPrimitiveSet& primitiveSet) {
    forEachObserver([&](TFPrimitiveSetObserver* o) { o->onTFTypeChanged(primitiveSet); });
}

TFPrimitiveSet::TFPrimitiveSet(const std::vector<TFPrimitiveData>& values, TFPrimitiveSetType type)
    : type_("type", type) {
    add(values);
}

TFPrimitiveSet::TFPrimitiveSet(const TFPrimitiveSet& rhs) : type_(rhs.type_) {
    for (auto& v : rhs.values_) {
        add(*v);
    }
}

TFPrimitiveSet& TFPrimitiveSet::operator=(const TFPrimitiveSet& rhs) {
    if (this != &rhs) {
        type_ = rhs.type_;
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

void TFPrimitiveSet::set(const_iterator sbegin, const_iterator send) {
    auto dbegin = values_.begin();
    auto dend = values_.end();

    while (dbegin != dend && sbegin != send) {
        **dbegin++ = *sbegin++;
    }
    while (sbegin != send) {
        add(*sbegin++);
    }
    while (dbegin != dend) {
        remove(--dend);
    }
    invalidate();
}

void TFPrimitiveSet::setType(TFPrimitiveSetType type) {
    if (type_ == type) {
        return;
    }
    type_ = type;
    notifyTFTypeChanged(*this);
}

dvec2 TFPrimitiveSet::getRange() const {
    switch (type_) {
        case TFPrimitiveSetType::Absolute: {
            if (sorted_.empty()) {
                return dvec2(0.0, 1.0);
            }
            return dvec2(sorted_.front()->getPosition(), sorted_.back()->getPosition());
        }
        case TFPrimitiveSetType::Relative:
        default:
            return dvec2(0.0, 1.0);
    }
}

size_t TFPrimitiveSet::size() const { return values_.size(); }

bool TFPrimitiveSet::empty() const { return values_.empty(); }

TFPrimitive& TFPrimitiveSet::operator[](size_t i) { return *sorted_[i]; }

const TFPrimitive& TFPrimitiveSet::operator[](size_t i) const { return *sorted_[i]; }

auto TFPrimitiveSet::begin() -> iterator {
    return iterator{[](TFPrimitive* p) -> TFPrimitive& { return *p; }, sorted_.begin()};
}

auto TFPrimitiveSet::end() -> iterator {
    return iterator{[](TFPrimitive* p) -> TFPrimitive& { return *p; }, sorted_.end()};
}

auto TFPrimitiveSet::begin() const -> const_iterator {
    return const_iterator{[](const TFPrimitive* const p) -> const TFPrimitive& { return *p; },
                          sorted_.cbegin()};
}

auto TFPrimitiveSet::end() const -> const_iterator {
    return const_iterator{[](const TFPrimitive* const p) -> const TFPrimitive& { return *p; },
                          sorted_.cend()};
}

auto TFPrimitiveSet::cbegin() const -> const_iterator {
    return const_iterator{[](const TFPrimitive* const p) -> const TFPrimitive& { return *p; },
                          sorted_.cbegin()};
}

auto TFPrimitiveSet::cend() const -> const_iterator {
    return const_iterator{[](const TFPrimitive* const p) -> const TFPrimitive& { return *p; },
                          sorted_.cend()};
}

TFPrimitive& TFPrimitiveSet::get(size_t i) { return *sorted_[i]; }

const TFPrimitive& TFPrimitiveSet::get(size_t i) const { return *sorted_[i]; }

TFPrimitive& TFPrimitiveSet::front() { return *sorted_.front(); }

const TFPrimitive& TFPrimitiveSet::front() const { return *sorted_.front(); }

TFPrimitive& TFPrimitiveSet::back() { return *sorted_.back(); }

const TFPrimitive& TFPrimitiveSet::back() const { return *sorted_.back(); }

std::vector<TFPrimitiveData> TFPrimitiveSet::get() const {
    std::vector<TFPrimitiveData> values;
    std::transform(sorted_.begin(), sorted_.end(), std::back_inserter(values),
                   [](auto& v) { return v->getData(); });
    return values;
}

std::vector<TFPrimitiveData> TFPrimitiveSet::getUnsorted() const {
    std::vector<TFPrimitiveData> values;
    std::transform(values_.begin(), values_.end(), std::back_inserter(values),
                   [](auto& v) { return v->getData(); });
    return values;
}

std::pair<std::vector<double>, std::vector<vec4>> TFPrimitiveSet::getVectors() const {
    std::pair<std::vector<double>, std::vector<vec4>> result;
    result.first.reserve(sorted_.size());
    result.second.reserve(sorted_.size());
    for (auto& v : sorted_) {
        result.first.push_back(static_cast<float>(v->getPosition()));
        result.second.push_back(v->getColor());
    }

    return result;
}

std::pair<std::vector<float>, std::vector<vec4>> TFPrimitiveSet::getVectorsf() const {
    std::pair<std::vector<float>, std::vector<vec4>> result;
    result.first.reserve(sorted_.size());
    result.second.reserve(sorted_.size());
    for (auto& v : sorted_) {
        result.first.push_back(static_cast<float>(v->getPosition()));
        result.second.push_back(v->getColor());
    }

    return result;
}

void TFPrimitiveSet::add(const TFPrimitive& primitive) {
    add(std::make_unique<TFPrimitive>(primitive));
}

void TFPrimitiveSet::add(double pos, const vec4& color) {
    add(std::make_unique<TFPrimitive>(pos, color));
}

void TFPrimitiveSet::add(const dvec2& pos) {
    const vec4 color(vec3(interpolateColor(pos.x)), static_cast<float>(pos.y));
    add(std::make_unique<TFPrimitive>(pos.x, color));
}

void TFPrimitiveSet::add(const TFPrimitiveData& data) { add(std::make_unique<TFPrimitive>(data)); }

void TFPrimitiveSet::add(const std::vector<TFPrimitiveData>& primitives) {
    for (auto& v : primitives) {
        add(v);
    }
}

bool TFPrimitiveSet::remove(const TFPrimitive& primitive) {
    auto it = std::find_if(values_.begin(), values_.end(),
                           [&](const auto& v) { return &primitive == v.get(); });

    return remove(it);
}

void TFPrimitiveSet::add(std::unique_ptr<TFPrimitive> primitive) {
    if ((type_ == TFPrimitiveSetType::Relative) &&
        ((primitive->getPosition() < 0.0f) || (primitive->getPosition() > 1.0f))) {
        throw RangeException("Adding TFPrimitive at " + std::to_string(primitive->getPosition()) +
                                 " outside of range [0,1]",
                             IVW_CONTEXT);
    }

    primitive->addObserver(this);
    auto it = std::upper_bound(sorted_.begin(), sorted_.end(), primitive.get(), comparePtr{});
    sorted_.insert(it, primitive.get());
    values_.push_back(std::move(primitive));

    invalidate();
    notifyTFPrimitiveAdded(*values_.back());
}

bool TFPrimitiveSet::remove(std::vector<std::unique_ptr<TFPrimitive>>::iterator it) {
    if (it != values_.end()) {
        // make sure we call the destructor after we have removed the point from points_
        auto dp = std::move(*it);
        values_.erase(it);
        util::erase_remove(sorted_, dp.get());
        invalidate();
        notifyTFPrimitiveRemoved(*dp);
        return true;
    } else {
        return false;
    }
}

void TFPrimitiveSet::clear() {
    while (values_.size() > 0) {
        remove(--values_.end());
    }
}

void TFPrimitiveSet::setPosition(const std::vector<TFPrimitive*> primitives, double pos) {
    // selected primitives need to be moved in correct order to maintain overall order of TF
    // That is, TF primitives closest to pos must be moved first
    std::set<TFPrimitive*> primitiveSet(primitives.begin(), primitives.end());

    std::vector<TFPrimitive*> sortedSelection;
    std::copy_if(sorted_.begin(), sorted_.end(), std::back_inserter(sortedSelection),
                 [&primitiveSet](auto item) { return primitiveSet.count(item) > 0; });

    // partition set of primitives at position pos
    auto partition =
        std::lower_bound(sortedSelection.begin(), sortedSelection.end(), pos,
                         [](const auto& p, double val) { return p->getPosition() < val; });

    // update upper half, i.e. all elements to the right of pos in ascending order
    for (auto it = partition; it != sortedSelection.end(); ++it) {
        (*it)->setPosition(pos);
    }

    // update lower half, i.e. all elements to the left of pos in descending order
    // to do this reverse sorted primitives from begin to the partition point
    std::reverse(sortedSelection.begin(), partition);
    for (auto it = sortedSelection.begin(); it != partition; ++it) {
        (*it)->setPosition(pos);
    }
}

void TFPrimitiveSet::setAlpha(const std::vector<TFPrimitive*> primitives, double alpha) {
    for (auto p : primitives) {
        if (util::contains(sorted_, p)) {
            p->setAlpha(static_cast<float>(alpha));
        }
    }
}

void TFPrimitiveSet::setColor(const std::vector<TFPrimitive*> primitives, const vec3& color) {
    for (auto p : primitives) {
        if (util::contains(sorted_, p)) {
            p->setColor(color);
        }
    }
}

void TFPrimitiveSet::onTFPrimitiveChange(const TFPrimitive& p) {
    sort();
    invalidate();
    notifyTFPrimitiveChanged(p);
}

void TFPrimitiveSet::serialize(Serializer& s) const {
    type_.serialize(s, PropertySerializationMode::All);
    s.serialize(serializationKey(), values_, serializationItemKey());
}

void TFPrimitiveSet::deserialize(Deserializer& d) {
    if (type_.deserialize(d, PropertySerializationMode::All)) {
        notifyTFTypeChanged(*this);
    }

    util::IndexedDeserializer<std::unique_ptr<TFPrimitive>>(serializationKey(),
                                                            serializationItemKey())
        .onNew([&](std::unique_ptr<TFPrimitive>& p) {
            p->addObserver(this);
            auto it = std::upper_bound(sorted_.begin(), sorted_.end(), p.get(), comparePtr{});
            sorted_.insert(it, p.get());
            notifyTFPrimitiveAdded(*p);
        })
        .onRemove([&](std::unique_ptr<TFPrimitive>& p) {
            util::erase_remove(sorted_, p.get());
            notifyTFPrimitiveRemoved(*p);
        })(d, values_);
    invalidate();
}

std::vector<FileExtension> TFPrimitiveSet::getSupportedExtensions() const { return {}; }
void TFPrimitiveSet::save(const std::string&, const FileExtension&) const {}
void TFPrimitiveSet::load(const std::string&, const FileExtension&) {}

void TFPrimitiveSet::sort() { std::stable_sort(sorted_.begin(), sorted_.end(), comparePtr{}); }

vec4 TFPrimitiveSet::interpolateColor(double t) const {
    if (empty()) return vec4(0.0f);

    auto it = std::upper_bound(begin(), end(), t,
                               [](double val, const auto& p) { return val < p.getPosition(); });

    if (it == begin()) {
        return front().getColor();
    } else if (it == end()) {
        return back().getColor();
    }

    auto next = it--;
    const auto x =
        static_cast<float>((t - it->getPosition()) / (next->getPosition() - it->getPosition()));
    return glm::mix(it->getColor(), next->getColor(), x);
}

void TFPrimitiveSet::interpolateAndStoreColors(vec4* dataArray, const size_t dataSize) const {
    const auto toInd = [&](const TFPrimitive& p) {
        return static_cast<size_t>(ceil(p.getPosition() * (dataSize - 1)));
    };

    if (empty()) {  // in case of 0 points
        std::fill(dataArray, dataArray + dataSize, vec4(0.0f));
    } else if (size() == 1) {  // in case of 1 point
        std::fill(dataArray, dataArray + dataSize, front().getColor());
    } else {  // in case of more than 1 points
        const size_t leftX = toInd(front());
        const size_t rightX = toInd(back());

        std::fill(dataArray, dataArray + leftX + 1, front().getColor());
        std::fill(dataArray + rightX, dataArray + dataSize, back().getColor());

        auto pLeft = begin();
        auto pRight = ++begin();

        while (pRight != end()) {
            const auto lrgba = pLeft->getColor();
            const auto rrgba = pRight->getColor();
            const auto lx = pLeft->getPosition() * (dataSize - 1);
            const auto rx = pRight->getPosition() * (dataSize - 1);

            for (size_t n = toInd(*pLeft); n < toInd(*pRight); ++n) {
                const float x = static_cast<float>((n - lx) / (rx - lx));
                dataArray[n] = glm::mix(lrgba, rrgba, x);
            }
            ++pLeft;
            ++pRight;
        }
    }
}

void TFPrimitiveSet::flipPositions(const std::vector<TFPrimitive*>& primitives) {
    dvec2 range{};
    std::vector<TFPrimitive*> selection;

    if (primitives.empty()) {
        selection = sorted_;
        range = getRange();
    } else {
        selection = util::copy_if(primitives, [&](auto p) { return util::contains(sorted_, p); });
        if (!selection.empty()) {
            auto minmax = std::minmax_element(
                selection.begin(), selection.end(),
                [](const TFPrimitive* a, const TFPrimitive* b) { return *a < *b; });
            range.x = (*minmax.first)->getPosition();
            range.y = (*minmax.second)->getPosition();
        }
    }

    if (selection.size() < 2) {
        return;
    }

    for (auto& elem : selection) {
        elem->setPosition(range.y - (elem->getPosition() - range.x));
    }
}

void TFPrimitiveSet::interpolateAlpha(const std::vector<TFPrimitive*>& primitives) {
    dvec2 range{};
    vec2 alpha{0.0f, 1.0f};
    std::vector<TFPrimitive*> selection;

    if (primitives.empty()) {
        selection = sorted_;
        range = getRange();
        if (!sorted_.empty()) {
            alpha.x = sorted_.front()->getAlpha();
            alpha.y = sorted_.back()->getAlpha();
        }
    } else {
        selection = util::copy_if(primitives, [&](auto p) { return util::contains(sorted_, p); });
        if (!selection.empty()) {
            auto minmax = std::minmax_element(
                selection.begin(), selection.end(),
                [](const TFPrimitive* a, const TFPrimitive* b) { return *a < *b; });

            range.x = (*minmax.first)->getPosition();
            range.y = (*minmax.second)->getPosition();
            alpha.x = (*minmax.first)->getColor().a;
            alpha.y = (*minmax.second)->getColor().a;
        }
    }

    if (selection.size() < 2) {
        return;
    }

    for (auto& elem : selection) {
        const float t = static_cast<float>((elem->getPosition() - range.x) / (range.y - range.x));
        elem->setAlpha(glm::mix(alpha.x, alpha.y, t));
    }
}

void TFPrimitiveSet::equalizeAlpha(const std::vector<TFPrimitive*>& primitives) {
    std::vector<TFPrimitive*> selection =
        (primitives.empty() ? sorted_ : util::copy_if(primitives, [&](auto p) {
            return util::contains(sorted_, p);
        }));

    if (selection.size() < 2) {
        return;
    }

    float alpha =
        std::accumulate(selection.begin(), selection.end(), 0.0f,
                        [](const float sum, TFPrimitive* p) { return sum + p->getAlpha(); });
    alpha /= selection.size();

    for (auto& elem : selection) {
        elem->setAlpha(alpha);
    }
}

std::string TFPrimitiveSet::getTitle() const { return "TFPrimitiveSet"; }

std::string TFPrimitiveSet::serializationKey() const { return "TFPrimitives"; }

std::string TFPrimitiveSet::serializationItemKey() const { return "TFPrimitive"; }

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
