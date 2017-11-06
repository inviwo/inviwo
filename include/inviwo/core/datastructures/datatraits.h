/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <type_traits>

namespace inviwo {

/**
 * \class DataTraits
 */
template <typename T, typename = void>
struct DataTraits {
    static std::string classIdentifier() {
        static_assert(util::HasClassIdentifier<T>::value,
                      "T must have a class identifier, if not add it, "
                      "or specialize DataTraits for T");
        return util::classIdentifier<T>();
    }
    static std::string dataName() {
        static_assert(util::HasDataName<T>::value,
                      "T must have a dataName, if not add it, "
                      "or specialize DataTraits for T");
        return util::dataName<T>();
    }
    static uvec3 colorCode() {
        static_assert(util::HasColorCode<T>::value,
                      "T must have a colorCode, if not add it, "
                      "or specialize DataTraits for T");
        return util::colorCode<T>();
    }
    static Document info(const T& data) {
        static_assert(util::HasInfo<T>::value || util::HasDataInfo<T>::value,
                      "T must have a info function, if not add it, "
                      "or specialize DataTraits for T");
        return util::info<T>(data);
    }
};

namespace util {

IVW_CORE_API uvec3 getDataFormatColor(NumericType t, size_t comp, size_t size);

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

template <typename T, typename A>
struct DataTraits<std::vector<T, A>> {
    static std::string classIdentifier() { return DataTraits<T>::classIdentifier() + ".vector"; }
    static std::string dataName() { return "vector<" + DataTraits<T>::dataName() + ">"; }
    static uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<T, A>& data) {
        Document doc;
        doc.append("p", dataName());
        doc.append("p", toString(data.size()));
        if (!data.empty()) {
            doc.append("p", DataTraits<T>::info(data.front()));
        }
        return doc;
    }
};
template <typename T, typename A>
struct DataTraits<std::vector<T*, A>> {
    static std::string classIdentifier() {
        return DataTraits<T>::classIdentifier() + ".ptr.vector";
    }
    static std::string dataName() { return "vector<" + DataTraits<T>::dataName() + "*>"; }
    static uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<T*, A>& data) {
        Document doc;
        doc.append("p", dataName());
        doc.append("p", toString(data.size()));
        if (!data.empty()) {
            doc.append("p", DataTraits<T>::info(*data.front()));
        }
        return doc;
    }
};
template <typename T, typename D, typename A>
struct DataTraits<std::vector<std::unique_ptr<T, D>, A>> {
    static std::string classIdentifier() {
        return DataTraits<T>::classIdentifier() + ".unique_ptr.vector";
    }
    static std::string dataName() { return "vector<unique_ptr<" + DataTraits<T>::dataName() + ">>"; }
    static uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<std::unique_ptr<T, D>, A>& data) {
        Document doc;
        doc.append("p", dataName());
        doc.append("p", toString(data.size()));
        if (!data.empty()) {
            doc.append("p", DataTraits<T>::info(*data.front()));
        }
        return doc;
    }
};
template <typename T, typename A>
struct DataTraits<std::vector<std::shared_ptr<T>, A>> {
    static std::string classIdentifier() {
        return DataTraits<T>::classIdentifier() + ".shared_ptr.vector";
    }
    static std::string dataName() { return "vector<shared_ptr<" + DataTraits<T>::dataName() + ">>"; }
    static uvec3 colorCode() {
        return glm::min(uvec3(30, 30, 30) + DataTraits<T>::colorCode(), uvec3(255));
    }
    static Document info(const std::vector<std::shared_ptr<T>, A>& data) {
        Document doc;
        doc.append("p", dataName());
        doc.append("p", toString(data.size()));
        if (!data.empty()) {
            doc.append("p", DataTraits<T>::info(*data.front()));
        }
        return doc;
    }
};

}  // namespace inviwo

#endif  // IVW_DATATRAITS_H
