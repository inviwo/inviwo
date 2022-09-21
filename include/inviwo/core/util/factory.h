/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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
#include <inviwo/core/util/iterrange.h>
#include <inviwo/core/util/typetraits.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/transformiterator.h>

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
 * A base class for factories using std::string_view keys.
 * @see Deserializer::registerFactory WorkspaceManager::registerFactory
 */
class IVW_CORE_API FactoryBase {
public:
    FactoryBase();
    virtual ~FactoryBase();
    FactoryBase(const FactoryBase&) = delete;
    FactoryBase& operator=(const FactoryBase&) = delete;
    FactoryBase(FactoryBase&&) = default;
    FactoryBase& operator=(FactoryBase&&) = default;

    virtual bool hasKey(std::string_view key) const = 0;
};

/**
 * An abstract factory interface.
 * @tparam T Models the object created, @p T will be constructed using @p Args...
 * @tparam Args A variadic list of arguments passed to @p T on construction
 */
template <typename T, typename K = std::string_view, typename... Args>
class Factory {
public:
    Factory() = default;
    virtual ~Factory() = default;
    Factory(const Factory&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory(Factory&&) = default;
    Factory& operator=(Factory&&) = default;

    virtual std::unique_ptr<T> create(K key, Args... args) const = 0;
    virtual bool hasKey(K key) const = 0;
};

/**
 * An abstract factory interface for factories using std::string_view keys. Inherits virtually from
 * factory base, since an implementation might implement several factory interfaces
 */
template <typename T, typename... Args>
class Factory<T, std::string_view, Args...> : public virtual FactoryBase {
public:
    Factory() = default;
    virtual std::unique_ptr<T> create(std::string_view key, Args... args) const = 0;
    virtual bool hasKey(std::string_view key) const = 0;
};

/**
 * A map of @p Keys and associated @p FactoryObjects.
 *
 * @tparam FactoryObject A factory type for some object. A `LookUpKey getClassIdentifier()´
 *                       function is required for registration in the map.
 * @tparam Key the key type used in the map
 * @tparam LookUpKey A type used to lookup FactoryObjects in the map
 */
template <typename FactoryObject, typename Key, typename LookUpKey>
class FactoryRegister : public FactoryObservable<FactoryObject> {
public:
    /**
     * The factory will not assume ownership over obj, although is assumes that obj will be
     * valid for the lifetime of the factory
     */
    virtual bool registerObject(FactoryObject* obj);
    virtual bool unRegisterObject(FactoryObject* obj);

    bool hasKey(LookUpKey key) const;
    std::vector<Key> getKeys() const;

    auto getKeyView() const {
        constexpr auto getFirst = [](auto&& pair) -> decltype(auto) { return pair.first; };
        return util::as_range(util::makeTransformIterator(getFirst, map_.begin()),
                              util::makeTransformIterator(getFirst, map_.end()));
    }

    FactoryObject* getFactoryObject(LookUpKey key) const;

protected:
    std::map<Key, FactoryObject*, std::less<>> map_;
};

/**
 * Map the "lookup key" type of a map to the stored key type.
 * I.e the lookup `const std::string&` would map to `std::string`
 */
template <typename K>
struct FactoryKeyType {
    using type = std::remove_cv_t<std::remove_reference_t<K>>;
};
/**
 * Specialization that maps std::string_view to std::string
 * @see FactoryKeyType
 */
template <>
struct FactoryKeyType<std::string_view> {
    using type = std::string;
};
template <typename K>
using factoryKeyType = typename FactoryKeyType<K>::type;

/**
 * Standard setup of Factory using factory objects to create an instance.
 * The factory will use a FactoryRegister of @p FactoryObject and @p factoryKeyType<Key>. This
 * indirection allows std::string_view to be used as key where as the FactoryRegister will store
 * strings for the keys.
 * @tparam T The object created
 * @tparam FactoryObject An object with a function `std::unique_ptr<T> create(Args...)` that can
 *                       create objects of type @p T with constructor `T(Args...)`.
 *                       @p FactoryObject would usually be a "factory object".
 *                       A `getClassIdentifier()´ function is also required for registration.
 * @tparam Key Models a key used to look up @p T
 * @tparam Args A variadic list of arguments passed to @p T on construction
 */
template <typename T, typename FactoryObject, typename Key = std::string_view, typename... Args>
class StandardFactory : public Factory<T, Key, Args...>,
                        public FactoryRegister<FactoryObject, factoryKeyType<Key>, Key> {
public:
    virtual std::unique_ptr<T> create(Key key, Args... args) const override {
        auto it = this->map_.find(key);
        if (it != end(this->map_)) {
            return it->second->create(args...);
        } else {
            return nullptr;
        }
    }
    virtual bool hasKey(Key key) const override {
        return FactoryRegister<FactoryObject, factoryKeyType<Key>, Key>::hasKey(key);
    }
};

/**
 * A factory for cloning object
 * The type @p T needs to have a `clone()` and a `getClassIdentifier()´ function.
 * @tparam T Models the object created.
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
M* FactoryRegister<M, Key, K>::getFactoryObject(K key) const {
    if (auto it = map_.find(key); it != map_.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

template <typename M, typename Key, typename K>
auto FactoryRegister<M, Key, K>::getKeys() const -> std::vector<Key> {
    auto res = std::vector<Key>();
    for (const auto& elem : map_) res.push_back(elem.first);
    return res;
}

}  // namespace inviwo
