/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/typetraits.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/util/logcentral.h>

#include <string>
#include <string_view>
#include <memory>
#include <map>

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
template <typename K = std::string_view>
class FactoryBase {
public:
    FactoryBase() = default;
    virtual ~FactoryBase() = default;
    FactoryBase(const FactoryBase&) = delete;
    FactoryBase& operator=(const FactoryBase&) = delete;
    FactoryBase(FactoryBase&&) = default;
    FactoryBase& operator=(FactoryBase&&) = default;

    virtual bool hasKey(K key) const = 0;
};
extern template class IVW_CORE_TMPL_EXP FactoryBase<std::string_view>;

/**
 * An abstract factory interface. Inherits virtually from factory base, since an implementation
 * might implement several factory interfaces
 * T Models the object created, T will be constructed using Args...
 */
template <typename T, typename K = std::string_view, typename... Args>
class Factory : public virtual FactoryBase<K> {
public:
    Factory() = default;
    virtual std::unique_ptr<T> create(K key, Args... args) const = 0;
    virtual bool hasKey(K key) const = 0;
};

template <typename M, typename Key, typename K>
class FactoryRegister : public FactoryObservable<M> {
public:
    // The factory will not assume ownership over obj, although is assumes that obj will be
    // valid for the lifetime of the factory
    virtual bool registerObject(M* obj);
    virtual bool unRegisterObject(M* obj);

    bool hasKey(K key) const;
    std::vector<Key> getKeys() const;

protected:
    std::map<Key, M*, std::less<>> map_;
};

template <typename K>
struct FactoryKeyType {
    using type = std::remove_cv_t<std::remove_reference_t<K>>;
};
template <>
struct FactoryKeyType<std::string_view> {
    using type = std::string;
};
template <typename K>
using factoryKeyType = typename FactoryKeyType<K>::type;

/**
 * T Models the object created
 * M Models a object with a function create(K key, Args...) that can create objects of type T with
 * constructor T(Args...)
 * M would usually be a "factory object" type
 * K Models a key used to look up T
 */
template <typename T, typename M, typename K = std::string_view, typename... Args>
class StandardFactory : public Factory<T, K, Args...>,
                        public FactoryRegister<M, factoryKeyType<K>, K> {
public:
    virtual std::unique_ptr<T> create(K key, Args... args) const override {
        auto it = this->map_.find(key);
        if (it != end(this->map_)) {
            return it->second->create(args...);
        } else {
            return nullptr;
        }
    }
    virtual bool hasKey(K key) const override {
        return FactoryRegister<M, factoryKeyType<K>, K>::hasKey(key);
    }
};

/**
 * T Models the object created
 * T needs to have a clone() function.
 */
template <typename T>
class CloningFactory : public Factory<T, std::string_view>,
                       public FactoryRegister<T, std::string, std::string_view> {
public:
    CloningFactory() = default;

    virtual std::unique_ptr<T> create(std::string_view key) const override {
        return std::unique_ptr<T>(
            util::map_find_or_null(this->map_, key, [](T* o) { return o->clone(); }));
    }
    virtual bool hasKey(std::string_view key) const override {
        return FactoryRegister<T, std::string, std::string_view>::hasKey(key);
    }
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

template <typename M, typename Key, typename K>
bool FactoryRegister<M, Key, K>::registerObject(M* obj) {
    if (util::insert_unique(map_, obj->getClassIdentifier(), obj)) {
        this->notifyObserversOnRegister(obj);
        return true;
    } else {
        LogWarn("Failed to register object \"" << detail::filter(obj->getClassIdentifier())
                                               << "\", already registered");
        return false;
    }
}

template <typename M, typename Key, typename K>
bool FactoryRegister<M, Key, K>::unRegisterObject(M* obj) {
    size_t removed =
        util::map_erase_remove_if(map_, [obj](const auto& elem) { return elem.second == obj; });
    if (removed > 0) {
        this->notifyObserversOnRegister(obj);
        return true;
    } else {
        return false;
    }
}

template <typename M, typename Key, typename K>
bool FactoryRegister<M, Key, K>::hasKey(K key) const {
    return map_.find(key) != map_.end();
}

template <typename M, typename Key, typename K>
auto FactoryRegister<M, Key, K>::getKeys() const -> std::vector<Key> {
    auto res = std::vector<Key>();
    for (const auto& elem : map_) res.push_back(elem.first);
    return res;
}

}  // namespace inviwo
