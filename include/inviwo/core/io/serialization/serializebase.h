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

#include <inviwo/core/common/inviwocoredefine.h>

#include <string>
#include <string_view>
#include <filesystem>

#include <fmt/format.h>
#include <fmt/std.h>

class TiXmlElement;
class TiXmlDocument;

namespace inviwo {

enum class SerializationTarget { Node, Attribute };

class NodeSwitch;
class Serializable;

class IVW_CORE_API SerializeBase {
public:
    using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

    /**
     * \brief Base class for Serializer and Deserializer.
     *
     * This class consists of features that are common to both serializer
     * and de-serializer.
     *
     * @param fileName full path to xml file (for reading or writing).
     */
    SerializeBase(const std::filesystem::path& fileName, allocator_type alloc = {});

    SerializeBase(const SerializeBase& rhs) = delete;
    SerializeBase(SerializeBase&& rhs) noexcept;
    SerializeBase& operator=(const SerializeBase&) = delete;
    SerializeBase& operator=(SerializeBase&&) noexcept;

    virtual ~SerializeBase();

    /**
     * \brief gets the xml file name.
     */
    virtual const std::filesystem::path& getFileName() const;

    /**
     * \brief Checks whether the given type is a primitive type.
     *
     * return true if type is one of following type:
     * bool, char, signed int, unsigned int, float, double, long double, std::string
     *
     * @tparam U the type to test
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

    static std::string nodeToString(const TiXmlElement& node);

protected:
    friend class NodeSwitch;

    std::filesystem::path fileName_;
    std::unique_ptr<TiXmlDocument> doc_;
    TiXmlElement* rootElement_;
    bool retrieveChild_;
};

namespace detail {

IVW_CORE_API void fromStr(std::string_view value, double& dest);
IVW_CORE_API void fromStr(std::string_view value, float& dest);
IVW_CORE_API void fromStr(std::string_view value, char& dest);
IVW_CORE_API void fromStr(std::string_view value, signed char& dest);
IVW_CORE_API void fromStr(std::string_view value, unsigned char& dest);
IVW_CORE_API void fromStr(std::string_view value, short& dest);
IVW_CORE_API void fromStr(std::string_view value, unsigned short& dest);
IVW_CORE_API void fromStr(std::string_view value, int& dest);
IVW_CORE_API void fromStr(std::string_view value, unsigned int& dest);
IVW_CORE_API void fromStr(std::string_view value, long& dest);
IVW_CORE_API void fromStr(std::string_view value, unsigned long& dest);
IVW_CORE_API void fromStr(std::string_view value, long long& dest);
IVW_CORE_API void fromStr(std::string_view value, unsigned long long& dest);

IVW_CORE_API void fromStr(std::string_view value, bool& dest);
IVW_CORE_API void fromStr(std::string_view value, std::string& dest);
IVW_CORE_API void fromStr(std::string_view value, std::pmr::string& dest);

template <class T>
decltype(auto) toStr(const T& value, std::pmr::vector<char>& buffer) {
    if constexpr (std::is_same_v<std::string, T>) {
        return value;
    } else if constexpr (std::is_same_v<std::string_view, T>) {
        return value;
    } else if constexpr (std::is_same_v<bool, T>) {
        static std::string_view trueVal = "1";
        static std::string_view falseVal = "0";
        return value ? trueVal : falseVal;
    } else {
        buffer.clear();
        auto [it, size] =
            fmt::format_to_n(std::back_inserter(buffer), buffer.max_size(), "{}", value);
        return std::string_view{buffer.data(), size};
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
     * @param node // the node to switch to
     * @param retrieveChild whether to retrieve child node or not.
     */
    NodeSwitch(SerializeBase& serializer, TiXmlElement* node, bool retrieveChild = true);
    NodeSwitch(SerializeBase& serializer, TiXmlElement& node, bool retrieveChild = true);

    /**
     * \brief NodeSwitch helps track parent node during recursive/nested function calls.
     *
     * @param serializer reference to serializer or deserializer
     * @param key the child to switch to.
     * @param retrieveChild whether to retrieve child node or not.
     */
    NodeSwitch(SerializeBase& serializer, std::string_view key, bool retrieveChild = true);

    ~NodeSwitch();

    operator bool() const;

private:
    SerializeBase* serializer_;  // reference to serializer or deserializer
    TiXmlElement* storedNode_;   // Parent (Ticpp Node) element.
    bool storedRetrieveChild_;
};

}  // namespace inviwo
