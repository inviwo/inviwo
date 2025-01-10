/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/foreacharg.h>

#include <unordered_map>
#include <algorithm>
#include <utility>
#include <mutex>

namespace inviwo {

namespace util {

NumericType commonNumericType(std::span<const DataFormatBase*> formats) {
    if (formats.empty()) return NumericType::NotSpecialized;

    NumericType type = formats.front()->getNumericType();
    for (auto f : formats) {
        if (type == NumericType::Float || f->getNumericType() == NumericType::Float) {
            type = NumericType::Float;
        } else if (type == NumericType::SignedInteger ||
                   f->getNumericType() == NumericType::SignedInteger) {
            type = NumericType::SignedInteger;
        } else if (f->getNumericType() == NumericType::UnsignedInteger) {
            type = NumericType::UnsignedInteger;
        }
    }
    return type;
}

size_t commonFormatPrecision(std::span<const DataFormatBase*> formats) {
    if (formats.empty()) return 0;

    return (*std::ranges::max_element(formats,
                                      [](auto format1, auto format2) {
                                          return format1->getPrecision() < format2->getPrecision();
                                      }))
        ->getPrecision();
}

}  // namespace util

DataFormatBase::DataFormatBase(DataFormatId t, size_t c, size_t size, double max, double min,
                               double lowest, NumericType nt, std::string_view s)
    : formatId_(t)
    , components_(c)
    , sizeInBytes_(size)
    , numericType_(nt)
    , max_(max)
    , min_(min)
    , lowest_(lowest)
    , formatStr_(s) {}

DataFormatBase::DataFormatBase()
    : DataFormatBase(DataFormatId::NotSpecialized, 0, 0, 0.0, 0.0, 0.0, NumericType::NotSpecialized,
                     "NotSpecialized") {}

size_t DataFormatBase::getSizeInBytes() const { return sizeInBytes_; }
size_t DataFormatBase::getSize() const { return sizeInBytes_; }

NumericType DataFormatBase::getNumericType() const { return numericType_; }

size_t DataFormatBase::getComponents() const { return components_; }

size_t DataFormatBase::getPrecision() const { return sizeInBytes_ / components_ * 8; }

double DataFormatBase::getMax() const { return max_; }

double DataFormatBase::getMin() const { return min_; }

double DataFormatBase::getLowest() const { return lowest_; }

std::string_view DataFormatBase::getString() const { return formatStr_; }

DataFormatId DataFormatBase::getId() const { return formatId_; }

const DataFormatBase* DataFormatBase::get() { return get(DataFormatId::NotSpecialized); }

const DataFormatBase* DataFormatBase::get(DataFormatId id) {
    if (id < DataFormatId::NotSpecialized || id >= DataFormatId::NumberOfFormats) {
        throw DataFormatException(IVW_CONTEXT_CUSTOM("DataFormat"), "Invalid format id {}",
                                  static_cast<int>(id));
    }

    static_assert(static_cast<size_t>(DataFormatId::NumberOfFormats) ==
                  std::tuple_size_v<DefaultDataFormats> + 1);

    using DataFormatArray =
        std::array<DataFormatBase, static_cast<int>(DataFormatId::NumberOfFormats)>;
    static DataFormatArray instances = {};

    static std::once_flag flag;
    std::call_once(flag, [&]() {
        util::for_each_type<DefaultDataFormats>{}(
            [&, i = static_cast<int>(DataFormatId::NotSpecialized)]<typename T>() mutable {
                static_assert(sizeof(T) == sizeof(DataFormatBase));
                new (&instances[++i]) T();
            });
        return 0;
    });

    return &instances[static_cast<int>(id)];
}

const DataFormatBase* DataFormatBase::get(std::string_view name) {
    static auto nameMap = []() {
        std::unordered_map<std::string_view, const DataFormatBase*, CaseInsensitiveStringHash,
                           CaseInsensitiveEqual>
            res;
        res["uchar"] = DataFormat<glm::u8>::get();
        res["char"] = DataFormat<glm::i8>::get();
        res["ushort"] = DataFormat<glm::u16>::get();
        res["short"] = DataFormat<glm::i16>::get();
        res["uint"] = DataFormat<glm::u32>::get();
        res["int"] = DataFormat<glm::i32>::get();
        res["float"] = DataFormat<glm::f32>::get();
        res["double"] = DataFormat<glm::f64>::get();
        res["notspecialized"] = DataFormatBase::get();

        util::for_each_type<DefaultDataFormats>{}([&]<typename T>() { res[T::str()] = T::get(); });

        return res;
    }();

    auto it = nameMap.find(name);
    if (it != nameMap.end()) {
        return it->second;
    } else {
        throw DataFormatException(IVW_CONTEXT_CUSTOM("DataFormat"), "Invalid format string: '{}'",
                                  name);
    }
}

const DataFormatBase* DataFormatBase::get(NumericType type, size_t components, size_t precision) {
    static const auto table = []() {
        std::array<std::array<std::array<const DataFormatBase*, 4>, 4>, 4> res = {nullptr};
        util::for_each_type<DefaultDataFormats>{}([&]<typename F>() {
            const auto n = static_cast<size_t>(F::numericType());
            const auto c = F::components() - 1;
            const auto p = 64 - std::countl_zero(F::precision()) - 4;
            res[n][c][p] = F::get();
        });
        return res;
    }();

    const auto n = static_cast<size_t>(type);
    const auto c = components - 1;
    const auto p = static_cast<size_t>(64 - std::countl_zero(precision) - 4);

    if (std::has_single_bit(precision) && n < table.size() && c < table[0].size() &&
        p < table[0][0].size()) {
        return table[n][c][p];
    } else {
        return nullptr;
    }
}

}  // namespace inviwo
