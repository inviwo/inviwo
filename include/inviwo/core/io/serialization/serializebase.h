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
#include <inviwo/core/io/serialization/serializeconstants.h>
#include <inviwo/core/io/serialization/serializationexception.h>

#include <map>
#include <string>
#include <charconv>
#include <array>
#include <sstream>

namespace ticpp {
class Element;
class Document;
}  // namespace ticpp

namespace inviwo {
using TxElement = ticpp::Element;
using TxDocument = ticpp::Document;

namespace config {
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
constexpr bool charconv = true;
#else
constexpr bool charconv = false;
#endif
}  // namespace config

namespace detail {
IVW_CORE_API std::string getNodeAttributeOrDefault(TxElement* node, const std::string& key,
                                                   const std::string& defaultValue);
}

template <typename T>
struct ElementIdentifier {
    virtual ~ElementIdentifier() = default;
    virtual void setKey(TxElement*) = 0;
    virtual bool operator()(const T* elem) const = 0;
};

template <typename T>
struct StandardIdentifier : public ElementIdentifier<T> {
    typedef std::string (T::*funcPtr)() const;

    StandardIdentifier(std::string key = "identifier", funcPtr ptr = &T::getIdentifier)
        : ptr_(ptr), key_(std::move(key)) {}

    virtual void setKey(TxElement* node) {
        identifier_ = detail::getNodeAttributeOrDefault(node, key_, "");
    }
    virtual bool operator()(const T* elem) const { return identifier_ == (*elem.*ptr_)(); }

private:
    funcPtr ptr_;
    std::string key_;
    std::string identifier_;
};

enum class SerializationTarget { Node, Attribute };

class NodeSwitch;
class Serializable;

class IVW_CORE_API SerializeBase {
public:
    /**
     * \brief Base class for Serializer and Deserializer.
     *
     * This class consists of features that are common to both serializer
     * and de-serializer. Some of them are reference data manager,
     * (ticpp::Node) node switch and factory registration.
     */
    SerializeBase();

    /**
     * \brief Base class for Serializer and Deserializer.
     *
     * This class consists of features that are common to both serializer
     * and de-serializer. Some of them are reference data manager,
     * (ticpp::Node) node switch and factory registration.
     *
     * @param fileName full path to xml file (for reading or writing).
     */
    SerializeBase(std::string_view fileName);

    /**
     * \brief Base class for Serializer and Deserializer.
     *
     * This class consists of features that are common to both serializer
     * and de-serializer. Some of them are reference data manager,
     * (ticpp::Node) node switch and factory registration.
     *
     * @param stream containing all xml data (for reading).
     * @param path A path that will be used to decode the location of data during deserialization.
     */
    SerializeBase(std::istream& stream, std::string_view path);

    SerializeBase(const SerializeBase& rhs) = delete;
    SerializeBase(SerializeBase&& rhs) noexcept;
    SerializeBase& operator=(const SerializeBase&) = delete;
    SerializeBase& operator=(SerializeBase&&) noexcept;

    virtual ~SerializeBase();

    /**
     * \brief gets the xml file name.
     */
    virtual const std::string& getFileName() const;

    /**
     * \brief Checks whether the given type is a primitive type.
     *
     * return true if type is one of following type:
     * bool, char, signed int, unsigned int, float, double, long double, std::string
     *
     * @param type can be one of  bool, char, signed int, unsigned int, float, double, long double,
     * std::string
     * @return true or false
     */
    template <typename U>
    static constexpr bool isPrimitiveType() {
        using T = std::remove_cv_t<std::remove_reference_t<U>>;
        return std::is_same_v<T, bool> || std::is_same_v<T, char> || std::is_same_v<T, int> ||
               std::is_same_v<T, signed int> || std::is_same_v<T, unsigned int> ||
               std::is_same_v<T, size_t> || std::is_same_v<T, long long> ||
               std::is_same_v<T, unsigned long long> || std::is_same_v<T, float> ||
               std::is_same_v<T, double> || std::is_same_v<T, long double> ||
               std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;
    }

    static std::string nodeToString(const TxElement& node);

protected:
    friend class NodeSwitch;

    std::string fileName_;
    std::unique_ptr<TxDocument> doc_;
    TxElement* rootElement_;
    bool retrieveChild_;
};

namespace detail {

template <class T>
decltype(auto) toStr(const T& value) {
    if constexpr (std::is_same_v<std::string, T>) {
        return value;
    } else if constexpr (std::is_same_v<std::string_view, T>) {
        return std::string{value};
    } else if constexpr (config::charconv &&
                         (std::is_same_v<double, T> || std::is_same_v<float, T> ||
                          (!std::is_same_v<bool, T> && std::is_integral_v<T>))) {
        std::array<char, 40> buff;
        auto [p, ec] = std::to_chars(buff.data(), buff.data() + buff.size(), value);
        if (ec != std::errc()) {
            throw SerializationException("Error writing number", IVW_CONTEXT_CUSTOM("toStr"));
        }
        return std::string{buff.data(), static_cast<size_t>(p - buff.data())};

    } else {
        std::ostringstream stream;
        if constexpr (std::is_same_v<T, double>) {
            stream.precision(17);
        } else if constexpr (std::is_same_v<T, float>) {
            stream.precision(8);
        }
        stream << value;
        return std::move(stream).str();
    }
}

template <class T>
void fromStr(const std::string& value, T& dest) {
    if constexpr (std::is_same_v<std::string, T>) {
        dest = value;
    } else if constexpr (config::charconv &&
                         (std::is_same_v<double, T> || std::is_same_v<float, T> ||
                          (!std::is_same_v<bool, T> && std::is_integral_v<T>))) {
        const auto end = value.data() + value.size();
        if (auto [p, ec] = std::from_chars(value.data(), end, dest);
            ec != std::errc() || p != end) {
            throw SerializationException("Error parsing number", IVW_CONTEXT_CUSTOM("fromStr"));
        }
    } else {
        std::istringstream stream{value};
        stream >> dest;
    }
}

}  // namespace detail

class IVW_CORE_API NodeSwitch {
public:
    NodeSwitch(const NodeSwitch&) = delete;
    NodeSwitch& operator=(const NodeSwitch&) = delete;

    NodeSwitch(NodeSwitch&&) noexcept;
    NodeSwitch& operator=(NodeSwitch&&) noexcept;

    /**
     * \brief NodeSwitch helps track parent node during recursive/nested function calls.
     *
     * @param serializer reference to serializer or deserializer
     * @param node //Parent (Ticpp Node) element.
     * @param retrieveChild whether to retrieve child node or not.
     */
    NodeSwitch(SerializeBase& serializer, TxElement* node, bool retrieveChild = true);

    /**
     * \brief NodeSwitch helps track parent node during recursive/nested function calls.
     *
     * @param serializer reference to serializer or deserializer
     * @param node //Parent (Ticpp Node) element.
     * @param retrieveChild whether to retrieve child node or not.
     */
    NodeSwitch(SerializeBase& serializer, std::unique_ptr<TxElement> node,
               bool retrieveChild = true);

    /**
     * \brief NodeSwitch helps track parent node during recursive/nested function calls.
     *
     * @param serializer reference to serializer or deserializer
     * @param key the child to switch to.
     * @param retrieveChild whether to retrieve child node or not.
     */
    NodeSwitch(SerializeBase& serializer, std::string_view key, bool retrieveChild = true);

    /**
     * \brief Destructor
     */
    ~NodeSwitch();

    operator bool() const;

private:
    std::unique_ptr<TxElement> node_;
    SerializeBase* serializer_;  // reference to serializer or deserializer
    TxElement* storedNode_;      // Parent (Ticpp Node) element.
    bool storedRetrieveChild_;
};

}  // namespace inviwo
