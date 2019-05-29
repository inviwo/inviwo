/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
    virtual void onRegister(T* /*p*/) {}
    virtual void onUnRegister(T* /*p*/) {}
};

template <typename T>
class FactoryObservable : public Observable<FactoryObserver<T>> {
protected:
    void notifyObserversOnRegister(T* p) {
        this->forEachObserver([&](FactoryObserver<T>* o) { o->onRegister(p); });
    }
    void notifyObserversOnUnRegister(T* p) {
        this->forEachObserver([&](FactoryObserver<T>* o) { o->onUnRegister(p); });
    }
};

/**
 * A common factory base class
 */
class IVW_CORE_API FactoryBase {
public:
    FactoryBase() = default;
    virtual ~FactoryBase() = default;
    FactoryBase(const FactoryBase&) = delete;
    FactoryBase& operator=(const FactoryBase&) = delete;
    FactoryBase(FactoryBase&&) = default;
    FactoryBase& operator=(FactoryBase&&) = default;
};

/**
 * An abstract factory interface. Inherits virtually from factory base, since an implementation
 * might implement several factory interfaces
 * T Models the object created, T will be constructed using Args...
 * K Models a key used to look up T
 */
template <typename T, typename K = const std::string&, typename... Args>
class Factory : public virtual FactoryBase {
public:
    Factory() = default;
    virtual std::unique_ptr<T> create(K key, Args... args) const = 0;
    virtual bool hasKey(K key) const = 0;
};

/**
 * T Models the object created
 * M Models a object with a function create(K key, Args...) that can create objects of type T with
 * constructor T(Args...)
 * M would usually be a "factory object" type
 * K Models a key used to look up T
 */
template <typename T, typename M, typename K = const std::string&, typename... Args>
class StandardFactory : public Factory<T, K, Args...>, public FactoryObservable<M> {
public:
    using Key = typename std::remove_cv<typename std::remove_reference<K>::type>::type;
    using Map = std::unordered_map<Key, M*>;
    StandardFactory() = default;

    // The factory will not assume ownership over obj, although is assumes that obj will be
    // valid for the lifetime of the factory
    virtual bool registerObject(M* obj);
    virtual bool unRegisterObject(M* obj);

    virtual std::unique_ptr<T> create(K key, Args... args) const override;
    virtual bool hasKey(K key) const override;
    virtual std::vector<Key> getKeys() const;

protected:
    Map map_;
};

namespace detail {
template <typename T>
typename std::enable_if<util::is_stream_insertable<T>::value, const T&>::type filter(const T& val) {
    return val;
}

template <typename T>
typename std::enable_if<!util::is_stream_insertable<T>::value, std::string>::type filter(const T&) {
    return "???";
}

}  // namespace detail

template <typename T, typename M, typename K, typename... Args>
bool StandardFactory<T, M, K, Args...>::registerObject(M* obj) {
    if (util::insert_unique(map_, obj->getClassIdentifier(), obj)) {
        this->notifyObserversOnRegister(obj);
        return true;
    } else {
        LogWarn("Failed to register object \"" << detail::filter(obj->getClassIdentifier())
                                               << "\", already registered");
        return false;
    }
}

template <typename T, typename M, typename K, typename... Args>
bool StandardFactory<T, M, K, Args...>::unRegisterObject(M* obj) {
    size_t removed = util::map_erase_remove_if(
        map_, [obj](typename Map::value_type& elem) { return elem.second == obj; });
    if (removed > 0) {
        this->notifyObserversOnRegister(obj);
        return true;
    } else {
        return false;
    }
}

template <typename T, typename M, typename K, typename... Args>
std::unique_ptr<T> StandardFactory<T, M, K, Args...>::create(K key, Args... args) const {
    auto it = map_.find(key);
    if (it != end(map_)) {
        return it->second->create(args...);
    } else {
        return nullptr;
    }
}

template <typename T, typename M, typename K, typename... Args>
bool StandardFactory<T, M, K, Args...>::hasKey(K key) const {
    return util::has_key(map_, key);
}

template <typename T, typename M, typename K, typename... Args>
auto StandardFactory<T, M, K, Args...>::getKeys() const -> std::vector<Key> {
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

}  // namespace inviwo

#endif  // IVW_INVIWOFACETORYBASE_H
