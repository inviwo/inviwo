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
#include <concepts>

#include <fmt/core.h>

namespace inviwo {

class Serializable;
class Deserializer;
class VersionConverter;
class InviwoApplication;

namespace detail {
template <typename T>
concept is_transparent = requires { typename T::key_compare::is_transparent; } || requires {
    typename T::hasher::is_transparent;
    typename T::key_equal::is_transparent;
};
}  // namespace detail

namespace deserializer {

constexpr auto defaultOnNew = []<typename T>(T&, size_t) {};
constexpr auto defaultOnRemove = [](std::string_view) {};
constexpr auto defaultOnMove = []<typename T>(T&, size_t) {};
constexpr auto defaultFilter = [](std::string_view, size_t) { return true; };

template <typename GetID, typename MakeNew, typename Filter = decltype(defaultFilter),
          typename OnNew = decltype(defaultOnNew), typename OnRemove = decltype(defaultOnRemove),
          typename OnMove = decltype(defaultOnMove)>
struct IdentifierFunctions {
    GetID getID;
    MakeNew makeNew;
    Filter filter = defaultFilter;
    OnNew onNew = defaultOnNew;
    OnRemove onRemove = defaultOnRemove;
    OnMove onMove = defaultOnMove;
};

constexpr auto defaultOnRemoveIndex = []<typename T>(T&) {};

template <typename MakeNew, typename OnNew = decltype(defaultOnNew),
          typename OnRemove = decltype(defaultOnRemoveIndex)>
struct IndexFunctions {
    MakeNew makeNew;
    OnNew onNew = defaultOnNew;
    OnRemove onRemove = defaultOnRemoveIndex;
};

constexpr auto defaultOnNewMap = []<typename K, typename T>(const K&, T&) {};
constexpr auto defaultOnRemoveMap = []<typename K>(const K&) {};
constexpr auto defaultFilterMap = []<typename K>(const K&) { return true; };

template <typename IdTransform, typename MakeNew, typename Filter = decltype(defaultFilterMap),
          typename OnNew = decltype(defaultOnNewMap),
          typename OnRemove = decltype(defaultOnRemoveMap)>
struct MapFunctions {
    std::string_view attributeKey = SerializeConstants::KeyAttribute;
    IdTransform idTransform;
    MakeNew makeNew;
    Filter filter = defaultFilterMap;
    OnNew onNew = defaultOnNewMap;
    OnRemove onRemove = defaultOnRemoveMap;
};

}  // namespace deserializer

class IVW_CORE_API Deserializer : public SerializeBase, public LogFilter {
public:
    /**
     * \brief Deserialize content from a file
     * @param fileName path to file that is to be deserialized.
     */
    explicit Deserializer(const std::filesystem::path& fileName,
                          std::string_view rootElement = SerializeConstants::InviwoWorkspace,
                          allocator_type alloc = {});

    /**
     * \brief Deserialize content from a stream.
     * @param stream Stream with content that is to be deserialized.
     * @param refPath Used to calculate paths relative to the stream source if any.
     */
    Deserializer(std::istream& stream, const std::filesystem::path& refPath,
                 std::string_view rootElement = SerializeConstants::InviwoWorkspace,
                 allocator_type alloc = {});

    /**
     * \brief Deserialize content from a stream.
     * @param content String with content that is to be deserialized.
     * @param refPath Used to calculate paths relative to the stream source if any.
     */
    Deserializer(const std::pmr::string& content, const std::filesystem::path& refPath,
                 std::string_view rootElement = SerializeConstants::InviwoWorkspace,
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
    template <typename T, typename A>
    void deserialize(std::string_view key, std::vector<T, A>& sVector,
                     std::string_view itemKey = "item");

    template <typename T, typename H, typename P, typename A>
    void deserialize(std::string_view key, std::unordered_set<T, H, P, A>& sSet,
                     std::string_view itemKey = "item");

    template <typename T>
    void deserialize(std::string_view key, std::list<T>& sContainer,
                     std::string_view itemKey = "item");

    template <typename T, size_t N>
    void deserialize(std::string_view key, std::array<T, N>& sContainer,
                     std::string_view itemKey = "item");

    enum class Result { NoChange, Modified, NotFound };
    template <std::regular T, typename A>
    Result deserializeTrackChanges(std::string_view key, std::vector<T, A>& sVector,
                                   std::string_view itemKey = "item");

    template <typename DeserializeFunction>
    void deserializeRange(std::string_view key, std::string_view itemKey,
                          DeserializeFunction deserializeFunction);

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

    template <typename K, typename V, typename H, typename C, typename A>
    void deserialize(std::string_view key, std::unordered_map<K, V, H, C, A>& map,
                     std::string_view itemKey = "item",
                     std::string_view comparisonAttribute = SerializeConstants::KeyAttribute);

    /**
     * For more advanced deserialization. Useful when one has to call observer notifications.
     * Example usage, serialize as usual
     * ```{.cpp}
     *     s.serialize("TFPrimitives", values_, "point");
     * ```
     * Then deserialize with notifications:
     * ```{.cpp}
     *    d.deserialize(("TFPrimitives", values_ "point", deserializer::IndexFunctions{
     *          .makeNew = []() { return std::unique_ptr<TFPrimitive>{}; },
     *          .onNew = [&](std::unique_ptr<TFPrimitiveSet>& primitive) {
     *                  notifyControlPointAdded(primitive.get());
     *              },
     *          .onRemove = [&](std::unique_ptr<TFPrimitiveSet>& primitive) {
     *                  notifyControlPointRemoved(primitive.get());
     *              }});
     * ```
     */
    template <typename C, typename T = typename C::value_type, typename... Funcs>
        requires requires(T& t, size_t i, std::string_view s,
                          deserializer::IndexFunctions<Funcs...> f) {
            { std::invoke(f.makeNew) } -> std::same_as<T>;
            { std::invoke(f.onNew, t, i) };
            { std::invoke(f.onRemove, t) };
        }
    void deserialize(std::string_view key, C& container, std::string_view itemKey,
                     deserializer::IndexFunctions<Funcs...> f);

    template <typename C, typename T = typename C::value_type, typename... Funcs>
        requires requires(T& t, size_t i, std::string_view s,
                          deserializer::IdentifierFunctions<Funcs...> f) {
            { std::invoke(f.getID, t) } -> std::same_as<std::string_view>;
            { std::invoke(f.makeNew) } -> std::same_as<T>;
            { std::invoke(f.filter, s, i) } -> std::same_as<bool>;
            { std::invoke(f.onNew, t, i) };
            { std::invoke(f.onRemove, s) };
            { std::invoke(f.onMove, t, i) };
        }
    void deserialize(std::string_view key, C& container, std::string_view itemKey,
                     deserializer::IdentifierFunctions<Funcs...> f);

    template <typename C, typename K = typename C::key_type, typename T = typename C::mapped_type,
              typename... Funcs>
        requires requires(const K& k, T& t, size_t i, std::string_view s,
                          deserializer::MapFunctions<Funcs...> f) {
            { std::invoke(f.idTransform, s) } -> std::same_as<K>;
            { std::invoke(f.makeNew) } -> std::same_as<T>;
            { std::invoke(f.filter, k) } -> std::same_as<bool>;
            { std::invoke(f.onNew, k, t) };
            { std::invoke(f.onRemove, k) };
        }
    void deserialize(std::string_view key, C& container, std::string_view itemKey,
                     deserializer::MapFunctions<Funcs...> f);

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

    /// \brief  Deserialize any Serializable object
    void deserialize(std::string_view key, Serializable& sObj);

    template <class Base, class T>
    void deserializeAs(std::string_view key, T*& data);

    template <class T>
    void deserialize(std::string_view key, std::unique_ptr<T>& data);

    template <class T>
    void deserialize(std::string_view key, std::shared_ptr<T>& data);

    template <class Base, class T>
    void deserializeAs(std::string_view key, std::unique_ptr<T>& data);

    template <class T>
    void deserialize(std::string_view key, T& sObj)
        requires requires(T& t, Deserializer& d) {
            { t.deserialize(d) };
        };

    std::optional<std::string_view> attribute(std::string_view key) const;
    std::optional<std::string_view> attribute(std::string_view child, std::string_view key) const;
    bool hasElement(std::string_view key) const;

    using ExceptionHandler = std::function<void(ExceptionContext)>;
    void setExceptionHandler(ExceptionHandler handler);
    void handleError(const ExceptionContext& context);

    void convertVersion(VersionConverter* converter);

    friend class NodeSwitch;

    void registerFactory(FactoryBase* factory);

    int getVersion() const;

private:
    /**
     * \brief For allocating objects such as processors, properties. using registered
     * factories.
     *
     * @param className is used by registered factories to allocate the required object.
     * @return nullptr if allocation fails or className does not exist in any factories.
     */
    template <typename T, bool Shared>
    auto getRegisteredType(std::string_view className) const;

    /**
     * \brief For allocating objects that do not belong to any registered factories.
     *
     * @return Smart pointer to object of type T.
     */
    template <typename T, bool Shared>
    static auto getNonRegisteredType();

    template <bool Shared, bool Resettable, typename Ptr>
    void deserializeSmartPtr(std::string_view key, Ptr& data);

    template <class T>
    void deserialize(std::string_view key, T*& data);

    TiXmlElement* retrieveChild(std::string_view key) const;

    ExceptionHandler exceptionHandler_;
    std::pmr::vector<FactoryBase*> registeredFactories_;

    int version_ = 0;
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

IVW_CORE_API std::string_view getAttribute(const TiXmlElement& node, std::string_view key);
IVW_CORE_API std::optional<std::string_view> attribute(const TiXmlElement* node,
                                                       std::string_view key);

template <typename T>
void getNodeAttribute(const TiXmlElement* node, std::string_view key, T& dest) {
    if (const auto str = attribute(node, key)) {
        detail::fromStr(*str, dest);
    }
}
template <typename T>
void getNodeAttribute(const TiXmlElement& node, std::string_view key, T& dest) {
    if (const auto str = attribute(&node, key)) {
        detail::fromStr(*str, dest);
    }
}

template <typename K, bool Transparent>
auto getChildKey(const TiXmlElement& child, std::string_view comparisonAttribute) {
    const auto childKeyStr = detail::attribute(&child, comparisonAttribute);
    if (!childKeyStr) {
        throw SerializationException(IVW_CONTEXT_CUSTOM("Deserializer"), "Missing childKeyStr");
    }
    if constexpr (std::is_same_v<K, std::string> && Transparent) {
        return *childKeyStr;
    } else {
        K childKey;
        detail::fromStr(*childKeyStr, childKey);
        return childKey;
    }
}

// This is enabled out of the box in c++26
template <typename Map, typename K, typename... Args>
auto try_emplace(Map& map, K&& key, Args&&... args) {
    auto it = map.find(key);
    bool inserted = false;
    if (it == map.end()) {
        it = map.emplace(std::piecewise_construct, std::forward_as_tuple(std::forward<K>(key)),
                         std::forward_as_tuple(std::forward<Args>(args)...))
                 .first;
        inserted = true;
    }
    return std::make_pair(it, inserted);
}

IVW_CORE_API TiXmlElement* firstChild(TiXmlElement* parent, std::string_view key);
IVW_CORE_API TiXmlElement* nextChild(TiXmlElement* child, std::string_view key);

template <typename F>
void forEachChild(TiXmlElement* node, std::string_view key, const F& fun) {
    for (TiXmlElement* child = firstChild(node, key); child; child = nextChild(child, key)) {
        fun(*child);
    }
}

}  // namespace detail

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
        } else if (const NodeSwitch ns{*this, key}) {
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
    if (const NodeSwitch ns{*this, key}) {
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

    if (const NodeSwitch ns{*this, key}) {
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
    if (auto str = attribute(key, SerializeConstants::ContentAttribute)) {
        bits = std::bitset<N>(str->data(), str->size());
    }
}

template <typename T, typename A>
void Deserializer::deserialize(std::string_view key, std::vector<T, A>& vector,
                               std::string_view itemKey) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    const NodeSwitch vectorNodeSwitch{*this, key};
    if (!vectorNodeSwitch) return;

    size_t i = 0;
    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        // In the next deserialization call do not fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        const NodeSwitch elementNodeSwitch{*this, child, false};

        if (vector.size() <= i) {
            vector.emplace_back();
            try {
                deserialize(itemKey, vector.back());
            } catch (...) {
                vector.pop_back();
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
    vector.erase(vector.begin() + i, vector.end());
}

template <std::regular T, typename A>
Deserializer::Result Deserializer::deserializeTrackChanges(std::string_view key,
                                                           std::vector<T, A>& vector,
                                                           std::string_view itemKey) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");

    using enum Result;

    const NodeSwitch vectorNodeSwitch{*this, key};
    if (!vectorNodeSwitch) return NotFound;

    Result result = NoChange;
    size_t i = 0;
    T old{};
    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        // In the next deserialization call do not fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        const NodeSwitch elementNodeSwitch{*this, child, false};

        if (vector.size() <= i) {
            result = Modified;
            vector.emplace_back();
            try {
                deserialize(itemKey, vector.back());
            } catch (...) {
                vector.pop_back();
                handleError(IVW_CONTEXT);
            }
        } else {
            try {
                old = vector[i];
                deserialize(itemKey, vector[i]);
                if (old != vector[i]) result = Modified;
            } catch (...) {
                handleError(IVW_CONTEXT);
            }
        }

        i++;
    });

    if (i != vector.size()) {
        vector.erase(vector.begin() + i, vector.end());
        result = Modified;
    }

    return result;
}

template <typename DeserializeFunction>
void Deserializer::deserializeRange(std::string_view key, std::string_view itemKey,
                                    DeserializeFunction deserializeFunction) {
    using enum Result;

    const NodeSwitch vectorNodeSwitch{*this, key};
    if (!vectorNodeSwitch) return;

    size_t i = 0;
    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        const NodeSwitch elementNodeSwitch{*this, child, false};
        try {
            deserializeFunction(*this, i);
        } catch (...) {
            handleError(IVW_CONTEXT);
        }
        i++;
    });
}

namespace detail {
template <typename C, typename Functions>
void reorder(Functions& f, C& list, const std::pmr::vector<std::string_view>& order) {
    size_t dst = 0;
    for (size_t i = 0; i < order.size() && dst < list.size(); ++i) {
        if (order[i] == f.getID(list[dst])) {
            ++dst;
            continue;
        }

        while (dst < list.size() &&
               std::find(order.begin() + i, order.end(), f.getID(list[dst])) == order.end()) {
            ++dst;
        }

        if (auto it = std::find_if(list.begin() + dst, list.end(),
                                   [&](const auto& item) { return f.getID(item) == order[i]; });
            it != list.end() && it != list.begin() + dst) {
            f.onMove(*it, dst);
        }
        ++dst;
    }
}
}  // namespace detail

template <typename C, typename T, typename... Funcs>
    requires requires(T& t, size_t i, std::string_view s,
                      deserializer::IdentifierFunctions<Funcs...> f) {
        { std::invoke(f.getID, t) } -> std::same_as<std::string_view>;
        { std::invoke(f.makeNew) } -> std::same_as<T>;
        { std::invoke(f.filter, s, i) } -> std::same_as<bool>;
        { std::invoke(f.onNew, t, i) };
        { std::invoke(f.onRemove, s) };
        { std::invoke(f.onMove, t, i) };
    }
void Deserializer::deserialize(std::string_view key, C& container, std::string_view itemKey,
                               deserializer::IdentifierFunctions<Funcs...> f) {

    std::pmr::vector<std::pmr::string> toRemove(getAllocator());
    for (const auto& item : container) {
        toRemove.emplace_back(f.getID(item));
    }

    const NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) {
        for (auto& id : toRemove) {
            f.onRemove(id);
        }
        return;
    }

    std::pmr::vector<std::string_view> foundIdentifiers(getAllocator());
    foundIdentifiers.reserve(container.size());

    size_t index = 0;
    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        const NodeSwitch elementNodeSwitch(*this, child, false);
        const std::string_view identifier = detail::getAttribute(child, "identifier");
        std::erase(toRemove, identifier);
        foundIdentifiers.emplace_back(identifier);

        auto it = std::ranges::find(container, identifier, f.getID);
        if (it != container.end()) {
            deserialize(itemKey, *it);
        } else if (f.filter(identifier, index)) {
            T newItem = f.makeNew();
            deserialize(itemKey, newItem);
            f.onNew(newItem, index);
        }
        ++index;
    });

    for (const auto& identifier : toRemove) f.onRemove(identifier);

    detail::reorder(f, container, foundIdentifiers);
}

template <typename C, typename T, typename... Funcs>
    requires requires(T& t, size_t i, std::string_view s,
                      deserializer::IndexFunctions<Funcs...> f) {
        { std::invoke(f.makeNew) } -> std::same_as<T>;
        { std::invoke(f.onNew, t, i) };
        { std::invoke(f.onRemove, t) };
    }
void Deserializer::deserialize(std::string_view key, C& container, std::string_view itemKey,
                               deserializer::IndexFunctions<Funcs...> f) {

    const NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) {
        while (!container.empty()) {
            auto elem = std::move(container.back());
            container.pop_back();
            f.onRemove(elem);
        }
        return;
    }

    size_t index = 0;
    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        const NodeSwitch elementNodeSwitch(*this, child, false);

        if (index < container.size()) {
            deserialize(itemKey, container[index]);
        } else {
            container.emplace_back(f.makeNew());
            deserialize(itemKey, container.back());
            f.onNew(container.back(), index);
        }
        ++index;
    });

    while (container.size() > index) {
        auto elem = std::move(container.back());
        container.pop_back();
        f.onRemove(elem);
    }
}

template <typename C, typename K, typename T, typename... Funcs>
    requires requires(const K& k, T& t, size_t i, std::string_view s,
                      deserializer::MapFunctions<Funcs...> f) {
        { std::invoke(f.idTransform, s) } -> std::same_as<K>;
        { std::invoke(f.makeNew) } -> std::same_as<T>;
        { std::invoke(f.filter, k) } -> std::same_as<bool>;
        { std::invoke(f.onNew, k, t) };
        { std::invoke(f.onRemove, k) };
    }
void Deserializer::deserialize(std::string_view key, C& container, std::string_view itemKey,
                               deserializer::MapFunctions<Funcs...> f) {

    std::pmr::vector<K> toRemove(getAllocator());
    for (const auto& item : container) {
        toRemove.emplace_back(item.first);
    }

    const NodeSwitch vectorNodeSwitch(*this, key);
    if (!vectorNodeSwitch) {
        for (const auto& item : toRemove) f.onRemove(item);
        return;
    }

    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        const NodeSwitch elementNodeSwitch(*this, child, false);
        const std::string_view identifier = detail::getAttribute(child, f.attributeKey);
        const K key = f.idTransform(identifier);
        std::erase(toRemove, key);

        auto it = container.find(key);
        if (it != container.end()) {
            deserialize(itemKey, it->second);
        } else if (f.filter(key)) {
            T newItem = f.makeNew();
            deserialize(itemKey, newItem);
            f.onNew(key, newItem);
        }
    });

    for (const auto& item : toRemove) f.onRemove(item);
}

template <typename T, typename H, typename P, typename A>
void Deserializer::deserialize(std::string_view key, std::unordered_set<T, H, P, A>& set,
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

    static_assert(!std::is_same_v<K, std::string> || detail::is_transparent<std::map<K, V, C, A>>);

    NodeSwitch mapNodeSwitch(*this, key);
    if (!mapNodeSwitch) return;

    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        // In the next deserialization call do not fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        NodeSwitch elementNodeSwitch(*this, child, false);

        const auto childKey = detail::getChildKey<K, detail::is_transparent<std::map<K, V, C, A>>>(
            child, comparisonAttribute);

        auto [it, inserted] = detail::try_emplace(map, childKey);
        try {
            deserialize(itemKey, it->second);
        } catch (...) {
            if (inserted) map.erase(it);
            handleError(IVW_CONTEXT);
        }
    });
}

template <typename K, typename V, typename H, typename C, typename A>
void Deserializer::deserialize(std::string_view key, std::unordered_map<K, V, H, C, A>& map,
                               std::string_view itemKey, std::string_view comparisonAttribute) {
    static_assert(isPrimitiveType<K>(), "Error: map key has to be a primitive type");
    static_assert(detail::canDeserialize<V>(), "Type is not serializable");

    static_assert(!std::is_same_v<K, std::string> ||
                  detail::is_transparent<std::unordered_map<K, V, H, C, A>>);

    NodeSwitch mapNodeSwitch(*this, key);
    if (!mapNodeSwitch) return;

    detail::forEachChild(rootElement_, itemKey, [&](TiXmlElement& child) {
        // In the next deserialization call do not fetch the "child" since we are looping...
        // hence the "false" as the last arg.
        const NodeSwitch elementNodeSwitch(*this, child, false);

        const auto childKey =
            detail::getChildKey<K, detail::is_transparent<std::unordered_map<K, V, H, C, A>>>(
                child, comparisonAttribute);

        auto [it, inserted] = detail::try_emplace(map, childKey);
        try {
            deserialize(itemKey, it->second);
        } catch (...) {
            if (inserted) map.erase(it);
            handleError(IVW_CONTEXT);
        }
    });
}

template <typename T, bool Shared>
auto Deserializer::getRegisteredType(std::string_view className) const {
    for (auto base : registeredFactories_) {
        if (base->hasKey(className)) {
            if (auto factory = dynamic_cast<Factory<T, std::string_view>*>(base)) {
                if constexpr (Shared) {
                    if (auto data = factory->createShared(className)) {
                        return data;
                    }
                } else {
                    if (auto data = factory->create(className)) {
                        return data;
                    }
                }
            }
        }
    }
    if constexpr (Shared) {
        return std::shared_ptr<T>();
    } else {
        return std::unique_ptr<T>();
    }
}

template <typename T, bool Shared>
auto Deserializer::getNonRegisteredType() {
    if constexpr (std::is_default_constructible_v<T>)
        if constexpr (Shared) {
            return std::make_shared<T>();
        } else {
            return std::make_unique<T>();
        }
    else {
        if constexpr (Shared) {
            return std::shared_ptr<T>();
        } else {
            return std::unique_ptr<T>();
        }
    }
}

template <bool Shared, bool Resettable, typename Ptr>
void Deserializer::deserializeSmartPtr(std::string_view key, Ptr& data) {
    auto keyNode = retrieveChild(key);
    if (!keyNode) return;

    using T = std::remove_cvref_t<decltype(*data)>;

    const auto typeAttr = detail::attribute(keyNode, SerializeConstants::TypeAttribute);

    if constexpr (requires(T t) {
                      { t.getClassIdentifier() } -> std::equality_comparable_with<std::string_view>;
                  }) {
        if (data && typeAttr && *typeAttr != data->getClassIdentifier()) {
            if constexpr (Resettable) {
                // object has wrong type, delete it and let the deserialization create a new object
                // with the correct type
                data.reset();
            } else {
                log::warn("Object with class Id: '{}' deserialized using type '{}'",
                          data->getClassIdentifier(), *typeAttr);
            }
        }
    }

    if (!data && typeAttr) {
        try {
            data = getRegisteredType<T, Shared>(*typeAttr);
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
            data = getNonRegisteredType<T, Shared>();
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
    deserializeSmartPtr<false, true>(key, data);
}

template <class T>
void Deserializer::deserialize(std::string_view key, std::shared_ptr<T>& data) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");
    deserializeSmartPtr<true, true>(key, data);
}

template <class T>
void Deserializer::deserialize(std::string_view key, T*& data) {
    static_assert(detail::canDeserialize<T>(), "Type is not serializable");
    if (data) {
        deserialize(key, *data);
    } else {
        std::unique_ptr<T> holder{};
        deserializeSmartPtr<false, false>(key, holder);
        data = holder.release();
    }
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

template <typename T>
void Deserializer::deserialize(std::string_view key, T& sObj)
    requires requires(T& t, Deserializer& d) {
        { t.deserialize(d) };
    }
{
    if (const NodeSwitch nodeSwitch{*this, key}) {
        sObj.deserialize(*this);
    }
}

void getVersion();

}  // namespace inviwo
