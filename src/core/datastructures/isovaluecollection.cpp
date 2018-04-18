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

#include <inviwo/core/datastructures/isovaluecollection.h>

#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/util/interpolation.h>
#include <inviwo/core/io/datareaderexception.h>

namespace inviwo {

void IsoValueCollectionObserver::onIsoValueAdded(IsoValue*) {}

void IsoValueCollectionObserver::onIsoValueRemoved(IsoValue*) {}

void IsoValueCollectionObserver::onIsoValueChanged(const IsoValue*) {}

void IsoValueCollectionObservable::notifyIsoValueAdded(IsoValue* v) {
    forEachObserver([&](IsoValueCollectionObserver* o) { o->onIsoValueAdded(v); });
}

void IsoValueCollectionObservable::notifyIsoValueRemoved(IsoValue* v) {
    forEachObserver([&](IsoValueCollectionObserver* o) { o->onIsoValueRemoved(v); });
}

void IsoValueCollectionObservable::notifyIsoValueChanged(const IsoValue* v) {
    forEachObserver([&](IsoValueCollectionObserver* o) { o->onIsoValueChanged(v); });
}

IsoValueCollection::IsoValueCollection(const std::vector<IsoValueData>& values) {
    addIsoValues(values);
}

IsoValueCollection::IsoValueCollection(const IsoValueCollection& rhs) {
    for (auto& v : rhs.values_) {
        addIsoValue(*v);
    }
}

IsoValueCollection::IsoValueCollection(IsoValueCollection&& rhs)
    : values_(std::move(rhs.values_)), sorted_(std::move(rhs.sorted_)) {}

IsoValueCollection& IsoValueCollection::operator=(const IsoValueCollection& rhs) {
    if (this != &rhs) {
        for (size_t i = 0; i < std::min(values_.size(), rhs.values_.size()); i++) {
            *values_[i] = *rhs.values_[i];
        }
        for (size_t i = std::min(values_.size(), rhs.values_.size()); i < rhs.values_.size(); i++) {
            addIsoValue(*rhs.values_[i]);
        }
        while (values_.size() > rhs.values_.size()) {
            removeIsoValue(--values_.end());
        }
    }
    return *this;
}

size_t IsoValueCollection::getNumIsoValues() const { return values_.size(); }

IsoValue& IsoValueCollection::getIsoValue(size_t i) { return *values_[i].get(); }

const IsoValue& IsoValueCollection::getIsoValue(size_t i) const { return *values_[i].get(); }

std::vector<IsoValueData> IsoValueCollection::getIsoValues() const {
    std::vector<IsoValueData> values;
    std::transform(values_.begin(), values_.end(), std::back_inserter(values),
                   [](auto& v) { return v->get(); });
    return values;
}

std::vector<IsoValueData> IsoValueCollection::getSortedIsoValues() const {
    std::vector<IsoValueData> values;
    std::transform(sorted_.begin(), sorted_.end(), std::back_inserter(values),
                   [](auto& v) { return v->get(); });
    return values;
}

std::pair<std::vector<float>, std::vector<vec4>> IsoValueCollection::getShaderUniformData() const {
    std::pair<std::vector<float>, std::vector<vec4>> result;
    result.first.reserve(sorted_.size());
    result.second.reserve(sorted_.size());
    for (auto& v : sorted_) {
        result.first.push_back(v->getIsoValue());
        result.second.push_back(v->getColor());
    }

    return result;
}

void IsoValueCollection::addIsoValue(const float value, const vec4& color) {
    addIsoValue(util::make_unique<IsoValue>(value, color));
}

void IsoValueCollection::addIsoValue(const IsoValue& value) {
    addIsoValue(util::make_unique<IsoValue>(value));
}

void IsoValueCollection::addIsoValue(const vec2& pos) {
    const vec4 color(vec3(interpolateColor(pos.x)), pos.y);
    addIsoValue(util::make_unique<IsoValue>(pos.x, color));
}

void IsoValueCollection::addIsoValue(const IsoValueData& value) {
    addIsoValue(value.isovalue, value.color);
}

void IsoValueCollection::addIsoValues(const std::vector<IsoValueData>& values) {
    for (auto& v : values) {
        addIsoValue(v);
    }
}

void IsoValueCollection::removeIsoValue(IsoValue* value) {
    auto it = std::find_if(values_.begin(), values_.end(),
                           [&](const auto& v) { return value == v.get(); });

    removeIsoValue(it);
}

void IsoValueCollection::addIsoValue(std::unique_ptr<IsoValue> value) {
    if ((value->getIsoValue() < 0.0f) || (value->getIsoValue() > 1.0f)) {
        throw RangeException("Adding isovalue at " + std::to_string(value->getIsoValue()) +
                                 " outside of range [0,1]",
                             IvwContext);
    }

    value->addObserver(this);
    auto it = std::upper_bound(sorted_.begin(), sorted_.end(), value.get(), comparePtr{});
    sorted_.insert(it, value.get());
    values_.push_back(std::move(value));

    notifyIsoValueAdded(values_.back().get());
}

void IsoValueCollection::removeIsoValue(std::vector<std::unique_ptr<IsoValue>>::iterator it) {
    if (it != values_.end()) {
        // make sure we call the destructor after we have removed the point from points_
        auto dp = std::move(*it);
        values_.erase(it);
        util::erase_remove(sorted_, dp.get());
        notifyIsoValueRemoved(dp.get());
    }
}

void IsoValueCollection::clearIsoValues() {
    while (values_.size() > 0) {
        removeIsoValue(--values_.end());
    }
}

void IsoValueCollection::onIsoValueChange(const IsoValue* v) {
    sort();
    notifyIsoValueChanged(v);
}

void IsoValueCollection::serialize(Serializer& s) const {
    s.serialize("IsoValues", values_, "IsoValue");
}

void IsoValueCollection::deserialize(Deserializer& d) {
    util::IndexedDeserializer<std::unique_ptr<IsoValue>>("IsoValues", "IsoValue")
        .onNew([&](std::unique_ptr<IsoValue>& v) {
            v->addObserver(this);
            auto it = std::upper_bound(sorted_.begin(), sorted_.end(), v.get(), comparePtr{});
            sorted_.insert(it, v.get());
            notifyIsoValueAdded(v.get());
        })
        .onRemove([&](std::unique_ptr<IsoValue>& v) {
            util::erase_remove(sorted_, v.get());
            notifyIsoValueRemoved(v.get());
        })(d, values_);
}

void IsoValueCollection::save(const std::string& filename, const FileExtension& ext) const {
    std::string extension = toLower(filesystem::getFileExtension(filename));

    if (ext.extension_ == "iiv" || (ext.empty() && extension == "iiv")) {
        Serializer serializer(filename);
        serialize(serializer);
        serializer.writeFile();
    } else {
        throw DataWriterException("Unsupported format for saving isovalues", IvwContext);
    }
}

void IsoValueCollection::load(const std::string& filename, const FileExtension& ext) {
    std::string extension = toLower(filesystem::getFileExtension(filename));

    if (ext.extension_ == "iiv" || (ext.empty() && extension == "iiv")) {
        Deserializer deserializer(filename);
        deserialize(deserializer);
    } else {
        throw DataReaderException("Unsupported format for loading isovalues", IvwContext);
    }
}

void IsoValueCollection::sort() { std::stable_sort(sorted_.begin(), sorted_.end(), comparePtr{}); }

vec4 IsoValueCollection::interpolateColor(float v) const {
    if (sorted_.empty()) return vec4(1.0f);

    if (v <= 0.0f) {
        return sorted_.front()->getColor();
    } else if (v >= 1.0f) {
        return sorted_.back()->getColor();
    }

    auto it = std::upper_bound(sorted_.begin(), sorted_.end(), v,
                               [](float val, const auto& p) { return val < p->getIsoValue(); });

    if (it == sorted_.begin()) {
        return sorted_.front()->getColor();
    } else if (it == sorted_.end()) {
        return sorted_.back()->getColor();
    }

    auto next = it--;
    float x = (v - (*it)->getIsoValue()) / ((*next)->getIsoValue() - (*it)->getIsoValue());
    return Interpolation<vec4, float>::linear((*it)->getColor(), (*next)->getColor(), x);
}

bool operator==(const IsoValueCollection& lhs, const IsoValueCollection& rhs) {
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

bool operator!=(const IsoValueCollection& lhs, const IsoValueCollection& rhs) {
    return !operator==(lhs, rhs);
}

}  // namespace inviwo
