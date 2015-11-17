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
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/util/logcentral.h>

#include <string>
#include <memory>
#include <unordered_map>

namespace inviwo {

class Serializable;

template <typename T>
class FactoryObserver : public Observer {
public:
    virtual void onRegister(T* p) {}
    virtual void onUnRegister(T* p) {}
};

template <typename T>
class FactoryObservable : public Observable<FactoryObserver<T>> {
protected:
    void notifyObserversOnRegister(T* p) const {
        for (auto o : this->observers_) o->onRegister(p);
    }
    void notifyObserversOnUnRegister(T* p) const {
        for (auto o : this->observers_) o->onUnRegister(p);
    }
};

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

    virtual std::unique_ptr<T> create(K key, Args&&... args) const = 0;
    virtual bool hasKey(K key) const = 0;
};

/**
 * T Models the object created
 * M Models a object with a function create(K key) that can create objects of type T
 * M would usually be a "factory object" type
 */
template <typename T, typename M, typename K = const std::string&>
class StandardFactory : public Factory<T, K> {
public:
    using Key = typename std::remove_cv<typename std::remove_reference<K>::type>::type;
    using Map = std::unordered_map<Key, M*>;
    StandardFactory() = default;
    virtual ~StandardFactory() = default;
    StandardFactory(const StandardFactory&) = delete;
    StandardFactory& operator=(const StandardFactory&) = delete;
    //StandardFactory(StandardFactory&&) = default; TODO enable... No support in VS2013!!! 
    //StandardFactory& operator=(StandardFactory&&) = default;

    // The factory will not assume ownership over obj, although is assumes that obj will be
    // valid for the lifetime of the factory
    virtual bool registerObject(M* obj);
    virtual bool unRegisterObject(M* obj);

    virtual std::unique_ptr<T> create(K key) const override;
    virtual bool hasKey(K key) const override;
    virtual std::vector<Key> getKeys() const;

protected:
    Map map_;
};

template <typename T, typename M, typename K>
inline bool StandardFactory<T, M, K>::registerObject(M* obj) {
    if (util::insert_unique(map_, obj->getClassIdentifier(), obj)) {
        return true;
    } else {
        LogWarn("Failed to register object " << obj->getClassIdentifier()
                                             << ", already registered");
        return false;
    }
}
template <typename T, typename M, typename K>
inline bool StandardFactory<T, M, K>::unRegisterObject(M* obj) {
    size_t removed = util::map_erase_remove_if(
        map_, [obj](typename Map::value_type& elem) { return elem.second == obj; });

    return removed > 0;
}

template <typename T, typename M, typename K>
std::unique_ptr<T> StandardFactory<T, M, K>::create(K key) const {
    return std::unique_ptr<T>(util::map_find_or_null(map_, key, [](M* o) { return o->create(); }));
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

/**
* T Models the object created
* T needs to have a clone() function.
*/
template <typename T, typename K = const std::string&>
class CloningFactory : public Factory<T, K> {
public:
    using Key = typename std::remove_cv<typename std::remove_reference<K>::type>::type;
    using Map = std::unordered_map<Key, T*>;
    CloningFactory() = default;
    virtual ~CloningFactory() = default;
    CloningFactory(const CloningFactory&) = delete;
    CloningFactory& operator=(const CloningFactory&) = delete;
    //CloningFactory(CloningFactory&&) = default;
    //CloningFactory& operator=(CloningFactory&&) = default;

    // The factory will not assume ownership over obj, although is assumes that obj will be
    // valid for the lifetime of the factory
    virtual bool registerObject(T* obj);
    virtual bool unRegisterObject(T* obj);

    virtual std::unique_ptr<T> create(K key) const override;
    virtual bool hasKey(K key) const override;
    virtual std::vector<Key> getKeys() const;

protected:
    Map map_;
};

template <typename T, typename K /*= const std::string&*/>
bool inviwo::CloningFactory<T, K>::registerObject(T* obj) {
    return util::insert_unique(map_, obj->getClassIdentifier(), obj);
}

template <typename T, typename K /*= const std::string&*/>
bool inviwo::CloningFactory<T, K>::unRegisterObject(T* obj) {
    size_t removed = util::map_erase_remove_if(
        map_, [obj](typename Map::value_type& elem) { return elem.second == obj; });

    return removed > 0;
}

template <typename T, typename K /*= const std::string&*/>
std::unique_ptr<T> inviwo::CloningFactory<T, K>::create(K key) const {
    return std::unique_ptr<T>(util::map_find_or_null(map_, key, [](T* o) { return o->clone(); }));
}

template <typename T, typename K /*= const std::string&*/>
bool inviwo::CloningFactory<T, K>::hasKey(K key) const {
    return util::has_key(map_, key);
}

template <typename T, typename K /*= const std::string&*/>
auto inviwo::CloningFactory<T, K>::getKeys() const -> std::vector<Key> {
    auto res = std::vector<Key>();
    for (auto& elem : map_) res.push_back(elem.first);
    return res;
}

}  // namespace

#endif  // IVW_INVIWOFACETORYBASE_H
