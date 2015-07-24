/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_INVIWOFACTORYBASE_H
#define IVW_INVIWOFACTORYBASE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <string>
#include <unordered_map>

namespace inviwo {

class IvwSerializable;

class IVW_CORE_API FactoryBase {
public:
    FactoryBase() {}
    virtual ~FactoryBase() {}
};

template <typename T, typename K = const std::string&, typename... Args>
class Factory : public FactoryBase {
public:
    Factory() = default;
    virtual ~Factory() = default;

    virtual T* create(K key, Args&&... args) const = 0;
    virtual bool hasKey(K key) const = 0;
};

template <typename T, typename M, typename K = const std::string&>
class StandardFactory : public Factory<T, K> {
public:
    using Key = typename std::remove_cv<typename std::remove_reference<K>::type>::type;
    using Map = std::unordered_map<Key, M*>;
    StandardFactory() = default;
    virtual ~StandardFactory() = default;

    virtual bool registerObject(M* obj);

    virtual T* create(K key) const override;
    virtual bool hasKey(K key) const override;
    virtual std::vector<Key> getKeys() const;

protected:
    Map map_;
};

template <typename T, typename M, typename K>
inline bool StandardFactory<T, M, K>::registerObject(M* obj) {
    return util::insert_unique(map_, obj->getClassIdentifier(), obj);
}

template <typename T, typename M, typename K>
T* StandardFactory<T, M, K>::create(K key) const {
    return util::map_find_or_null(map_, key, [](M* o) { return o->create(); });
}

template <typename T, typename M, typename K>
bool StandardFactory<T, M, K>::hasKey(K key) const {
    return util::has_key(map_, key);
}

template <typename T, typename M, typename K>
auto StandardFactory<T, M, K>::getKeys() const -> std::vector<Key> {
    auto res = std::vector<Key>();
    for (auto& elem : map_) res.push_back(elem.first);
    return res;
}

}  // namespace

#endif  // IVW_INVIWOFACETORYBASE_H
