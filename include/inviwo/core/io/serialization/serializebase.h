/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <bitset>
#include <limits>
#include <memory>
#include <filesystem>

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
     * @param alloc Allocator to use for memory resources.
     */
    explicit SerializeBase(const std::filesystem::path& fileName, allocator_type alloc = {});

    SerializeBase(const SerializeBase& rhs) = delete;
    SerializeBase(SerializeBase&& rhs) noexcept;
    SerializeBase& operator=(const SerializeBase&) = delete;
    SerializeBase& operator=(SerializeBase&&) noexcept;

    virtual ~SerializeBase();

    /**
     * \brief Gets the workspace file name.
     */
    const std::filesystem::path& getFileName() const { return fileName_; }
    /**
     * \brief Gets the workspace file directory.
     */
    const std::filesystem::path& getFileDir() const { return fileDir_; }

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
               std::is_same_v<T, std::string> || std::is_same_v<T, std::pmr::string> ||
               std::is_same_v<T, std::string_view>;
    }

    allocator_type getAllocator() const;

    TiXmlDocument& doc();

protected:
    friend class NodeSwitch;

    std::filesystem::path fileName_;
    std::filesystem::path fileDir_;
    std::unique_ptr<TiXmlDocument> doc_;

    TiXmlElement* rootElement_;
    bool retrieveChild_;
};

namespace detail {

// NOLINTBEGIN(google-runtime-int)
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
inline void fromStr(std::string_view value, std::string& dest) { dest = value; }
inline void fromStr(std::string_view value, std::pmr::string& dest) { dest = value; }

IVW_CORE_API void formatTo(double value, std::pmr::string& out);
IVW_CORE_API void formatTo(float value, std::pmr::string& out);
IVW_CORE_API void formatTo(char value, std::pmr::string& out);
IVW_CORE_API void formatTo(signed char value, std::pmr::string& out);
IVW_CORE_API void formatTo(unsigned char value, std::pmr::string& out);
IVW_CORE_API void formatTo(short value, std::pmr::string& out);
IVW_CORE_API void formatTo(unsigned short value, std::pmr::string& out);
IVW_CORE_API void formatTo(int value, std::pmr::string& out);
IVW_CORE_API void formatTo(unsigned int value, std::pmr::string& out);
IVW_CORE_API void formatTo(long value, std::pmr::string& out);
IVW_CORE_API void formatTo(unsigned long value, std::pmr::string& out);
IVW_CORE_API void formatTo(long long value, std::pmr::string& out);
IVW_CORE_API void formatTo(unsigned long long value, std::pmr::string& out);
IVW_CORE_API void formatTo(const std::filesystem::path& value, std::pmr::string& out);

inline void formatTo(bool value, std::pmr::string& out) {
    static constexpr char trueVal = '1';
    static constexpr char falseVal = '0';
    out.push_back(value ? trueVal : falseVal);
}
inline void formatTo(const std::string& value, std::pmr::string& out) { out.append(value); }
inline void formatTo(const std::pmr::string& value, std::pmr::string& out) { out.append(value); }

IVW_CORE_API void formatToBinary(unsigned long long value, size_t bits, std::pmr::string& out);
template <size_t N>
void formatTo(const std::bitset<N>& value, std::pmr::string& out) {
    if constexpr (N <= std::numeric_limits<unsigned long long>::digits) {
        formatToBinary(value.to_ullong(), value.size(), out);
    } else {
        out.append(value.to_string());
    }
}

// NOLINTEND(google-runtime-int)

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
     * @param node the node to switch to
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
