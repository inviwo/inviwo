/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_DATATRAITS_H
#define IVW_DATATRAITS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/introspection.h>
#include <inviwo/core/util/stringconversion.h>

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
 *         static std::string classIdentifier() {
 *             return "org.something.mydatatype";
 *         }
 *         static std::string dataName() {
 *             return "MyDataType";
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
     * The default implementation will look for a static std::string member T::classIdentifier.
     * In case it is not found an empty string will be returned. An empty class identifier will be
     * considered an error in various factories.
     */
    static std::string classIdentifier() { return util::classIdentifier<T>(); }

    /**
     * Should return a user friendly version of the above identifier, "MyDataType" for example.
     * Does not have to be unique, and usually shorter then the class identifier.
     * The default implementation will look for a static std::string member T::dataName.
     * In case it is not found the classIdentifier will be returned.
     */

    static std::string dataName() { return util::dataName<T>(); }
    /**
     * Should return a color that will be used to identify ports of this data type
     * The default implementation will look for a static uvec3 member T::colorCode.
     * In case it is not found black will be returned.
     */
    static uvec3 colorCode() { return util::colorCode<T>(); }

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

IVW_CORE_API uvec3 getDataFormatColor(NumericType t, size_t comp, size_t size);

/**
 * Appends b to a if a is not empty and returns a.
 * Useful if an empty a is considered an error and we want to propagate that error.
 */
IVW_CORE_API std::string appendIfNotEmpty(const std::string& a, const std::string& b);

}  // namespace util

// Specializations for data format types
template <typename T>
struct DataTraits<T, std::enable_if_t<util::HasDataFormat<T>::value>> {
    static std::string classIdentifier() { return "org.inviwo." + DataFormat<T>::str(); }
    static std::string dataName() { return DataFormat<T>::str(); }
    static uvec3 colorCode() {
        return util::getDataFormatColor(DataFormat<T>::numtype, DataFormat<T>::comp,
                                        DataFormat<T>::compsize);
    }
    static Document info(const T& data) {
        Document doc;
        doc.append("p", dataName());
        doc.append("p", toString(data));
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

template <typename T, typename A>
struct DataTraits<std::vector<T, A>> {
    static std::string classIdentifier() {
        return util::appendIfNotEmpty(DataTraits<T>::classIdentifier(), ".vector");
    }
    static std::string dataName() { return "vector<" + DataTraits<T>::dataName() + ">"; }
    static uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<T, A>& data) {
        return detail::vectorInfo<T>(data.size(), data.empty() ? nullptr : &data.front(),
                                     data.empty() ? nullptr : &data.back());
    }
};
template <typename T, typename A>
struct DataTraits<std::vector<T*, A>> {
    static std::string classIdentifier() {
        return util::appendIfNotEmpty(DataTraits<T>::classIdentifier(), ".ptr.vector");
    }
    static std::string dataName() { return "vector<" + DataTraits<T>::dataName() + "*>"; }
    static uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<T*, A>& data) {
        return detail::vectorInfo<T>(data.size(), data.empty() ? nullptr : data.front(),
                                     data.empty() ? nullptr : data.back());
    }
};
template <typename T, typename D, typename A>
struct DataTraits<std::vector<std::unique_ptr<T, D>, A>> {
    static std::string classIdentifier() {
        return util::appendIfNotEmpty(DataTraits<T>::classIdentifier(), ".unique_ptr.vector");
    }
    static std::string dataName() {
        return "vector<unique_ptr<" + DataTraits<T>::dataName() + ">>";
    }
    static uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<std::unique_ptr<T, D>, A>& data) {
        return detail::vectorInfo<T>(data.size(), data.empty() ? nullptr : data.front().get(),
                                     data.empty() ? nullptr : data.back().get());
    }
};
template <typename T, typename A>
struct DataTraits<std::vector<std::shared_ptr<T>, A>> {
    static std::string classIdentifier() {
        return util::appendIfNotEmpty(DataTraits<T>::classIdentifier(), ".shared_ptr.vector");
    }
    static std::string dataName() {
        return "vector<shared_ptr<" + DataTraits<T>::dataName() + ">>";
    }
    static uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<std::shared_ptr<T>, A>& data) {
        return detail::vectorInfo<T>(data.size(), data.empty() ? nullptr : data.front().get(),
                                     data.empty() ? nullptr : data.back().get());
    }
};

}  // namespace inviwo

#endif  // IVW_DATATRAITS_H
