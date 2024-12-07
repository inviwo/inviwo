/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmfmt.h>
#include <inviwo/core/util/introspection.h>
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/util/defaultvalues.h>

#include <type_traits>

namespace inviwo {

/**
 * \class DataTraits
 * \brief A traits class for getting information about a data object.
 * This provides a customization point if one wants to generate the information dynamically,
 * by specializing the traits for your kind of Data:
 *
 *     template <>
 *     struct DataTraits<MyDataType> {
 *         static std::string_view classIdentifier() {
 *             static constexpr std::string_view id{"org.something.mydatatype"};
 *             return id;
 *         }
 *         static std::string_view dataName() {
 *             static constexpr std::string_view name{"MyDataType"};
 *             return name;
 *         }
 *         static uvec3 colorCode() {
 *             return uvec3{55,66,77};
 *         }
 *         static Document info(const MyDataType& data) {
 *              Document doc;
 *              doc.append("p", data.someInfo());
 *              return doc;
 *         }
 *     };
 *
 * The default behavior uses the static members "classIdentifier", "dataName",
 * "colorCode" and "getInfo()".
 */
template <typename T, typename = void>
struct DataTraits {
    /**
     * The Class Identifier has to be globally unique. Use a reverse DNS naming scheme.
     * Example: "org.someorg.mydatatype"
     * The default implementation will look for a static std::string / std::string_view member
     * T::classIdentifier. In case it is not found an empty string will be returned.
     * An empty class identifier will be considered an error in various factories.
     */
    static constexpr std::string_view classIdentifier() { return util::classIdentifier<T>(); }

    /**
     * Should return a user friendly version of the above identifier, "MyDataType" for example.
     * Does not have to be unique, and usually shorter then the class identifier.
     * The default implementation will look for a static std::string / std::string_view member
     * T::dataName. In case it is not found the classIdentifier will be returned.
     */
    static constexpr std::string_view dataName() { return util::dataName<T>(); }
    /**
     * Should return a color that will be used to identify ports of this data type
     * The default implementation will look for a static uvec3 member T::colorCode.
     * In case it is not found black will be returned.
     */
    static constexpr uvec3 colorCode() { return util::colorCode<T>(); }

    /**
     * Should return a document with information describing the data.
     * The default implementation will look for a static function
     * Document T::getInfo(const T& data).
     * In case it is not found a Document only containing the dataName() will be returned.
     * @see Document
     */
    static Document info(const T& data) { return util::info<T>(data); }
};

namespace util {

constexpr uvec3 getDataFormatColor(NumericType t, size_t comp, size_t size) {
    uvec3 color{};
    switch (t) {
        case NumericType::Float:
            color.r = 30;
            break;
        case NumericType::SignedInteger:
            color.r = 60;
            break;
        case NumericType::UnsignedInteger:
            color.r = 90;
            break;
        default:
            color.r = 0;
            break;
    }

    switch (comp) {
        case 1:
            color.g = 30;
            break;
        case 2:
            color.g = 60;
            break;
        case 3:
            color.g = 90;
            break;
        case 4:
            color.g = 120;
            break;
        default:
            color.g = 0;
            break;
    }
    switch (size) {
        case 1:
            color.b = 30;
            break;
        case 2:
            color.b = 60;
            break;
        case 3:
            color.b = 90;
            break;
        case 4:
            color.b = 120;
            break;
        case 8:
            color.b = 150;
            break;
        default:
            color.b = 0;
            break;
    }
    return color;
}

/**
 * Appends b to a if a is not empty and returns a.
 * Useful if an empty a is considered an error and we want to propagate that error.
 */
IVW_CORE_API std::string appendIfNotEmpty(std::string_view a, std::string_view b);

}  // namespace util

// Specializations for data format types
template <typename T>
struct DataTraits<T,
                  std::enable_if_t<DataFormatBase::typeToId<T>() != DataFormatId::NotSpecialized>> {
    static constexpr auto classId{"org.inviwo." + DataFormat<T>::staticStr()};
    static constexpr std::string_view classIdentifier() { return classId; }
    static constexpr std::string_view dataName() { return DataFormat<T>::str(); }
    static constexpr uvec3 colorCode() {
        return util::getDataFormatColor(DataFormat<T>::numtype, DataFormat<T>::comp,
                                        DataFormat<T>::compsize);
    }
    static Document info(const T& data) {
        Document doc;
        doc.append("p", dataName());
        doc.append("p", fmt::to_string(data));
        return doc;
    }
};

// Specializations for glm mat types
template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct DataTraits<glm::mat<C, R, T, Q>,
                  std::enable_if_t<DataFormatBase::typeToId<glm::mat<C, R, T, Q>>() ==
                                   DataFormatId::NotSpecialized>> {
    static constexpr auto classId{"org.inviwo." + Defaultvalues<glm::mat<C, R, T, Q>>::getName()};
    static constexpr auto name{Defaultvalues<glm::mat<C, R, T, Q>>::getName()}; 
    static constexpr std::string_view classIdentifier() { return classId; }
    static constexpr std::string_view dataName() { return name; }
    static constexpr uvec3 colorCode() {
        uvec3 color{};

        if constexpr (std::is_same_v<T, float>) {
            color.r = 30;
        } else if constexpr (std::is_same_v<T, double>) {
            color.r = 60;
        } else if constexpr (std::is_same_v<T, int>) {
            color.r = 90;
        } else if constexpr (std::is_same_v<T, unsigned int>) {
            color.r = 120;
        } else {
            color.r = 240;
        }

        if constexpr (C == 1) {
            color.g = 60;
        } else if constexpr (C == 2) {
            color.g = 90;
        } else if constexpr (C == 3) {
            color.g = 120;
        } else if constexpr (C == 4) {
            color.g = 150;
        }

        if constexpr (R == 1) {
            color.b = 60;
        } else if constexpr (R == 2) {
            color.b = 90;
        } else if constexpr (R == 3) {
            color.b = 120;
        } else if constexpr (R == 4) {
            color.b = 150;
        }
        return color;
    }
    static Document info(const glm::mat<C, R, T, Q>& data) {
        Document doc;
        doc.append("p", dataName());
        doc.append("p", fmt::to_string(data));
        return doc;
    }
};

// Specialization for vectors.

namespace detail {
template <typename T>
static Document vectorInfo(size_t size, const T* first, const T* last) {
    Document doc;
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    auto p = doc.append("p");
    utildoc::TableBuilder tb(p, P::end());
    tb(H("Size"), size);
    if (first) {
        tb(H("First"), DataTraits<T>::info(*first));
    }
    if (last && last != first) {
        tb(H("Last"), DataTraits<T>::info(*last));
    }
    return doc;
}

}  // namespace detail

template <>
struct DataTraits<std::string> {
    static constexpr std::string_view classIdentifier() { return "string"; }
    static constexpr std::string_view dataName() { return "string"; }
    static constexpr uvec3 colorCode() { return uvec3{126, 210, 90}; }
    static Document info(const std::string& data) { return Document{data}; }
};

template <typename T, typename A>
struct DataTraits<std::vector<T, A>> {
    static constexpr auto cid = []() {
        constexpr auto name = DataTraits<T>::classIdentifier();
        if constexpr (!name.empty()) {
            return StaticString<name.size()>(name) + ".vector";
        } else {
            return StaticString<0>{};
        }
    }();
    static constexpr auto name = []() {
        constexpr auto tName = DataTraits<T>::dataName();
        if constexpr (!tName.empty()) {
            return "vector<" + StaticString<tName.size()>(tName) + ">";
        } else {
            return StaticString{"vector<?>"};
        }
    }();
    static constexpr std::string_view classIdentifier() { return cid; }
    static constexpr std::string_view dataName() { return name; }
    static constexpr uvec3 colorCode() { return color::lighter(DataTraits<T>::colorCode(), 1.12f); }
    static Document info(const std::vector<T, A>& data) {
        return detail::vectorInfo<T>(data.size(), data.empty() ? nullptr : &data.front(),
                                     data.empty() ? nullptr : &data.back());
    }
};
template <typename T, typename A>
struct DataTraits<std::vector<const T, A>> {
    static constexpr auto cid = []() {
        constexpr auto name = DataTraits<T>::classIdentifier();
        if constexpr (!name.empty()) {
            return StaticString<name.size()>(name) + ".const_vector";
        } else {
            return StaticString<0>{};
        }
    }();
    static constexpr auto name = []() {
        constexpr auto tName = DataTraits<T>::dataName();
        if constexpr (!tName.empty()) {
            return "vector<const " + StaticString<tName.size()>(tName) + ">";
        } else {
            return StaticString{"vector<const ?>"};
        }
    }();
    static constexpr std::string_view classIdentifier() { return cid; }
    static constexpr std::string_view dataName() { return name; }
    static constexpr uvec3 colorCode() { return color::lighter(DataTraits<T>::colorCode(), 1.12f); }
    static Document info(const std::vector<const T, A>& data) {
        return detail::vectorInfo<T>(data.size(), data.empty() ? nullptr : &data.front(),
                                     data.empty() ? nullptr : &data.back());
    }
};

template <typename T, typename A>
struct DataTraits<std::vector<T*, A>> {
    static constexpr auto cid = []() {
        constexpr auto name = DataTraits<T>::classIdentifier();
        if constexpr (!name.empty()) {
            return StaticString<name.size()>(name) + ".ptr.vector";
        } else {
            return StaticString<0>{};
        }
    }();
    static constexpr auto name = []() {
        constexpr auto tName = DataTraits<T>::dataName();
        if constexpr (!tName.empty()) {
            return "vector<" + StaticString<tName.size()>(tName) + "*>";
        } else {
            return StaticString{"vector<?*>"};
        }
    }();
    static constexpr std::string_view classIdentifier() { return cid; }
    static constexpr std::string_view dataName() { return name; }
    static constexpr uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<T*, A>& data) {
        return detail::vectorInfo<T>(data.size(), data.empty() ? nullptr : data.front(),
                                     data.empty() ? nullptr : data.back());
    }
};

template <typename T, typename A>
struct DataTraits<std::vector<const T*, A>> {
    static constexpr auto cid = []() {
        constexpr auto name = DataTraits<T>::classIdentifier();
        if constexpr (!name.empty()) {
            return StaticString<name.size()>(name) + ".const_ptr.vector";
        } else {
            return StaticString<0>{};
        }
    }();
    static constexpr auto name = []() {
        constexpr auto tName = DataTraits<T>::dataName();
        if constexpr (!tName.empty()) {
            return "vector<const " + StaticString<tName.size()>(tName) + "*>";
        } else {
            return StaticString{"vector<const ?*>"};
        }
    }();
    static constexpr std::string_view classIdentifier() { return cid; }
    static constexpr std::string_view dataName() { return name; }
    static constexpr uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<const T*, A>& data) {
        return detail::vectorInfo<T>(data.size(), data.empty() ? nullptr : data.front(),
                                     data.empty() ? nullptr : data.back());
    }
};

template <typename T, typename D, typename A>
struct DataTraits<std::vector<std::unique_ptr<T, D>, A>> {
    static constexpr auto cid = []() {
        constexpr auto name = DataTraits<T>::classIdentifier();
        if constexpr (!name.empty()) {
            return StaticString<name.size()>(name) + ".unique_ptr.vector";
        } else {
            return StaticString<0>{};
        }
    }();
    static constexpr auto name = []() {
        constexpr auto tName = DataTraits<T>::dataName();
        if constexpr (!tName.empty()) {
            return "vector<unique_ptr<" + StaticString<tName.size()>(tName) + ">>";
        } else {
            return StaticString{"vector<unique_ptr<?>>"};
        }
    }();
    static constexpr std::string_view classIdentifier() { return cid; }
    static constexpr std::string_view dataName() { return name; }
    static constexpr uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<std::unique_ptr<T, D>, A>& data) {
        return detail::vectorInfo<T>(data.size(), data.empty() ? nullptr : data.front().get(),
                                     data.empty() ? nullptr : data.back().get());
    }
};

template <typename T, typename D, typename A>
struct DataTraits<std::vector<std::unique_ptr<const T, D>, A>> {
    static constexpr auto cid = []() {
        constexpr auto name = DataTraits<T>::classIdentifier();
        if constexpr (!name.empty()) {
            return StaticString<name.size()>(name) + ".const_unique_ptr.vector";
        } else {
            return StaticString<0>{};
        }
    }();
    static constexpr auto name = []() {
        constexpr auto tName = DataTraits<T>::dataName();
        if constexpr (!tName.empty()) {
            return "vector<unique_ptr<const " + StaticString<tName.size()>(tName) + ">>";
        } else {
            return StaticString{"vector<unique_ptr<const ?>>"};
        }
    }();
    static constexpr std::string_view classIdentifier() { return cid; }
    static constexpr std::string_view dataName() { return name; }
    static constexpr uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<std::unique_ptr<const T, D>, A>& data) {
        return detail::vectorInfo<T>(data.size(), data.empty() ? nullptr : data.front().get(),
                                     data.empty() ? nullptr : data.back().get());
    }
};

template <typename T, typename A>
struct DataTraits<std::vector<std::shared_ptr<T>, A>> {
    static constexpr auto cid = []() {
        constexpr auto name = DataTraits<T>::classIdentifier();
        if constexpr (!name.empty()) {
            return StaticString<name.size()>(name) + ".shared_ptr.vector";
        } else {
            return StaticString<0>{};
        }
    }();
    static constexpr auto name = []() {
        constexpr auto tName = DataTraits<T>::dataName();
        if constexpr (!tName.empty()) {
            return "vector<shared_ptr<" + StaticString<tName.size()>(tName) + ">>";
        } else {
            return StaticString{"vector<shared_ptr<?>>"};
        }
    }();
    static constexpr std::string_view classIdentifier() { return cid; }
    static constexpr std::string_view dataName() { return name; }
    static constexpr uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<std::shared_ptr<T>, A>& data) {
        return detail::vectorInfo<T>(data.size(), data.empty() ? nullptr : data.front().get(),
                                     data.empty() ? nullptr : data.back().get());
    }
};

template <typename T, typename A>
struct DataTraits<std::vector<std::shared_ptr<const T>, A>> {
    static constexpr auto cid = []() {
        constexpr auto name = DataTraits<T>::classIdentifier();
        if constexpr (!name.empty()) {
            return StaticString<name.size()>(name) + ".const_shared_ptr.vector";
        } else {
            return StaticString<0>{};
        }
    }();
    static constexpr auto name = []() {
        constexpr auto tName = DataTraits<T>::dataName();
        if constexpr (!tName.empty()) {
            return "vector<shared_ptr<const " + StaticString<tName.size()>(tName) + ">>";
        } else {
            return StaticString{"vector<shared_ptr<const ?>>"};
        }
    }();
    static constexpr std::string_view classIdentifier() { return cid; }
    static constexpr std::string_view dataName() { return name; }
    static constexpr uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<std::shared_ptr<const T>, A>& data) {
        return detail::vectorInfo<T>(data.size(), data.empty() ? nullptr : data.front().get(),
                                     data.empty() ? nullptr : data.back().get());
    }
};

}  // namespace inviwo
