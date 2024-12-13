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
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/io/serialization/serializeconstants.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/glmutils.h>
#include <inviwo/core/util/typetraits.h>
#include <inviwo/core/util/detected.h>

#include <flags/flags.h>

#include <type_traits>
#include <list>
#include <bitset>
#include <vector>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <memory>
#include <filesystem>

namespace inviwo {

class IVW_CORE_API Serializer : public SerializeBase {
public:
    /**
     * \brief Initializes serializer with a file name that will be used to set relative paths to
     * data.
     *
     * The specified file name will not be used to write any content until writeFile() is called.
     * @param fileName full path to xml file.
     * @throws SerializationException
     */
    explicit Serializer(const std::filesystem::path& fileName, allocator_type alloc = {});

    virtual ~Serializer();

    /**
     * \brief Writes serialized data to the file specified by the currently set file name.
     * @throws SerializationException
     */
    void writeFile();

    /**
     * \brief Writes serialized data to stream.
     *
     * @param stream Stream to be written to.
     * @param format Format the output, i.e. insert line breaks and tabs.
     * @throws SerializationException
     */
    void writeFile(std::ostream& stream, bool format = false);

    void write(std::pmr::string& xml, bool format = false);

    // std containers
    template <typename T, typename Alloc, typename Pred = util::alwaysTrue,
              typename Proj = util::identity>
    void serialize(std::string_view key, const std::vector<T, Alloc>& sVector,
                   std::string_view itemKey = "item", Pred pred = {}, Proj proj = {});
    template <typename T>
    void serialize(std::string_view key, const std::unordered_set<T>& sSet,
                   std::string_view itemKey = "item");

    template <typename T>
    void serialize(std::string_view key, const std::list<T>& container,
                   std::string_view itemKey = "item");

    template <typename T, size_t N>
    void serialize(std::string_view key, const std::array<T, N>& container,
                   std::string_view itemKey = "item");

    template <typename K, typename V, typename C, typename A>
    void serialize(std::string_view key, const std::map<K, V, C, A>& map,
                   std::string_view itemKey = "item");

    template <typename K, typename V, typename H, typename C, typename A,
              typename Pred = util::alwaysTrue, typename KProj = util::identity,
              typename VProj = util::identity>
    void serialize(std::string_view key, const std::unordered_map<K, V, H, C, A>& map,
                   std::string_view itemKey = "item", Pred pred = {}, KProj kproj = {},
                   VProj vproj = {});

    // Specializations for chars
    void serialize(std::string_view key, const signed char& data,
                   SerializationTarget target = SerializationTarget::Node);
    void serialize(std::string_view key, const char& data,
                   SerializationTarget target = SerializationTarget::Node);
    void serialize(std::string_view key, const unsigned char& data,
                   SerializationTarget target = SerializationTarget::Node);

    // String types
    void serialize(std::string_view key, std::string_view data,
                   SerializationTarget target = SerializationTarget::Node);
    void serialize(std::string_view key, const std::string& data,
                   SerializationTarget target = SerializationTarget::Node);
    void serialize(std::string_view key, const std::pmr::string& data,
                   SerializationTarget target = SerializationTarget::Node);

    // integers, reals
    template <typename T>
        requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
    void serialize(std::string_view key, const T& data,
                   SerializationTarget target = SerializationTarget::Node);

    // Enum types
    template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
    void serialize(std::string_view key, const T& data,
                   SerializationTarget target = SerializationTarget::Node);

    // Flag types
    template <typename T>
    void serialize(std::string_view key, const flags::flags<T>& data,
                   SerializationTarget target = SerializationTarget::Node);

    // glm vector types
    template <typename Vec, typename std::enable_if<util::rank<Vec>::value == 1, int>::type = 0>
    void serialize(std::string_view key, const Vec& data);

    // glm matrix types
    template <typename Mat, typename std::enable_if<util::rank<Mat>::value == 2, int>::type = 0>
    void serialize(std::string_view key, const Mat& data);

    // bitsets
    template <size_t N>
    void serialize(std::string_view key, const std::bitset<N>& bits);

    // path
    void serialize(std::string_view key, const std::filesystem::path& path,
                   SerializationTarget target = SerializationTarget::Node);

    // serializable classes
    void serialize(std::string_view key, const Serializable& sObj);

    // pointers to something of the above.
    template <class T>
    void serialize(std::string_view key, const T* const& data);

    // unique_ptr to something of the above.
    template <class T, class D>
    void serialize(std::string_view key, const std::unique_ptr<T, D>& data);

    template <typename T>
    using HasSerialize = decltype(std::declval<const T>().serialize(std::declval<Serializer&>()));

    // serializable classes
    template <typename T,
              typename = std::enable_if_t<util::is_detected_exact_v<void, HasSerialize, T>>>
    void serialize(std::string_view key, const T& sObj);

    // SerializeFunction should look like this by default
    // [](Serializer& s, auto&& item) { s.serialize("item", std::forward<decltype(item)>(item)); };
    template <typename Range, typename SerializeFunction>
    void serializeRange(std::string_view key, Range&& range,
                        SerializeFunction serializeFunction = {}) {
        auto nodeSwitch = switchToNewNode(key);
        for (const auto& item : std::forward<Range>(range)) {
            serializeFunction(*this, item);
        }
    }

    std::pmr::string& addAttribute(std::string_view key);

    NodeSwitch switchToNewNode(std::string_view key);

    void setWorkspaceSaveMode(WorkspaceSaveMode mode) { workspaceSaveMode_ = mode; }
    WorkspaceSaveMode getWorkspaceSaveMode() const { return workspaceSaveMode_; }

protected:
    friend class NodeSwitch;
    TiXmlElement* getLastChild() const;

    static std::pmr::string& addAttribute(TiXmlElement* node, std::string_view key);

    WorkspaceSaveMode workspaceSaveMode_ = WorkspaceSaveMode::Disk;
};

template <typename T, typename Alloc, typename Pred, typename Proj>
void Serializer::serialize(std::string_view key, const std::vector<T, Alloc>& vector,
                           std::string_view itemKey, Pred pred, Proj proj) {
    if (vector.empty()) return;

    const auto nodeSwitch = switchToNewNode(key);
    for (const auto& item : vector) {
        if (std::invoke(pred, item)) {
            serialize(itemKey, std::invoke(proj, item));
        }
    }
}

template <typename T>
void Serializer::serialize(std::string_view key, const std::unordered_set<T>& set,
                           std::string_view itemKey) {
    if (set.empty()) return;

    const auto nodeSwitch = switchToNewNode(key);
    for (const auto& item : set) {
        serialize(itemKey, item);
    }
}

template <typename T>
void Serializer::serialize(std::string_view key, const std::list<T>& container,
                           std::string_view itemKey) {
    if (container.empty()) return;

    const auto nodeSwitch = switchToNewNode(key);
    for (const auto& item : container) {
        serialize(itemKey, item);
    }
}

template <typename T, size_t N>
void Serializer::serialize(std::string_view key, const std::array<T, N>& container,
                           std::string_view itemKey) {
    if (container.empty()) return;

    const auto nodeSwitch = switchToNewNode(key);
    for (const auto& item : container) {
        serialize(itemKey, item);
    }
}

template <typename K, typename V, typename C, typename A>
void Serializer::serialize(std::string_view key, const std::map<K, V, C, A>& map,
                           std::string_view itemKey) {
    static_assert(isPrimitiveType<K>());

    if (map.empty()) return;

    const auto nodeSwitch = switchToNewNode(key);
    for (const auto& item : map) {
        serialize(itemKey, item.second);
        auto& attr = addAttribute(getLastChild(), SerializeConstants::KeyAttribute);
        detail::formatTo(item.first, attr);
    }
}

template <typename K, typename V, typename H, typename C, typename A, typename Pred, typename KProj,
          typename VProj>
void Serializer::serialize(std::string_view key, const std::unordered_map<K, V, H, C, A>& map,
                           std::string_view itemKey, Pred pred, KProj kproj, VProj vproj) {
    static_assert(isPrimitiveType<decltype(std::invoke(kproj, std::declval<const K&>()))>());

    if (map.empty()) return;

    const auto nodeSwitch = switchToNewNode(key);
    for (const auto& item : map) {
        if (std::invoke(pred, item)) {
            serialize(itemKey, std::invoke(vproj, item.second));
            auto& attr = addAttribute(getLastChild(), SerializeConstants::KeyAttribute);
            detail::formatTo(std::invoke(kproj, item.first), attr);
        }
    }
}

template <class T, class D>
void Serializer::serialize(std::string_view key, const std::unique_ptr<T, D>& data) {
    serialize(key, data.get());
}

template <class T>
void Serializer::serialize(std::string_view key, const T* const& data) {
    serialize(key, *data);
}

// integers, reals
template <typename T>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
void Serializer::serialize(std::string_view key, const T& data, SerializationTarget target) {
    if (target == SerializationTarget::Attribute) {
        auto& attr = addAttribute(key);
        detail::formatTo(data, attr);
    } else {
        const auto nodeSwitch = switchToNewNode(key);
        auto& attr = addAttribute(SerializeConstants::ContentAttribute);
        detail::formatTo(data, attr);
    }
}

// enum types
template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type>
void Serializer::serialize(std::string_view key, const T& data, SerializationTarget target) {
    using ET = std::underlying_type_t<T>;
    const ET tmpdata{static_cast<const ET>(data)};
    serialize(key, tmpdata, target);
}

// Flag types
template <typename T>
void Serializer::serialize(std::string_view key, const flags::flags<T>& data,
                           SerializationTarget target) {
    serialize(key, data.underlying_value(), target);
}

// glm vector types
template <typename Vec, typename std::enable_if<util::rank<Vec>::value == 1, int>::type>
void Serializer::serialize(std::string_view key, const Vec& data) {
    const auto nodeSwitch = switchToNewNode(key);
    for (size_t i = 0; i < util::extent<Vec, 0>::value; ++i) {
        auto& attr = addAttribute(SerializeConstants::VectorAttributes[i]);
        detail::formatTo(data[i], attr);
    }
}

// glm matrix types
template <typename Mat, typename std::enable_if<util::rank<Mat>::value == 2, int>::type>
void Serializer::serialize(std::string_view key, const Mat& data) {
    const auto nodeSwitch = switchToNewNode(key);
    for (size_t i = 0; i < util::extent<Mat, 0>::value; ++i) {
        serialize(SerializeConstants::MatrixAttributes[i], data[i]);
    }
}

template <size_t N>
void Serializer::serialize(std::string_view key, const std::bitset<N>& bits) {
    const auto nodeSwitch = switchToNewNode(key);
    auto& attr = addAttribute(SerializeConstants::ContentAttribute);
    detail::formatTo(bits, attr);
}

// serializable classes
template <typename T, typename>
void Serializer::serialize(std::string_view key, const T& sObj) {
    const auto nodeSwitch = switchToNewNode(key);
    sObj.serialize(*this);
}

}  // namespace inviwo
