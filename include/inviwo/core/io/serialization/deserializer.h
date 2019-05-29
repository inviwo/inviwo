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

#ifndef IVW_DESERIALIZER_H
#define IVW_DESERIALIZER_H

#include <inviwo/core/io/serialization/serializebase.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/logfilter.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/io/serialization/nodedebugger.h>

#include <flags/flags.h>

#include <type_traits>
#include <list>
#include <istream>
#include <bitset>

namespace inviwo {

class Serializable;
class VersionConverter;
class InviwoApplication;
class FactoryBase;
template <typename T, typename K>
class ContainerWrapper;

class IVW_CORE_API Deserializer : public SerializeBase, public LogFilter {
public:
    /**
     * \brief Deserializer constructor
     *
     * @param fileName path to file that is to be deserialized.
     * @param allowReference flag to manage references to avoid multiple object creation.
     */
    Deserializer(std::string fileName, bool allowReference = true);
    /**
     * \brief Deserializes content from the stream using refPath to calculate relative paths to
     * data.
     *
     * @param stream Stream with content that is to be deserialized.
     * @param refPath A path that will be used to decode the location of data during
     * deserialization.
     * @param allowReference flag to manage references to avoid multiple object creation.
     */
    Deserializer(std::istream& stream, const std::string& refPath, bool allowReference = true);

    Deserializer(const Deserializer&) = delete;
    Deserializer(Deserializer&&) = default;
    Deserializer& operator=(const Deserializer& that) = delete;
    Deserializer& operator=(Deserializer&& that) = default;

    virtual ~Deserializer() = default;

    // std containers
    /**
     * \brief Deserialize a vector
     *
     * Deserialize the vector that has pre-allocated objects of type T or allocated by deserializer.
     * A vector is identified by key and vector items are identified by itemKey
     *
     * eg. xml tree with key=Properties and itemKey=Property
     *
     *     <Properties>
     *          <Property identifier="enableMIP" displayName="MIP">
     *              <value content="0" />
     *          </Property>
     *          <Property identifier="enableShading" displayName="Shading">
     *              <value content="0" />
     *          </Property>
     *     <Properties>
     *
     * @param key vector key.
     * @param sVector vector to be deserialized.
     * @param itemKey vector item key
     */
    template <typename T>
    void deserialize(const std::string& key, std::vector<T*>& sVector,
                     const std::string& itemKey = "item");

    template <typename T, typename C>
    void deserialize(const std::string& key, std::vector<T*>& sVector, const std::string& itemKey,
                     C identifier);

    template <typename T>
    void deserialize(const std::string& key, std::vector<T>& sVector,
                     const std::string& itemKey = "item");

    template <typename T>
    void deserialize(const std::string& key, std::unordered_set<T>& sSet,
                     const std::string& itemKey = "item");

    template <typename T>
    void deserialize(const std::string& key, std::vector<std::unique_ptr<T>>& vector,
                     const std::string& itemKey = "item");

    template <typename T>
    void deserialize(const std::string& key, std::list<T>& sContainer,
                     const std::string& itemKey = "item");

    /**
     * \brief  Deserialize a map
     *
     * Deserialize a map, which can have
     * keys of type K,
     * values of type V* (pointers)
     * and compare function C ( optional if
     * K primitive type, i.e., std::string, int, etc.,)
     * eg., std::map<std::string, Property*>
     *
     * eg. xml tree
     *\code{.xml}
     *     <Properties>
     *          <Property identifier="enableMIP" displayName="MIP">
     *              <value content="0" />
     *          </Property>
     *          <Property identifier="enableShading" displayName="Shading">
     *              <value content="0" />
     *          </Property>
     *     <Properties>
     *\endcode
     * In the above xml tree,
     *
     * key                   = "Properties"
     * itemKey               = "Property"
     * param comparisionAttribute  = "identifier"
     * param sMap["enableMIP"]     = address of a property
     *       sMap["enableShading"] = address of a property
     *         where, "enableMIP" & "enableShading" are keys.
     *         address of a property is a value
     *
     * Note: If children has attribute "type", then comparisionAttribute becomes meaningless.
     *       Because deserializer always allocates a new instance of type using registered
     *       factories. eg.,
     *           <Processor type="EntryExitPoints" identifier="EntryExitPoints" reference="ref2" />
     *
     * @param key Map key or parent node of itemKey.
     * @param sMap  map to be deserialized - source / input map.
     * @param itemKey map item key of children nodes.
     * @param comparisionAttribute forced comparison attribute.
     */
    template <typename K, typename V, typename C, typename A>
    void deserialize(const std::string& key, std::map<K, V, C, A>& sMap,
                     const std::string& itemKey = "item",
                     const std::string& comparisionAttribute = SerializeConstants::KeyAttribute);

    template <typename K, typename V, typename C, typename A>
    void deserialize(const std::string& key, std::map<K, V*, C, A>& sMap,
                     const std::string& itemKey = "item",
                     const std::string& comparisionAttribute = SerializeConstants::KeyAttribute);

    template <typename K, typename V, typename C, typename A>
    void deserialize(const std::string& key, std::map<K, std::unique_ptr<V>, C, A>& sMap,
                     const std::string& itemKey = "item",
                     const std::string& comparisionAttribute = SerializeConstants::KeyAttribute);

    template <typename T, typename K>
    void deserialize(const std::string& key, ContainerWrapper<T, K>& container);

    // Specializations for chars
    void deserialize(const std::string& key, signed char& data,
                     const SerializationTarget& target = SerializationTarget::Node);
    void deserialize(const std::string& key, char& data,
                     const SerializationTarget& target = SerializationTarget::Node);
    void deserialize(const std::string& key, unsigned char& data,
                     const SerializationTarget& target = SerializationTarget::Node);

    // integers, strings, reals
    template <typename T, typename std::enable_if<std::is_integral<T>::value ||
                                                      util::is_floating_point<T>::value ||
                                                      util::is_string<T>::value,
                                                  int>::type = 0>
    void deserialize(const std::string& key, T& data,
                     const SerializationTarget& target = SerializationTarget::Node);

    // Enum types
    template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
    void deserialize(const std::string& key, T& data,
                     const SerializationTarget& target = SerializationTarget::Node);

    // Flag types
    template <typename T>
    void deserialize(const std::string& key, flags::flags<T>& data,
                     const SerializationTarget& target = SerializationTarget::Node);

    // glm vector types
    template <typename Vec, typename std::enable_if<util::rank<Vec>::value == 1, int>::type = 0>
    void deserialize(const std::string& key, Vec& data);

    // glm matrix types
    template <typename Mat, typename std::enable_if<util::rank<Mat>::value == 2, int>::type = 0>
    void deserialize(const std::string& key, Mat& data);

    // bitsets
    template <unsigned N>
    void deserialize(const std::string& key, std::bitset<N>& bits);

    /**
     * \brief  Deserialize any Serializable object
     */
    void deserialize(const std::string& key, Serializable& sObj);
    /**
     * \brief  Deserialize pointer data of type T, which is of type
     *         serializable object or primitive data
     */
    template <class T>
    void deserialize(const std::string& key, T*& data);
    template <class Base, class T>
    void deserializeAs(const std::string& key, T*& data);

    template <class T, class D>
    void deserialize(const std::string& key, std::unique_ptr<T, D>& data);

    template <class Base, class T, class D>
    void deserializeAs(const std::string& key, std::unique_ptr<T, D>& data);

    void setExceptionHandler(ExceptionHandler handler);
    void handleError(const ExceptionContext& context);

    void convertVersion(VersionConverter* converter);

    /**
     * \brief For allocating objects such as processors, properties.. using registered factories.
     *
     * @param className is used by registered factories to allocate the required object.
     * @return T* nullptr if allocation fails or className does not exist in any factories.
     */
    template <typename T>
    T* getRegisteredType(const std::string& className);

    /**
     * \brief For allocating objects that do not belong to any registered factories.
     *
     * @return T* Pointer to object of type T.
     */
    template <typename T>
    T* getNonRegisteredType();

    friend class NodeSwitch;
    template <typename T, typename K>
    friend class ContainerWrapper;

    void registerFactory(FactoryBase* factory);

    int getInviwoWorkspaceVersion() const;

private:
    // integers, strings
    template <typename T,
              typename std::enable_if<!util::is_floating_point<T>::value, int>::type = 0>
    void getSafeValue(const std::string& key, T& data);

    // reals
    template <typename T, typename std::enable_if<util::is_floating_point<T>::value, int>::type = 0>
    void getSafeValue(const std::string& key, T& data);

    void storeReferences(TxElement* node);

    ExceptionHandler exceptionHandler_;
    std::map<std::string, TxElement*> referenceLookup_;

    std::vector<FactoryBase*> registeredFactories_;

    int inviwoWorkspaceVersion_ = 0;
};

/**
 * \class ContainerWrapper
 */
template <typename T, typename K = std::string>
class ContainerWrapper {
public:
    struct Item {
        bool doDeserialize;
        T& value;
        std::function<void(T&)> callback;
    };
    using Getter = std::function<Item(const K& id, size_t ind)>;
    using IdentityGetter = std::function<K(TxElement* node)>;

    ContainerWrapper(std::string itemKey, Getter getItem) : getItem_(getItem), itemKey_(itemKey) {}

    virtual ~ContainerWrapper() = default;

    const std::string& getItemKey() const { return itemKey_; }

    void deserialize(Deserializer& d, TxElement* node, size_t ind) {
        auto item = getItem_(idGetter_(node), ind);
        if (item.doDeserialize) {
            try {
                d.deserialize(itemKey_, item.value);
                item.callback(item.value);
            } catch (...) {
                d.handleError(IVW_CONTEXT);
            }
        }
    }

    void setIdentityGetter(IdentityGetter getter) { idGetter_ = getter; }

private:
    IdentityGetter idGetter_ = [](TxElement* node) {
        K key{};
        node->GetAttribute("identifier", &key, false);
        return key;
    };

    Getter getItem_;
    const std::string itemKey_;
};

namespace util {

/**
 * A helper class for more advanced deserialization. useful when one has to call observer
 * notifications for example.
 * Example usage, serialize as usual
 * ```{.cpp}
 *     s.serialize("TFPrimitives", values_, "point");
 *
 * ```
 * Then deserialize with notifications:
 * ```{.cpp}
 *    util::IndexedDeserializer<std::unique_ptr<TFPrimitiveSet>>("TFPrimitives", "point")
 *       .onNew([&](std::unique_ptr<TFPrimitiveSet>& primitive) {
 *           notifyControlPointAdded(primitive.get());
 *       })
 *       .onRemove([&](std::unique_ptr<TFPrimitiveSet>& primitive) {
 *           notifyControlPointRemoved(primitive.get());
 *       })(d, values_);
 * ```
 */
template <typename T>
class IndexedDeserializer {
public:
    IndexedDeserializer(std::string key, std::string itemKey) : key_(key), itemKey_(itemKey) {}

    IndexedDeserializer<T>& setMakeNew(std::function<T()> makeNewItem) {
        makeNewItem_ = std::move(makeNewItem);
        return *this;
    }
    IndexedDeserializer<T>& onNew(std::function<void(T&)> onNewItem) {
        onNewItem_ = std::move(onNewItem);
        return *this;
    }
    IndexedDeserializer<T>& onRemove(std::function<void(T&)> onRemoveItem) {
        onRemoveItem_ = std::move(onRemoveItem);
        return *this;
    }

    template <typename C>
    void operator()(Deserializer& d, C& container) {
        T tmp{};
        size_t count = 0;
        ContainerWrapper<T> cont(
            itemKey_, [&](std::string id, size_t ind) -> typename ContainerWrapper<T>::Item {
                ++count;
                if (ind < container.size()) {
                    return {true, container[ind], [&](T& /*val*/) {}};
                } else {
                    tmp = makeNewItem_();
                    return {true, tmp, [&](T& /*val*/) {
                                container.push_back(std::move(tmp));
                                onNewItem_(container.back());
                            }};
                }
            });

        d.deserialize(key_, cont);

        while (container.size() > count) {
            auto elem = std::move(container.back());
            container.pop_back();
            onRemoveItem_(elem);
        }
    }

private:
    std::function<T()> makeNewItem_ = []() -> T { return T{}; };
    std::function<void(T&)> onNewItem_ = [](T&) {};
    std::function<void(T&)> onRemoveItem_ = [](T&) {};

    std::string key_;
    std::string itemKey_;
};

template <typename K, typename T>
class IdentifiedDeserializer {
public:
    IdentifiedDeserializer(std::string key, std::string itemKey) : key_(key), itemKey_(itemKey) {}

    IdentifiedDeserializer<K, T>& setGetId(std::function<K(const T&)> getID) {
        getID_ = getID;
        return *this;
    }
    IdentifiedDeserializer<K, T>& setMakeNew(std::function<T()> makeNewItem) {
        makeNewItem_ = makeNewItem;
        return *this;
    }
    IdentifiedDeserializer<K, T>& setNewFilter(
        std::function<bool(const K& id, size_t ind)> filter) {
        filter_ = filter;
        return *this;
    }
    IdentifiedDeserializer<K, T>& onNew(std::function<void(T&)> onNewItem) {
        onNewItem_ = [onNewItem](T& i, size_t) { onNewItem(i); };
        return *this;
    }
    IdentifiedDeserializer<K, T>& onNewIndexed(std::function<void(T&, size_t)> onNewItem) {
        onNewItem_ = onNewItem;
        return *this;
    }
    IdentifiedDeserializer<K, T>& onRemove(std::function<void(const K&)> onRemoveItem) {
        onRemoveItem_ = onRemoveItem;
        return *this;
    }

    template <typename C>
    void operator()(Deserializer& d, C& container) {
        T tmp{};
        auto toRemove = util::transform(container, [&](const T& x) -> K { return getID_(x); });
        ContainerWrapper<T, K> cont(
            itemKey_, [&](K id, size_t ind) -> typename ContainerWrapper<T, K>::Item {
                util::erase_remove(toRemove, id);
                auto it = util::find_if(container, [&](T& i) { return getID_(i) == id; });
                if (it != container.end()) {
                    return {true, *it, [&](T& /*val*/) {}};
                } else {
                    tmp = makeNewItem_();
                    return {filter_(id, ind), tmp, [&](T& val) { onNewItem_(val, ind); }};
                }
            });

        d.deserialize(key_, cont);
        for (auto& id : toRemove) onRemoveItem_(id);
    }

private:
    std::function<K(const T&)> getID_ = [](const T&) -> K {
        throw Exception("GetID callback is not set!");
    };
    std::function<T()> makeNewItem_ = []() -> T {
        throw Exception("MakeNew callback is not set!");
    };
    std::function<void(T&, size_t)> onNewItem_ = [](T&, size_t) {
        throw Exception("OnNew callback is not set!");
    };
    std::function<void(const K&)> onRemoveItem_ = [](const K&) {
        throw Exception("OnRemove callback is not set!");
    };

    std::function<bool(const K& id, size_t ind)> filter_ = [](const K& /*id*/, size_t /*ind*/) {
        return true;
    };

    std::string key_;
    std::string itemKey_;
};

template <typename K, typename T>
class MapDeserializer {
public:
    MapDeserializer(std::string key, std::string itemKey,
                    std::string attribKey = SerializeConstants::KeyAttribute)
        : key_(key), itemKey_(itemKey), attribKey_(attribKey) {}

    MapDeserializer<K, T>& setMakeNew(std::function<T()> makeNewItem) {
        makeNewItem_ = makeNewItem;
        return *this;
    }
    MapDeserializer<K, T>& setNewFilter(std::function<bool(const K& id, size_t ind)> filter) {
        filter_ = filter;
        return *this;
    }
    MapDeserializer<K, T>& onNew(std::function<void(const K&, T&)> onNewItem) {
        onNewItem_ = onNewItem;
        return *this;
    }
    MapDeserializer<K, T>& onRemove(std::function<void(const K&)> onRemoveItem) {
        onRemoveItem_ = onRemoveItem;
        return *this;
    }
    MapDeserializer<K, T>& setIdentifierTransform(std::function<K(const K&)> identifierTransform) {
        identifierTransform_ = identifierTransform;
        return *this;
    }

    template <typename C>
    void operator()(Deserializer& d, C& container) {
        T tmp{};
        auto toRemove = util::transform(
            container, [](const std::pair<const K, T>& item) { return item.first; });
        ContainerWrapper<T, K> cont(
            itemKey_, [&](K id, size_t ind) -> typename ContainerWrapper<T, K>::Item {
                util::erase_remove(toRemove, id);
                auto it = container.find(id);
                if (it != container.end()) {
                    return {true, it->second, [&](T& /*val*/) {}};
                } else {
                    tmp = makeNewItem_();
                    return {filter_(id, ind), tmp, [&, id](T& val) { onNewItem_(id, val); }};
                }
            });

        cont.setIdentityGetter([&](TxElement* node) {
            K key{};
            node->GetAttribute(attribKey_, &key);
            return identifierTransform_(key);
        });

        d.deserialize(key_, cont);

        for (auto& id : toRemove) onRemoveItem_(id);
    }

private:
    std::function<T()> makeNewItem_ = []() -> T {
        throw Exception("MakeNew callback is not set!");
    };
    std::function<void(const K&, T&)> onNewItem_ = [](const K&, T&) {
        throw Exception("OnNew callback is not set!");
    };
    std::function<void(const K&)> onRemoveItem_ = [](const K&) {
        throw Exception("OnRemove callback is not set!");
    };
    std::function<bool(const K& id, size_t ind)> filter_ = [](const K& /*id*/, size_t /*ind*/) {
        return true;
    };
    std::function<K(const K&)> identifierTransform_ = [](const K& identifier) {
        return identifier;
    };

    const std::string key_;
    const std::string itemKey_;
    const std::string attribKey_;
};

}  // namespace util

template <typename T>
T* Deserializer::getRegisteredType(const std::string& className) {
    for (auto base : registeredFactories_) {
        if (auto factory = dynamic_cast<Factory<T>*>(base)) {
            if (auto data = factory->create(className)) return data.release();
        }
    }
    return nullptr;
}

template <typename T>
T* Deserializer::getNonRegisteredType() {
    return util::defaultConstructType<T>();
}

template <typename T>
class DeserializationErrorHandle {
public:
    template <typename... Args>
    DeserializationErrorHandle(Deserializer& d, Args&&... args);
    virtual ~DeserializationErrorHandle();
    T& getHandler();

private:
    T handler_;
    Deserializer& d_;
};

template <typename T>
template <typename... Args>
DeserializationErrorHandle<T>::DeserializationErrorHandle(Deserializer& d, Args&&... args)
    : handler_(std::forward<Args>(args)...), d_(d) {
    d_.setExceptionHandler([this](ExceptionContext c) { handler_(c); });
}

template <typename T>
DeserializationErrorHandle<T>::~DeserializationErrorHandle() {
    d_.setExceptionHandler(nullptr);
}

template <typename T>
T& DeserializationErrorHandle<T>::getHandler() {
    return handler_;
}

template <typename T>
struct ParseWrapper {
    ParseWrapper(T& val) : value(val) {}
    T& value;
};

template <class Elem, class Traits, typename T>
std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& is,
                                             ParseWrapper<T>& wrapper) {
    auto sp = is.tellg();  // Save position
    if (is >> wrapper.value) return is;

    // Handle parse errors
    if (is.rdstate() != std::ios_base::failbit) return is;

    is.clear();
    is.seekg(sp);  // restore position

    std::string tmp;
    if (is >> tmp) {
        if (tmp == "inf")
            wrapper.value = std::numeric_limits<T>::infinity();
        else if (tmp == "-inf")
            wrapper.value = -std::numeric_limits<T>::infinity();
        else if (tmp == "nan")
            wrapper.value = std::numeric_limits<T>::quiet_NaN();
        else if (tmp == "-nan" || tmp == "-nan(ind)")
            wrapper.value = -std::numeric_limits<T>::quiet_NaN();
        else
            is.setstate(std::ios_base::failbit);

        throw SerializationException("Error deserializing value: \"" + tmp + "\"",
                                     IVW_CONTEXT_CUSTOM("Deserialization"));
    }

    return is;
}

// integers, strings
template <typename T, typename std::enable_if<!util::is_floating_point<T>::value, int>::type>
void Deserializer::getSafeValue(const std::string& key, T& data) {
    rootElement_->GetAttribute(key, &data, false);
}

// reals specialization for reals to handled inf/nan values
template <typename T, typename std::enable_if<util::is_floating_point<T>::value, int>::type>
void Deserializer::getSafeValue(const std::string& key, T& data) {
    ParseWrapper<T> wrapper(data);
    try {
        rootElement_->GetAttribute(key, &wrapper, false);
    } catch (SerializationException& e) {
        NodeDebugger nd(rootElement_);
        throw SerializationException(e.getMessage() + ". At " + nd.getDescription(),
                                     e.getContext());
    }
}

// integers, strings, reals
template <typename T,
          typename std::enable_if<std::is_integral<T>::value || util::is_floating_point<T>::value ||
                                      util::is_string<T>::value,
                                  int>::type>
void Deserializer::deserialize(const std::string& key, T& data, const SerializationTarget& target) {
    try {
        if (target == SerializationTarget::Attribute) {
            getSafeValue(key, data);
        } else {
            if (NodeSwitch ns{*this, key}) {
                getSafeValue(SerializeConstants::ContentAttribute, data);
                return;
            }
            if (NodeSwitch ns{*this, key, true}) {
                getSafeValue(SerializeConstants::ContentAttribute, data);
                return;
            }
        }
    } catch (...) {
        handleError(IVW_CONTEXT);
    }
}

// enum types
template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type>
void Deserializer::deserialize(const std::string& key, T& data, const SerializationTarget& target) {
    using ET = typename std::underlying_type<T>::type;
    ET tmpdata{static_cast<ET>(data)};
    deserialize(key, tmpdata, target);
    data = static_cast<T>(tmpdata);
}

// Flag types
template <typename T>
void Deserializer::deserialize(const std::string& key, flags::flags<T>& data,
                               const SerializationTarget& target) {

    auto tmp = data.underlying_value();
    deserialize(key, tmp, target);
    data.set_underlying_value(tmp);
}

// glm vector types
template <typename Vec, typename std::enable_if<util::rank<Vec>::value == 1, int>::type>
void Deserializer::deserialize(const std::string& key, Vec& data) {
    if (NodeSwitch ns{*this, key}) {
        for (size_t i = 0; i < util::extent<Vec, 0>::value; ++i) {
            try {
                getSafeValue(SerializeConstants::VectorAttributes[i], data[i]);
            } catch (...) {
                handleError(IVW_CONTEXT);
            }
        }
    }
}

// glm matrix types
template <typename Mat, typename std::enable_if<util::rank<Mat>::value == 2, int>::type>
void Deserializer::deserialize(const std::string& key, Mat& data) {
    if (NodeSwitch ns{*this, key}) {
        for (size_t i = 0; i < util::extent<Mat, 0>::value; ++i) {
            // Deserialization of row needs to be here for backwards compatibility,
            // we used to incorrectly serialize columns as row
            deserialize("row" + toString(i), data[i]);
            deserialize("col" + toString(i), data[i]);
        }
    }
}

template <unsigned N>
void Deserializer::deserialize(const std::string& key, std::bitset<N>& bits) {
    std::string value = bits.to_string();
    deserialize(key, value);
    bits = std::bitset<N>(value);
}

template <typename T>
void Deserializer::deserialize(const std::string& key, std::vector<T*>& vector,
                               const std::string& itemKey) {
    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;

    unsigned int i = 0;
    TxEIt child(itemKey);
    for (child = child.begin(rootElement_); child != child.end(); ++child) {
        // In the next deserialization call do net fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, &(*child), false);
        if (vector.size() <= i) {
            T* item = nullptr;
            try {
                deserialize(itemKey, item);
                vector.push_back(item);
            } catch (...) {
                delete item;
                handleError(IVW_CONTEXT);
            }
        } else {
            try {
                deserialize(itemKey, vector[i]);
            } catch (...) {
                handleError(IVW_CONTEXT);
            }
        }
        i++;
    }
}

template <typename T>
void Deserializer::deserialize(const std::string& key, std::vector<std::unique_ptr<T>>& vector,
                               const std::string& itemKey) {
    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;

    unsigned int i = 0;
    TxEIt child(itemKey);
    for (child = child.begin(rootElement_); child != child.end(); ++child) {
        // In the next deserialization call do net fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, &(*child), false);
        if (vector.size() <= i) {
            T* item = nullptr;
            try {
                deserialize(itemKey, item);
                vector.emplace_back(item);
            } catch (...) {
                delete item;
                handleError(IVW_CONTEXT);
            }
        } else {
            try {
                if (auto ptr = vector[i].get()) {
                    deserialize(itemKey, ptr);
                } else {
                    deserialize(itemKey, ptr);
                    vector[i].reset(ptr);
                }
            } catch (...) {
                handleError(IVW_CONTEXT);
            }
        }
        i++;
    }
}

template <typename T, typename C>
void Deserializer::deserialize(const std::string& key, std::vector<T*>& vector,
                               const std::string& itemKey, C identifier) {
    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;

    auto lastInsertion = vector.begin();

    TxEIt child(itemKey);
    for (child = child.begin(rootElement_); child != child.end(); ++child) {
        identifier.setKey(child.Get());
        auto it = std::find_if(vector.begin(), vector.end(), identifier);

        if (it != vector.end()) {  // There is a item in vector with same identifier as on disk
            NodeSwitch elementNodeSwitch(*this, &(*child), false);
            try {
                deserialize(itemKey, *it);
                lastInsertion = it;
            } catch (...) {
                handleError(IVW_CONTEXT);
            }
        } else {  // No item in vector matches item on disk, create a new one.
            T* item = nullptr;
            NodeSwitch elementNodeSwitch(*this, &(*child), false);
            try {
                deserialize(itemKey, item);
                // Insert new item after the previous item deserialized
                lastInsertion = lastInsertion == vector.end() ? lastInsertion : ++lastInsertion;
                lastInsertion = vector.insert(lastInsertion, item);
            } catch (...) {
                delete item;
                handleError(IVW_CONTEXT);
            }
        }
    }
}

template <typename T>
void Deserializer::deserialize(const std::string& key, std::vector<T>& vector,
                               const std::string& itemKey) {
    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;

    unsigned int i = 0;
    TxEIt child(itemKey);

    for (child = child.begin(rootElement_); child != child.end(); ++child) {
        // In the next deserialization call do net fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, &(*child), false);
        try {
            if (vector.size() <= i) {
                T item;
                deserialize(itemKey, item);
                vector.push_back(std::move(item));
            } else {
                deserialize(itemKey, vector[i]);
            }
        } catch (...) {
            handleError(IVW_CONTEXT);
        }
        i++;
    }
}

template <typename T>
void Deserializer::deserialize(const std::string& key, std::unordered_set<T>& set,
                               const std::string& itemKey) {
    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;

    TxEIt child(itemKey);

    for (child = child.begin(rootElement_); child != child.end(); ++child) {
        // In the next deserialization call do net fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, &(*child), false);
        try {
            T item;
            deserialize(itemKey, item);
            set.insert(std::move(item));

        } catch (...) {
            handleError(IVW_CONTEXT);
        }
    }
}

template <typename T>
void Deserializer::deserialize(const std::string& key, std::list<T>& container,
                               const std::string& itemKey) {
    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;
    unsigned int i = 0;
    TxEIt child(itemKey);

    for (child = child.begin(rootElement_); child != child.end(); ++child) {
        // In the next deserialization call do net fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, &(*child), false);
        try {
            if (container.size() <= i) {
                T item;
                deserialize(itemKey, item);
                container.push_back(item);
            } else {
                deserialize(itemKey, *std::next(container.begin(), i));
            }
        } catch (...) {
            handleError(IVW_CONTEXT);
        }
        i++;
    }
}

template <typename K, typename V, typename C, typename A>
void Deserializer::deserialize(const std::string& key, std::map<K, V, C, A>& map,
                               const std::string& itemKey,
                               const std::string& comparisionAttribute) {
    if (!isPrimitiveType(typeid(K)) || comparisionAttribute.empty())
        throw SerializationException("Error: map key has to be a primitive type", IVW_CONTEXT);

    NodeSwitch mapNodeSwitch(*this, key);
    if (!mapNodeSwitch) return;
    TxEIt child(itemKey);

    for (child = child.begin(rootElement_); child != child.end(); ++child) {
        // In the next deserialization call do net fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, &(*child), false);
        K childkey;
        child->GetAttribute(comparisionAttribute, &childkey, false);

        V value;
        auto it = map.find(childkey);
        if (it != map.end()) value = it->second;
        try {
            deserialize(itemKey, value);
            map[childkey] = value;
        } catch (...) {
            handleError(IVW_CONTEXT);
        }
    }
}

// Pointer specialization
template <typename K, typename V, typename C, typename A>
void Deserializer::deserialize(const std::string& key, std::map<K, V*, C, A>& map,
                               const std::string& itemKey,
                               const std::string& comparisionAttribute) {
    if (!isPrimitiveType(typeid(K)) || comparisionAttribute.empty())
        throw SerializationException("Error: map key has to be a primitive type", IVW_CONTEXT);

    NodeSwitch mapNodeSwitch(*this, key);
    if (!mapNodeSwitch) return;
    TxEIt child(itemKey);

    for (child = child.begin(rootElement_); child != child.end(); ++child) {
        // In the next deserialization call do net fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, &(*child), false);
        K childkey;
        child->GetAttribute(comparisionAttribute, &childkey);

        auto it = map.find(childkey);
        V* value = (it != map.end() ? it->second : nullptr);

        try {
            deserialize(itemKey, value);
            map[childkey] = value;
        } catch (...) {
            handleError(IVW_CONTEXT);
        }
    }
}

// unique_ptr specialization
template <typename K, typename V, typename C, typename A>
void Deserializer::deserialize(const std::string& key, std::map<K, std::unique_ptr<V>, C, A>& map,
                               const std::string& itemKey,
                               const std::string& comparisionAttribute) {
    if (!isPrimitiveType(typeid(K)) || comparisionAttribute.empty())
        throw SerializationException("Error: map key has to be a primitive type", IVW_CONTEXT);

    NodeSwitch mapNodeSwitch(*this, key);
    if (!mapNodeSwitch) return;
    TxEIt child(itemKey);

    for (child = child.begin(rootElement_); child != child.end(); ++child) {
        // In the next deserialization call do net fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, &(*child), false);
        K childkey;
        child->GetAttribute(comparisionAttribute, &childkey);

        auto it = map.find(childkey);
        if (it != map.end()) {
            try {
                if (auto ptr = it->second.get()) {
                    deserialize(itemKey, ptr);
                } else {
                    deserialize(itemKey, ptr);
                    it->second.reset(ptr);
                }
            } catch (...) {
                handleError(IVW_CONTEXT);
            }
        } else {
            V* ptr = nullptr;
            try {
                deserialize(itemKey, ptr);
                map.emplace(childkey, std::unique_ptr<V>(ptr));
            } catch (...) {
                delete ptr;
                handleError(IVW_CONTEXT);
            }
        }
    }
}

template <class T>
void Deserializer::deserialize(const std::string& key, T*& data) {
    auto keyNode = retrieveChild_ ? rootElement_->FirstChildElement(key, false) : rootElement_;
    if (!keyNode) return;

    const std::string type_attr(keyNode->GetAttribute(SerializeConstants::TypeAttribute));
    const std::string ref_attr(keyNode->GetAttribute(SerializeConstants::RefAttribute));
    const std::string id_attr(keyNode->GetAttribute(SerializeConstants::IDAttribute));

    if (!data) {
        if (allowRef_ && !ref_attr.empty()) {
            // Has reference identifier, data should already be allocated and we only have to find
            // and set the pointer to it.
            data = static_cast<T*>(refDataContainer_.find(type_attr, ref_attr));
            if (!data) {
                auto it = referenceLookup_.find(ref_attr);
                if (it != referenceLookup_.end()) {
                    NodeDebugger error(keyNode);
                    throw SerializationException(
                        "Reference to " + error.toString(0) + " not instantiated", IVW_CONTEXT,
                        error[0].key, error[0].type, error[0].identifier, it->second);
                } else {
                    throw SerializationException(
                        "Could not find reference to " + key + ": " + type_attr, IVW_CONTEXT, key,
                        type_attr);
                }
            }
            return;

        } else if (!type_attr.empty()) {
            try {
                data = getRegisteredType<T>(type_attr);
            } catch (Exception& e) {
                NodeDebugger error(keyNode);
                throw SerializationException(
                    "Error trying to create " + error.toString(0) + ". Reason: " + e.getMessage(),
                    e.getContext(), key, type_attr, error[0].identifier, keyNode);
            }
            if (!data) {
                NodeDebugger error(keyNode);
                throw SerializationException(
                    "Could not create " + error.toString(0) + ". Reason: \"" + type_attr +
                        "\" Not found in factory.",
                    IVW_CONTEXT, key, type_attr, error[0].identifier, keyNode);
            }

        } else {
            try {
                data = getNonRegisteredType<T>();
            } catch (Exception& e) {
                NodeDebugger error(keyNode);
                throw SerializationException(
                    "Error trying to create " + error.toString(0) + ". Reason: " + e.getMessage(),
                    e.getContext(), key, type_attr, error[0].identifier, keyNode);
            }
            if (!data) {
                NodeDebugger error(keyNode);
                throw SerializationException("Could not create " + error.toString(0) +
                                                 ". Reason: No default constructor found.",
                                             IVW_CONTEXT, key, type_attr, error[0].identifier,
                                             keyNode);
            }
        }
    }

    if (data) {
        deserialize(key, *data);
        if (allowRef_ && !id_attr.empty()) refDataContainer_.insert(data, keyNode);
    }
}

template <class T, class D>
void Deserializer::deserialize(const std::string& key, std::unique_ptr<T, D>& data) {
    if (auto ptr = data.get()) {
        deserialize(key, ptr);
    } else {
        deserialize(key, ptr);
        data.reset(ptr);
    }
}

template <typename T, typename K>
void Deserializer::deserialize(const std::string& key, ContainerWrapper<T, K>& container) {
    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;
    unsigned int i = 0;
    TxEIt child(container.getItemKey());

    for (child = child.begin(rootElement_); child != child.end(); ++child) {
        // In the next deserialization call do net fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, &(*child), false);
        container.deserialize(*this, &(*child), i);
        i++;
    }
}

template <class Base, class T>
void Deserializer::deserializeAs(const std::string& key, T*& data) {
    static_assert(std::is_base_of<Base, T>::value, "T should be derived from Base");

    if (Base* ptr = data) {
        deserialize(key, ptr);
    } else {
        deserialize(key, ptr);
        if (auto typeptr = dynamic_cast<T*>(ptr)) {
            data = typeptr;
        } else {
            delete ptr;
            throw SerializationException(
                "Could not deserialize \"" + key + "\" types does not match", IVW_CONTEXT);
        }
    }
}

template <class Base, class T, class D>
void Deserializer::deserializeAs(const std::string& key, std::unique_ptr<T, D>& data) {
    static_assert(std::is_base_of<Base, T>::value, "T should be derived from Base");

    if (Base* ptr = data.get()) {
        deserialize(key, ptr);
    } else {
        deserialize(key, ptr);
        if (auto typeptr = dynamic_cast<T*>(ptr)) {
            data.reset(typeptr);
        } else {
            delete ptr;
            throw SerializationException(
                "Could not deserialize \"" + key + "\" types does not match", IVW_CONTEXT);
        }
    }
}

}  // namespace inviwo
#endif
