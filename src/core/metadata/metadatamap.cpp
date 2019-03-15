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

#include <inviwo/core/metadata/metadatamap.h>
#include <inviwo/core/io/serialization/serializable.h>

namespace inviwo {

MetaDataMap::MetaDataMap(const MetaDataMap& inMap) {
    for (const auto& elem : inMap.metaData_) {
        if (elem.second) {
            metaData_[elem.first] = std::unique_ptr<MetaData>(elem.second->clone());
        }
    }
}

MetaData* MetaDataMap::add(const std::string& key, MetaData* metaData) {
    metaData_[key] = std::unique_ptr<MetaData>(metaData);
    return metaData;
}

void MetaDataMap::remove(const std::string& key) { metaData_.erase(key); }

void MetaDataMap::removeAll() { metaData_.clear(); }

void MetaDataMap::rename(const std::string& newKey, const std::string& oldKey) {
    auto it = metaData_.find(oldKey);
    if (it != metaData_.end()) {
        metaData_[newKey] = std::move(it->second);
        metaData_.erase(oldKey);
    }
}

std::vector<std::string> MetaDataMap::getKeys() const {
    std::vector<std::string> keys;
    keys.reserve(metaData_.size());
    for (const auto& elem : metaData_) keys.emplace_back(elem.first);
    return keys;
}

MetaData* MetaDataMap::get(const std::string& key) {
    auto it = metaData_.find(key);
    if (it != metaData_.end()) return it->second.get();
    return nullptr;
}

const MetaData* MetaDataMap::get(const std::string& key) const {
    auto it = metaData_.find(key);
    if (it != metaData_.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool MetaDataMap::empty() const { return metaData_.empty(); }

MetaDataMap& MetaDataMap::operator=(const MetaDataMap& map) {
    if (this != &map) {
        removeAll();

        for (const auto& elem : map.metaData_) {
            metaData_[elem.first] = std::unique_ptr<MetaData>(elem.second->clone());
        }
    }
    return *this;
}

void MetaDataMap::serialize(Serializer& s) const {
    if (!metaData_.empty()) s.serialize("MetaDataMap", metaData_, "MetaDataItem");
}

void MetaDataMap::deserialize(Deserializer& d) {
    d.deserialize("MetaDataMap", metaData_, "MetaDataItem");
}

bool operator==(const MetaDataMap& lhs, const MetaDataMap& rhs) {

    if (lhs.metaData_.size() != rhs.metaData_.size()) {
        return false;
    }
    for (const auto& _cIt : lhs.metaData_) {
        auto elem = rhs.metaData_.find(_cIt.first);
        if (elem == rhs.metaData_.end()) return false;
        if (*(elem->second) != *(_cIt.second)) return false;
    }
    return true;
}

bool operator!=(const MetaDataMap& lhs, const MetaDataMap& rhs) { return !operator==(lhs, rhs); }

}  // namespace inviwo
