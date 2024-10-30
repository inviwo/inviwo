/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <inviwo/core/io/serialization/serializebase.h>
#include <inviwo/core/io/serialization/nodedebugger.h>
#include <inviwo/core/io/serialization/serializationexception.h>
#include <inviwo/core/io/serialization/serializeconstants.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/factory.h>
#include <inviwo/core/util/logfilter.h>
#include <inviwo/core/util/typetraits.h>
#include <inviwo/core/util/detected.h>
#include <inviwo/core/util/glmutils.h>

#include <flags/flags.h>

#include <type_traits>
#include <list>
#include <istream>
#include <bitset>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <filesystem>
#include <optional>

#include <fmt/core.h>

namespace inviwo {

class Serializable;
class Deserializer;
class VersionConverter;
class InviwoApplication;

template <typename T>
struct ContainerWrapperItem {
    bool doDeserialize;
    T& value;
    std::function<void(T&)> callback;
};

/**
 * \class ContainerWrapper
 */
template <typename T, typename ValueGetter, typename IdGetter>
    requires std::is_invocable_v<IdGetter, TiXmlElement&> &&
             std::is_invocable_r_v<ContainerWrapperItem<T>, ValueGetter,
                                   std::invoke_result_t<IdGetter, TiXmlElement&>, size_t>
class ContainerWrapper {
public:
    ContainerWrapper(std::string_view itemKey, ValueGetter getItem, IdGetter idGetter)
        : idGetter_{std::move(idGetter)}, valueGetter_(std::move(getItem)), itemKey_(itemKey) {}

    std::string_view getItemKey() const { return itemKey_; }

    void deserialize(Deserializer& d, TiXmlElement& node, size_t ind);

private:
    IdGetter idGetter_;
    ValueGetter valueGetter_;
    std::string itemKey_;
};

class IVW_CORE_API Deserializer : public SerializeBase, public LogFilter {
public:
    /**
     * \brief Deserialize content from a file
     * @param fileName path to file that is to be deserialized.
     */
    Deserializer(const std::filesystem::path& fileName, allocator_type alloc = {});

    /**
     * \brief Deserialize content from a stream.
     * @param stream Stream with content that is to be deserialized.
     * @param refPath Used to calculate paths relative to the stream source if any.
     */
    Deserializer(std::istream& stream, const std::filesystem::path& refPath,
                 allocator_type alloc = {});

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
    void deserialize(std::string_view key, std::vector<T*>& sVector,
                     std::string_view itemKey = "item");

    template <typename T, typename C>
    void deserialize(std::string_view key, std::vector<T*>& sVector, std::string_view itemKey,
                     C identifier);

    template <typename T, typename Alloc>
    void deserialize(std::string_view key, std::vector<T, Alloc>& sVector,
                     std::string_view itemKey = "item");

    template <typename T>
    void deserialize(std::string_view key, std::unordered_set<T>& sSet,
                     std::string_view itemKey = "item");

    template <typename T>
    void deserialize(std::string_view key, std::vector<std::unique_ptr<T>>& vector,
                     std::string_view itemKey = "item");

    template <typename T>
    void deserialize(std::string_view key, std::list<T>& sContainer,
                     std::string_view itemKey = "item");

    template <typename T, size_t N>
    void deserialize(std::string_view key, std::array<T, N>& sContainer,
                     std::string_view itemKey = "item");

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
     * param comparisonAttribute  = "identifier"
     * param sMap["enableMIP"]     = address of a property
     *       sMap["enableShading"] = address of a property
     *         where, "enableMIP" & "enableShading" are keys.
     *         address of a property is a value
     *
     * \note If children has attribute "type", then comparisonAttribute becomes meaningless.
     *       Because deserializer always allocates a new instance of type using registered
     *       factories. eg.,
     *       \code{.xml}
     *           <Processor type="EntryExitPoints" identifier="EntryExitPoints" reference="ref2" />
     *       \endcode
     *
     * @param key Map key or parent node of itemKey.
     * @param sMap  map to be deserialized - source / input map.
     * @param itemKey map item key of children nodes.
     * @param comparisonAttribute forced comparison attribute.
     */
    template <typename K, typename V, typename C, typename A>
    void deserialize(std::string_view key, std::map<K, V, C, A>& sMap,
                     std::string_view itemKey = "item",
                     std::string_view comparisonAttribute = SerializeConstants::KeyAttribute);

    template <typename K, typename V, typename C, typename A>
    void deserialize(std::string_view key, std::map<K, V*, C, A>& sMap,
                     std::string_view itemKey = "item",
                     std::string_view comparisonAttribute = SerializeConstants::KeyAttribute);

    template <typename K, typename V, typename C, typename A>
    void deserialize(std::string_view key, std::map<K, std::unique_ptr<V>, C, A>& sMap,
                     std::string_view itemKey = "item",
                     std::string_view comparisonAttribute = SerializeConstants::KeyAttribute);

    template <typename K, typename V, typename H, typename C, typename A>
    void deserialize(std::string_view key, std::unordered_map<K, V, H, C, A>& map,
                     std::string_view itemKey = "item",
                     std::string_view comparisonAttribute = SerializeConstants::KeyAttribute);

    template <typename T, typename ValueGetter, typename IdGetter>
    void deserialize(std::string_view key, ContainerWrapper<T, ValueGetter, IdGetter>& container);

    // Specializations for chars
    void deserialize(std::string_view key, signed char& data,
                     const SerializationTarget& target = SerializationTarget::Node);
    void deserialize(std::string_view key, char& data,
                     const SerializationTarget& target = SerializationTarget::Node);
    void deserialize(std::string_view key, unsigned char& data,
                     const SerializationTarget& target = SerializationTarget::Node);

    // integers, strings, reals
    template <typename T, typename std::enable_if<std::is_integral<T>::value ||
                                                      util::is_floating_point<T>::value ||
                                                      util::is_string<T>::value,
                                                  int>::type = 0>
    void deserialize(std::string_view key, T& data,
                     const SerializationTarget& target = SerializationTarget::Node);

    // Enum types
    template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
    void deserialize(std::string_view key, T& data,
                     const SerializationTarget& target = SerializationTarget::Node);

    // Flag types
    template <typename T>
    void deserialize(std::string_view key, flags::flags<T>& data,
                     const SerializationTarget& target = SerializationTarget::Node);

    // glm vector types
    template <typename Vec, typename std::enable_if<util::rank<Vec>::value == 1, int>::type = 0>
    void deserialize(std::string_view key, Vec& data);

    // glm matrix types
    template <typename Mat, typename std::enable_if<util::rank<Mat>::value == 2, int>::type = 0>
    void deserialize(std::string_view key, Mat& data);

    // bitsets
    template <size_t N>
    void deserialize(std::string_view key, std::bitset<N>& bits);

    // path
    void deserialize(std::string_view key, std::filesystem::path& path,
                     const SerializationTarget& target = SerializationTarget::Node);
    /**
     * \brief  Deserialize any Serializable object
     */
    void deserialize(std::string_view key, Serializable& sObj);
    /**
     * \brief  Deserialize pointer data of type T, which is of type
     *         serializable object or primitive data
     */
    template <class T>
    void deserialize(std::string_view key, T*& data);
    template <class Base, class T>
    void deserializeAs(std::string_view key, T*& data);

    template <class T>
    void deserialize(std::string_view key, std::unique_ptr<T>& data);

    template <class T>
    void deserialize(std::string_view key, std::shared_ptr<T>& data);

    template <class Base, class T>
    void deserializeAs(std::string_view key, std::unique_ptr<T>& data);

    template <typename T>
    using HasDeserialize = decltype(std::declval<T>().deserialize(std::declval<Deserializer&>()));
    template <typename T,
              typename = std::enable_if_t<util::is_detected_exact_v<void, HasDeserialize, T>>>
    void deserialize(std::string_view key, T& sObj);

    using ExceptionHandler = std::function<void(ExceptionContext)>;
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
    T* getRegisteredType(std::string_view className);

    /**
     * \brief For allocating objects that do not belong to any registered factories.
     *
     * @return T* Pointer to object of type T.
     */
    template <typename T>
    T* getNonRegisteredType();

    friend class NodeSwitch;

    template <typename T, typename ValueGetter, typename IdGetter>
        requires std::is_invocable_v<IdGetter, TiXmlElement&> &&
                 std::is_invocable_r_v<ContainerWrapperItem<T>, ValueGetter,
                                       std::invoke_result_t<IdGetter, TiXmlElement&>, size_t>
    friend class ContainerWrapper;

    void registerFactory(FactoryBase* factory);

    int getInviwoWorkspaceVersion() const;

private:
    TiXmlElement* retrieveChild(std::string_view key);

    ExceptionHandler exceptionHandler_;
    std::vector<FactoryBase*> registeredFactories_;

    int inviwoWorkspaceVersion_ = 0;
};

namespace detail {

template <typename T>
using isDeserializable = decltype(std::declval<Deserializer>().deserialize(
    std::declval<std::string_view>(), std::declval<T&>()));

template <typename T>
using isStreamable = decltype(std::declval<std::istream&>() >> std::declval<T&>());

template <typename T>
constexpr bool canDeserialize() {
    return util::is_detected_exact_v<std::istream&, isStreamable, T> ||
           util::is_detected_v<isDeserializable, T> || std::is_enum_v<T>;
}

IVW_CORE_API std::string_view getAttribute(TiXmlElement& node, std::string_view key);
IVW_CORE_API std::optional<std::string_view> attribute(TiXmlElement* node, std::string_view key);

template <typename T>
void getNodeAttribute(TiXmlElement* node, std::string_view key, T& dest) {
    if (const auto str = attribute(node, key)) {
        detail::fromStr(*str, dest);
    }
}
template <typename T>
void getNodeAttribute(TiXmlElement& node, std::string_view key, T& dest) {
    if (const auto str = attribute(&node, key)) {
        detail::fromStr(*str, dest);
    }
}

IVW_CORE_API void forEachChild(TiXmlElement* node, std::string_view key,
                               const std::function<void(TiXmlElement&)>& func);

}  // namespace detail

template <typename T, typename ValueGetter, typename IdGetter>
    requires std::is_invocable_v<IdGetter, TiXmlElement&> &&
             std::is_invocable_r_v<ContainerWrapperItem<T>, ValueGetter,
                                   std::invoke_result_t<IdGetter, TiXmlElement&>, size_t>
void ContainerWrapper<T, ValueGetter, IdGetter>::deserialize(Deserializer& d, TiXmlElement& node,
                                                             size_t ind) {
    auto item = valueGetter_(idGetter_(node), ind);
    if (item.doDeserialize) {
        try {
            d.deserialize(itemKey_, item.value);
            item.callback(item.value);
        } catch (...) {
            d.handleError(IVW_CONTEXT);
        }
    }
}

namespace util {

constexpr auto defaultIdGetter = [](TiXmlElement& node) -> std::string_view {
    return inviwo::detail::getAttribute(node, "identifier");
};

template <typename T, typename ValueGetter, typename IdGetter = decltype(defaultIdGetter)>
    requires std::is_invocable_v<IdGetter, TiXmlElement&> &&
             std::is_invocable_r_v<ContainerWrapperItem<T>, ValueGetter,
                                   std::invoke_result_t<IdGetter, TiXmlElement&>, size_t>
auto makeContainerWrapper(std::string_view key, ValueGetter valueGetter,
                          IdGetter idGetter = defaultIdGetter) {
    return ContainerWrapper<T, ValueGetter, IdGetter>{key, std::move(valueGetter),
                                                      std::move(idGetter)};
}

template <typename T>
using classIdentifierType = decltype(std::declval<T>().getClassIdentifier());

template <class T>
using HasGetClassIdentifier = is_detected_exact<std::string, classIdentifierType, T>;

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
    IndexedDeserializer(std::string_view key, std::string_view itemKey)
        : key_(key), itemKey_(itemKey) {}

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
        size_t count = 0;
        T tmp{};
        auto cont = util::makeContainerWrapper<T>(
            itemKey_, [&](std::string_view, size_t ind) -> ContainerWrapperItem<T> {
                if (ind < container.size()) {
                    return {true, container[ind], [&](T&) { ++count; }};
                } else {
                    tmp = makeNewItem_();
                    return {true, tmp, [&](T& value) {
                                container.emplace_back(std::move(value));
                                ++count;
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
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    std::function<T()> makeNewItem_ = []() -> T { return T{}; };
    std::function<void(T&)> onNewItem_ = [](T&) {};
    std::function<void(T&)> onRemoveItem_ = [](T&) {};
#endif

    std::string key_;
    std::string itemKey_;
};

template <typename K, typename T>
class IdentifiedDeserializer {
public:
    IdentifiedDeserializer(std::string_view key, std::string_view itemKey)
        : key_(key), itemKey_(itemKey) {}

    IdentifiedDeserializer<K, T>& setGetId(std::function<const K&(const T&)> getID) {
        getID_ = getID;
        return *this;
    }
    IdentifiedDeserializer<K, T>& setMakeNew(std::function<T()> makeNewItem) {
        makeNewItem_ = makeNewItem;
        return *this;
    }
    IdentifiedDeserializer<K, T>& setNewFilter(
        std::function<bool(std::string_view, size_t)> filter) {
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
    IdentifiedDeserializer<K, T>& onMove(std::function<void(T&, size_t)> onMoveItem) {
        move_ = onMoveItem;
        return *this;
    }

    template <typename C>
    void operator()(Deserializer& d, C& container) {
        T tmp{};
        auto toRemove = util::transform(container, [&](const T& x) -> K { return getID_(x); });
        std::vector<K> foundIdentifiers;

        auto cont = util::makeContainerWrapper<T>(
            itemKey_, [&](std::string_view identifier, size_t ind) -> ContainerWrapperItem<T> {
                std::erase(toRemove, identifier);
                foundIdentifiers.emplace_back(identifier);
                auto it =
                    util::find_if(container, [&](const T& i) { return getID_(i) == identifier; });
                if (it != container.end()) {
                    return {true, *it, [&](T&) {}};
                } else {
                    tmp = makeNewItem_();
                    return {filter_(identifier, ind), tmp,
                            [this, ind](T& val) { onNewItem_(val, ind); }};
                }
            });

        d.deserialize(key_, cont);
        for (auto& id : toRemove) onRemoveItem_(id);

        reorder(container, foundIdentifiers);
    }

private:
    template <typename C>
    void reorder(C& list, const std::vector<K>& order) {
        size_t dst = 0;
        for (size_t i = 0; i < order.size() && dst < list.size(); ++i) {
            if (order[i] == getID_(list[dst])) {
                ++dst;
                continue;
            }

            while (dst < list.size() &&
                   std::find(order.begin() + i, order.end(), getID_(list[dst])) == order.end()) {
                ++dst;
            }

            if (auto it = std::find_if(list.begin() + dst, list.end(),
                                       [&](const T& item) { return getID_(item) == order[i]; });
                it != list.end() && it != list.begin() + dst) {
                move_(*it, dst);
            }
            ++dst;
        }
    }

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    std::function<const K&(const T&)> getID_ = [](const T&) -> const K& {
        throw Exception("IdentifiedDeserializer: GetID callback is not set!");
    };
    std::function<T()> makeNewItem_ = []() -> T {
        throw Exception("IdentifiedDeserializer: MakeNew callback is not set!");
    };
    std::function<void(T&, size_t)> onNewItem_ = [](T&, size_t) {
        throw Exception("IdentifiedDeserializer: OnNew callback is not set!");
    };
    std::function<void(const K&)> onRemoveItem_ = [](const K&) {
        throw Exception("IdentifiedDeserializer: OnRemove callback is not set!");
    };
    std::function<void(T&, size_t)> move_ = [](T&, size_t) {
        throw Exception("IdentifiedDeserializer: OnMove callback is not set!");
    };

    std::function<bool(std::string_view id, size_t ind)> filter_ =
        [](std::string_view /*id*/, size_t /*ind*/) { return true; };

#endif

    std::string key_;
    std::string itemKey_;
};

template <typename K, typename T>
class MapDeserializer {
public:
    MapDeserializer(std::string_view key, std::string_view itemKey,
                    std::string_view attributeKey = SerializeConstants::KeyAttribute)
        : key_(key), itemKey_(itemKey), attributeKey_(attributeKey) {

        if constexpr (std::is_same_v<K, std::string_view>) {
            identifierTransform_ = [](std::string_view identifier) -> K { return identifier; };
        } else {
            identifierTransform_ = [](std::string_view) -> K {
                throw Exception("MapDeserializer: setIdentifierTransform callback is not set!");
            };
        }
    }

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
    MapDeserializer<K, T>& setIdentifierTransform(
        std::function<K(std::string_view)> identifierTransform) {
        identifierTransform_ = identifierTransform;
        return *this;
    }

    template <typename C>
    void operator()(Deserializer& d, C& container) {
        T tmp{};
        auto toRemove = util::transform(
            container, [](const std::pair<const K, T>& item) { return item.first; });
        auto cont = util::makeContainerWrapper<T>(
            itemKey_,
            [&](const K& id, size_t ind) -> ContainerWrapperItem<T> {
                std::erase(toRemove, id);
                auto it = container.find(id);
                if (it != container.end()) {
                    return {true, it->second, [&](T& /*val*/) {}};
                } else {
                    tmp = makeNewItem_();
                    return {filter_(id, ind), tmp, [&, id](T& val) { onNewItem_(id, val); }};
                }
            },
            [&](TiXmlElement& node) -> decltype(auto) {
                return identifierTransform_(::inviwo::detail::getAttribute(node, attributeKey_));
            });

        d.deserialize(key_, cont);

        for (auto& id : toRemove) onRemoveItem_(id);
    }

private:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    std::function<T()> makeNewItem_ = []() -> T {
        throw Exception("MapDeserializer: MakeNew callback is not set!");
    };
    std::function<void(const K&, T&)> onNewItem_ = [](const K&, T&) {
        throw Exception("MapDeserializer: OnNew callback is not set!");
    };
    std::function<void(const K&)> onRemoveItem_ = [](const K&) {
        throw Exception("MapDeserializer: OnRemove callback is not set!");
    };
    std::function<bool(const K& id, size_t ind)> filter_ = [](const K& /*id*/, size_t /*ind*/) {
        return true;
    };
    std::function<K(std::string_view)> identifierTransform_;
#endif

    const std::string key_;
    const std::string itemKey_;
    const std::string attributeKey_;
};

}  // namespace util

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

// integers, strings, reals
template <typename T,
          typename std::enable_if<std::is_integral<T>::value || util::is_floating_point<T>::value ||
                                      util::is_string<T>::value,
                                  int>::type>
void Deserializer::deserialize(std::string_view key, T& data, const SerializationTarget& target) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    try {
        if (target == SerializationTarget::Attribute) {
            detail::getNodeAttribute(rootElement_, key, data);
        } else if (NodeSwitch ns{*this, key}) {
            detail::getNodeAttribute(rootElement_, SerializeConstants::ContentAttribute, data);
        }
    } catch (...) {
        handleError(IVW_CONTEXT);
    }
}

// enum types
template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type>
void Deserializer::deserialize(std::string_view key, T& data, const SerializationTarget& target) {
    using ET = typename std::underlying_type<T>::type;
    ET tmpdata{static_cast<ET>(data)};
    deserialize(key, tmpdata, target);
    data = static_cast<T>(tmpdata);
}

// Flag types
template <typename T>
void Deserializer::deserialize(std::string_view key, flags::flags<T>& data,
                               const SerializationTarget& target) {

    auto tmp = data.underlying_value();
    deserialize(key, tmp, target);
    data.set_underlying_value(tmp);
}

// glm vector types
template <typename Vec, typename std::enable_if<util::rank<Vec>::value == 1, int>::type>
void Deserializer::deserialize(std::string_view key, Vec& data) {
    if (NodeSwitch ns{*this, key}) {
        for (size_t i = 0; i < util::extent<Vec, 0>::value; ++i) {
            try {
                detail::getNodeAttribute(rootElement_, SerializeConstants::VectorAttributes[i],
                                         data[i]);
            } catch (...) {
                handleError(IVW_CONTEXT);
            }
        }
    }
}

// glm matrix types
template <typename Mat, typename std::enable_if<util::rank<Mat>::value == 2, int>::type>
void Deserializer::deserialize(std::string_view key, Mat& data) {
    constexpr std::array<std::string_view, 4> rows{"row0", "row1", "row2", "row3"};
    constexpr std::array<std::string_view, 4> cols{"col0", "col1", "col2", "col3"};

    if (NodeSwitch ns{*this, key}) {
        for (size_t i = 0; i < util::extent<Mat, 0>::value; ++i) {
            // Deserialization of row needs to be here for backwards compatibility,
            // we used to incorrectly serialize columns as row
            deserialize(rows[i], data[i]);
            deserialize(cols[i], data[i]);
        }
    }
}

template <size_t N>
void Deserializer::deserialize(std::string_view key, std::bitset<N>& bits) {
    std::string value = bits.to_string();
    deserialize(key, value);
    bits = std::bitset<N>(value);
}

template <typename T>
void Deserializer::deserialize(std::string_view key, std::vector<T*>& vector,
                               std::string_view itemKey) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;

    size_t i = 0;
    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        // In the next deserialization call do not fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, child, false);
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
    });
}

template <typename T>
void Deserializer::deserialize(std::string_view key, std::vector<std::unique_ptr<T>>& vector,
                               std::string_view itemKey) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;

    size_t i = 0;
    detail::forEachChild(rootElement_, itemKey,
                         [&](TiXmlElement& child) {  // In the next deserialization call do not
                                                     // fetch the "child" since we are looping...
                             // hence the "false" as the last arg.
                             NodeSwitch elementNodeSwitch(*this, child, false);
                             try {
                                 if (i < vector.size()) {
                                     deserialize(itemKey, vector[i]);
                                 } else {
                                     std::unique_ptr<T> item;
                                     deserialize(itemKey, item);
                                     vector.emplace_back(std::move(item));
                                 }
                             } catch (...) {
                                 handleError(IVW_CONTEXT);
                             }
                             i++;
                         });
}

template <typename T, typename C>
void Deserializer::deserialize(std::string_view key, std::vector<T*>& vector,
                               std::string_view itemKey, C identifier) {

    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;

    auto lastInsertion = vector.begin();

    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        identifier.setKey(child);
        auto it = std::find_if(vector.begin(), vector.end(), identifier);

        if (it != vector.end()) {  // There is a item in vector with same identifier as on disk
            NodeSwitch elementNodeSwitch(*this, child, false);
            try {
                deserialize(itemKey, *it);
                lastInsertion = it;
            } catch (...) {
                handleError(IVW_CONTEXT);
            }
        } else {  // No item in vector matches item on disk, create a new one.
            T* item = nullptr;
            NodeSwitch elementNodeSwitch(*this, child, false);
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
    });
}

template <typename T, typename Alloc>
void Deserializer::deserialize(std::string_view key, std::vector<T, Alloc>& vector,
                               std::string_view itemKey) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;

    size_t i = 0;
    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        // In the next deserialization call do not fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, child, false);
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
    });
}

template <typename T>
void Deserializer::deserialize(std::string_view key, std::unordered_set<T>& set,
                               std::string_view itemKey) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;

    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        // In the next deserialization call do not fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, child, false);
        try {
            T item;
            deserialize(itemKey, item);
            set.insert(std::move(item));

        } catch (...) {
            handleError(IVW_CONTEXT);
        }
    });
}

template <typename T>
void Deserializer::deserialize(std::string_view key, std::list<T>& container,
                               std::string_view itemKey) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;
    size_t i = 0;
    detail::forEachChild(rootElement_, itemKey,
                         [&](TiXmlElement& child) {  // In the next deserialization call do not
                                                     // fetch the "child" since we are looping...
                             // hence the "false" as the last arg.
                             NodeSwitch elementNodeSwitch(*this, child, false);
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
                         });
}

template <typename T, size_t N>
void Deserializer::deserialize(std::string_view key, std::array<T, N>& cont,
                               std::string_view itemKey) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;

    size_t i = 0;
    detail::forEachChild(rootElement_, itemKey,
                         [&](TiXmlElement& child) {  // In the next deserialization call do not
                                                     // fetch the "child" since we are looping...
                             // hence the "false" as the last arg.
                             NodeSwitch elementNodeSwitch(*this, child, false);
                             try {
                                 if (i < cont.size()) {
                                     deserialize(itemKey, cont[i]);
                                 } else {
                                     throw SerializationException(
                                         "To many elements found for std::array", IVW_CONTEXT, key);
                                 }
                             } catch (...) {
                                 handleError(IVW_CONTEXT);
                             }
                             i++;
                         });
}

template <typename K, typename V, typename C, typename A>
void Deserializer::deserialize(std::string_view key, std::map<K, V, C, A>& map,
                               std::string_view itemKey, std::string_view comparisonAttribute) {
    static_assert(isPrimitiveType<K>(), "Error: map key has to be a primitive type");
    static_assert(detail::canDeserialize<V>(), "Type is not serializable");

    NodeSwitch mapNodeSwitch(*this, key);
    if (!mapNodeSwitch) return;

    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        // In the next deserialization call do not fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, child, false);
        K childkey;
        detail::getNodeAttribute(child, comparisonAttribute, childkey);

        V value;
        auto it = map.find(childkey);
        if (it != map.end()) value = it->second;
        try {
            deserialize(itemKey, value);
            map[childkey] = value;
        } catch (...) {
            handleError(IVW_CONTEXT);
        }
    });
}

// Pointer specialization
template <typename K, typename V, typename C, typename A>
void Deserializer::deserialize(std::string_view key, std::map<K, V*, C, A>& map,
                               std::string_view itemKey, std::string_view comparisonAttribute) {
    static_assert(isPrimitiveType<K>(), "Error: map key has to be a primitive type");
    static_assert(detail::canDeserialize<V>(), "Type is not serializable");

    NodeSwitch mapNodeSwitch(*this, key);
    if (!mapNodeSwitch) return;

    detail::forEachChild(rootElement_, itemKey,
                         [&](TiXmlElement& child) {  // In the next deserialization call do not
                                                     // fetch the "child" since we are looping...
                             // hence the "false" as the last arg.
                             NodeSwitch elementNodeSwitch(*this, child, false);
                             K childkey;
                             detail::getNodeAttribute(child, comparisonAttribute, childkey);

                             auto it = map.find(childkey);
                             V* value = (it != map.end() ? it->second : nullptr);

                             try {
                                 deserialize(itemKey, value);
                                 map[childkey] = value;
                             } catch (...) {
                                 handleError(IVW_CONTEXT);
                             }
                         });
}

// unique_ptr specialization
template <typename K, typename V, typename C, typename A>
void Deserializer::deserialize(std::string_view key, std::map<K, std::unique_ptr<V>, C, A>& map,
                               std::string_view itemKey, std::string_view comparisonAttribute) {
    static_assert(isPrimitiveType<K>(), "Error: map key has to be a primitive type");
    static_assert(detail::canDeserialize<V>(), "Type is not serializable");

    NodeSwitch mapNodeSwitch(*this, key);
    if (!mapNodeSwitch) return;

    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        // In the next deserialization call do not fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, child, false);
        K childkey{};
        detail::getNodeAttribute(child, comparisonAttribute, childkey);

        const auto it = map.find(childkey);
        try {
            if (it != map.end()) {
                deserialize(itemKey, it->second);
            } else {
                std::unique_ptr<V> item;
                deserialize(itemKey, item);
                map.emplace(childkey, std::move(item));
            }
        } catch (...) {
            handleError(IVW_CONTEXT);
        }
    });
}

template <typename K, typename V, typename H, typename C, typename A>
void Deserializer::deserialize(std::string_view key, std::unordered_map<K, V, H, C, A>& map,
                               std::string_view itemKey, std::string_view comparisonAttribute) {
    static_assert(isPrimitiveType<K>(), "Error: map key has to be a primitive type");
    static_assert(detail::canDeserialize<V>(), "Type is not serializable");

    NodeSwitch mapNodeSwitch(*this, key);
    if (!mapNodeSwitch) return;

    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        // In the next deserialization call do not fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, child, false);
        K childKey;
        detail::getNodeAttribute(child, comparisonAttribute, childKey);

        V value;
        auto it = map.find(childKey);
        if (it != map.end()) value = it->second;
        try {
            deserialize(itemKey, value);
            map[childKey] = value;
        } catch (...) {
            handleError(IVW_CONTEXT);
        }
    });
}

template <typename T>
T* Deserializer::getRegisteredType(std::string_view className) {
    for (auto base : registeredFactories_) {
        if (base->hasKey(className)) {
            if (auto factory = dynamic_cast<Factory<T, std::string_view>*>(base)) {
                if (auto data = factory->create(className)) return data.release();
            }
        }
    }
    return nullptr;
}

template <typename T>
T* Deserializer::getNonRegisteredType() {
    return util::defaultConstructType<T>();
}

template <class T>
void Deserializer::deserialize(std::string_view key, T*& data) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    auto keyNode = retrieveChild(key);
    if (!keyNode) return;

    const auto typeAttr = detail::attribute(keyNode, SerializeConstants::TypeAttribute);

    if (!data && typeAttr) {
        try {
            data = getRegisteredType<T>(*typeAttr);
        } catch (Exception& e) {
            NodeDebugger error(keyNode);
            throw SerializationException(
                "Error trying to create " + error.toString(0) + ". Reason:\n" + e.getMessage(),
                e.getContext(), key, *typeAttr, error[0].identifier, keyNode);
        }
        if (!data) {
            NodeDebugger error(keyNode);
            throw SerializationException("Could not create " + error.toString(0) + ". Reason: \"" +
                                             std::string{*typeAttr} + "\" Not found in factory.",
                                         IVW_CONTEXT, key, *typeAttr, error[0].identifier, keyNode);
        }
    } else if (!data) {
        try {
            data = getNonRegisteredType<T>();
        } catch (Exception& e) {
            NodeDebugger error(keyNode);
            throw SerializationException(
                "Error trying to create " + error.toString(0) + ". Reason:\n" + e.getMessage(),
                e.getContext(), key, "", error[0].identifier, keyNode);
        }
        if (!data) {
            NodeDebugger error(keyNode);
            throw SerializationException(
                "Could not create " + error.toString(0) + ". Reason: No default constructor found.",
                IVW_CONTEXT, key, "", error[0].identifier, keyNode);
        }
    }

    if (data) {
        deserialize(key, *data);
    }
}

template <class T>
void Deserializer::deserialize(std::string_view key, std::unique_ptr<T>& data) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    if constexpr (util::HasGetClassIdentifier<T>::value) {
        if (auto keyNode = retrieveChild(key)) {
            const auto typeAttr = detail::attribute(keyNode, SerializeConstants::TypeAttribute);
            if (data && typeAttr && *typeAttr != data->getClassIdentifier()) {
                // object has wrong type, delete it and let the deserialization create a new object
                // with the correct type
                data.reset();
            }
        }
    }

    if (data) {
        deserialize(key, *data);
    } else {
        T* ptr = nullptr;
        deserialize(key, ptr);
        data.reset(ptr);
    }
}

template <class T>
void Deserializer::deserialize(std::string_view key, std::shared_ptr<T>& data) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    auto keyNode = retrieveChild(key);
    if (!keyNode) return;

    const auto typeId = detail::attribute(keyNode, SerializeConstants::TypeAttribute);

    if constexpr (util::HasGetClassIdentifier<T>::value) {
        if (data && typeId && *typeId != data->getClassIdentifier()) {
            // object has wrong type, delete it and let the deserialization create a new object
            // with the correct type
            data.reset();
        }
    }

    if (!data) {
        if (!typeId) {
            try {
                data.reset(getNonRegisteredType<T>());
            } catch (Exception& e) {
                NodeDebugger error(keyNode);
                throw SerializationException(
                    "Error trying to create " + error.toString(0) + ". Reason:\n" + e.getMessage(),
                    e.getContext(), key, "", error[0].identifier, keyNode);
            }
            if (!data) {
                NodeDebugger error(keyNode);
                throw SerializationException("Could not create " + error.toString(0) +
                                                 ". Reason: No default constructor found.",
                                             IVW_CONTEXT, key, "", error[0].identifier, keyNode);
            }
        } else {
            try {
                for (auto base : registeredFactories_) {
                    if (base->hasKey(*typeId)) {
                        if (auto factory = dynamic_cast<Factory<T, std::string_view>*>(base)) {
                            if ((data = factory->createShared(*typeId))) {
                                break;
                            }
                        }
                    }
                }
            } catch (Exception& e) {
                NodeDebugger error(keyNode);
                throw SerializationException(
                    "Error trying to create " + error.toString(0) + ". Reason:\n" + e.getMessage(),
                    e.getContext(), key, *typeId, error[0].identifier, keyNode);
            }
            if (!data) {
                NodeDebugger error(keyNode);
                throw SerializationException(
                    "Could not create " + error.toString(0) + ". Reason: \"" +
                        std::string{*typeId} + "\" Not found in factory.",
                    IVW_CONTEXT, key, *typeId, error[0].identifier, keyNode);
            }
        }
    }

    if (data) {
        deserialize(key, *data);
    }
}

template <typename T, typename ValueGetter, typename IdGetter>
void Deserializer::deserialize(std::string_view key,
                               ContainerWrapper<T, ValueGetter, IdGetter>& container) {
    NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) return;
    size_t i = 0;

    detail::forEachChild(rootElement_, container.getItemKey(), [&](TiXmlElement& child) {
        // In the next deserialization call do not fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, child, false);
        container.deserialize(*this, child, i);
        i++;
    });
}

template <class Base, class T>
void Deserializer::deserializeAs(std::string_view key, T*& data) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

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
                fmt::format("Could not deserialize \"{}\" types does not match", key), IVW_CONTEXT);
        }
    }
}

template <class Base, class T>
void Deserializer::deserializeAs(std::string_view key, std::unique_ptr<T>& data) {
    static_assert(std::is_base_of<Base, T>::value, "T should be derived from Base");
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    if (Base* ptr = data.get()) {
        deserialize(key, ptr);
    } else {
        deserialize(key, ptr);
        if (auto typeptr = dynamic_cast<T*>(ptr)) {
            data.reset(typeptr);
        } else {
            delete ptr;
            throw SerializationException(
                fmt::format("Could not deserialize \"{}\" types does not match", key), IVW_CONTEXT);
        }
    }
}

template <typename T, typename>
void Deserializer::deserialize(std::string_view key, T& sObj) {
    NodeSwitch nodeSwitch(*this, key);
    if (!nodeSwitch) return;
    sObj.deserialize(*this);
}

}  // namespace inviwo
